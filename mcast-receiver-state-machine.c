#include "pcc.h"
#include "mcast-receiver-state-machine.h"
#include "mcast_setup.h"
#include "debug_helpers.h"
#include "dsoundplay.h"
#include "fifo-circular-buffer.h"
#include "wave_utils.h"
#include "var-database.h"

/**
 * @brief
 */
#define DEFAULT_MCASTADDRV4    "234.5.6.7"

/**
 * @brief
 */
#define DEFAULT_MCASTPORT      "25000"

/**
 * @brief
 */
#define DEFAULT_MCASTADDRV6    "ff12::1"

/**
 * @brief
 */
static receiver_state_t g_state;

static var_database_t g_var_database;

/**
 * @brief
 */
receiver_state_t receiver_get_state(void)
{
    return g_state;
}

static DWORD WINAPI ReceiverThreadProc(LPVOID param)
{
    uint16_t count = 0;
    struct data_item item;
    struct sockaddr_in sock_addr;
    struct fifo_circular_buffer * fifo;
    struct mcast_connection * conn;
    HANDLE h_stop_event;
    BOOL bDupResult;
    socklen_t sock_addr_size;
    sock_addr_size  = sizeof(struct sockaddr_in);
    item.data_      = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DATA_ITEM_SIZE);
    item.count_     = DATA_ITEM_SIZE;
    /* Add another reference to the data structures used below.
     * The enclosing data structure may become invalid right after external client signals h_stop_event */
    conn = get_var(g_var_database, GLOBAL_MCAST_CONNECTION); /**< \todo Add 'add_ref' here */
    assert(NULL != conn);
    fifo = get_var(g_var_database, GLOBAL_FIFO_QUEUE); /**< \todo Add 'add_ref' here */
    assert(NULL != fifo);
    h_stop_event = get_var(g_var_database, GLOBAL_RCV_EVENT);
    /* Along similiar lines - duplicate handle to the stop event. Another thread may call 'CloseHandle' on it, 
     * effectively invalidating it. Calling 'DuplicateHandle' is a precaution for such a condition */
    bDupResult = DuplicateHandle(GetCurrentProcess(), h_stop_event, GetCurrentProcess(), &h_stop_event, 0, FALSE, DUPLICATE_SAME_ACCESS);
    if (FALSE == bDupResult)
    {
        debug_outputln("%s %d : %8.8u", __FILE__, __LINE__, GetLastError());
        return -1;
    }
    for (; ; ++count)
    {
        int bytes_recevied;
        DWORD dwWaitResult;
        dwWaitResult = WaitForSingleObject(h_stop_event, 50);
        if (WAIT_TIMEOUT == dwWaitResult) /* Timedout - hence, nobody wants us to finish yet */
        {
            /* Receive data from the socket until you receiving this WSAMSGSIZE error */
            do { 
                bytes_recevied = recvfrom(
                        conn->socket_,
                        item.data_, 
                        item.count_, 
                        0,
                        (struct sockaddr*)&sock_addr,
                        &sock_addr_size
                        );
                if (SOCKET_ERROR != bytes_recevied)
                {
                    debug_outputln("%s %d : %8.8u", __FILE__, __LINE__, bytes_recevied);
                    item.count_ = bytes_recevied;
                    fifo_circular_buffer_push_item(fifo, &item);
                    break;
                }
                debug_outputln("%s %d : %8.8u", __FILE__, __LINE__, WSAGetLastError());
                item.data_ = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, item.data_, item.count_ + DATA_ITEM_SIZE);
                item.count_ += DATA_ITEM_SIZE;
            } while (WSAGetLastError() == WSAEMSGSIZE);
        }
        else if (WAIT_OBJECT_0 == dwWaitResult) /* Signaled - hence, the external enviroment has just requested termination */
        {
            
            break;
        }
        else
        {
            debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, dwWaitResult);
            break;
        }
    }
    CloseHandle(h_stop_event);
    HeapFree(GetProcessHeap(), 0, item.data_);
    debug_outputln("%s %d : %8.8u", __FILE__, __LINE__, WSAGetLastError());
    return 0;
}

static int handle_rcvstop_internal(void)
{
    HANDLE h_stop_event;
	assert(g_var_database);
    h_stop_event = get_var(g_var_database, GLOBAL_RCV_EVENT);
    assert (NULL != h_stop_event);
    SetEvent(h_stop_event);
    release_ref(g_var_database, GLOBAL_RCV_EVENT);
    return 0;
}

static int handle_mcastjoin_internal(void)
{
	assert(g_var_database);
    if (NULL == get_var(g_var_database, GLOBAL_MCAST_CONNECTION))
    {
        struct mcast_connection * p_mcast_conn;
        p_mcast_conn = setup_multicast_2(DEFAULT_MCASTADDRV4, DEFAULT_MCASTPORT);
        assert(NULL != p_mcast_conn);
        set_var(g_var_database, GLOBAL_MCAST_CONNECTION, p_mcast_conn);
    }
    add_ref(g_var_database, GLOBAL_MCAST_CONNECTION);
    return 0;
}

static int handle_mcastleave_internal(void)
{
    struct mcast_connection * p_mcast_conn;
    assert(NULL != get_var(g_var_database, GLOBAL_MCAST_CONNECTION));
    p_mcast_conn = get_var(g_var_database, GLOBAL_MCAST_CONNECTION);
    close_multicast(p_mcast_conn);
    release_ref(g_var_database, GLOBAL_MCAST_CONNECTION);
    return 0;
}

static int handle_stop_internal(void)
{
    DSOUNDPLAY player;
    player = get_var(g_var_database, GLOBAL_PLAYER);
    assert(NULL != player);
    if (NULL != player)
    {
        dsoundplayer_stop(player);    
        debug_outputln("%s %d", __FILE__, __LINE__);
        return 0;
    }
    return -1;
}

static int handle_play_internal(HWND hMainWnd)
{
    DSOUNDPLAY player;
    player = get_var(g_var_database, GLOBAL_PLAYER);
    debug_outputln("%s %d", __FILE__, __LINE__);
    if (NULL == player)
    {
        WAVEFORMATEX * p_wfex = get_var(g_var_database, GLOBAL_WFEX);
        struct fifo_circular_buffer * fifo = get_var(g_var_database, GLOBAL_FIFO_QUEUE);
        debug_outputln("%s %d", __FILE__, __LINE__);
        assert(NULL != p_wfex);
        if (NULL == fifo)
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            fifo = fifo_circular_buffer_create();
            set_var(g_var_database, GLOBAL_FIFO_QUEUE, fifo);
            add_ref(g_var_database, GLOBAL_FIFO_QUEUE);
        }
        assert(NULL != fifo);
        player = dsoundplayer_create(hMainWnd, p_wfex, fifo);
        set_var(g_var_database, GLOBAL_PLAYER, player);
    }
    add_ref(g_var_database, GLOBAL_PLAYER);
    dsoundplayer_play(player);    
    return 0;
}

static int handle_rcvstart_internal(void)
{
    HANDLE h_stop_event;
    HANDLE h_rcv_thread;
    struct fifo_circular_buffer * fifo;
    h_stop_event = get_var(g_var_database, GLOBAL_RCV_EVENT);
    if (NULL == h_stop_event)
    {
        HANDLE h_stop_event =  CreateEvent(NULL, TRUE, FALSE, "recv-event-0");
        set_var(g_var_database, GLOBAL_RCV_EVENT, (void*)h_stop_event);
        add_ref(g_var_database, GLOBAL_RCV_EVENT); 
    }
    fifo = get_var(g_var_database, GLOBAL_FIFO_QUEUE);
    if (NULL == fifo)
    {
        debug_outputln("%s %d", __FILE__, __LINE__);
        fifo = fifo_circular_buffer_create();
        set_var(g_var_database, GLOBAL_FIFO_QUEUE, fifo);
        add_ref(g_var_database, GLOBAL_FIFO_QUEUE);
    }
    ResetEvent(h_stop_event);
    h_rcv_thread = CreateThread(NULL, 0, ReceiverThreadProc, NULL, 0, NULL);
    CloseHandle(h_rcv_thread);
    return 0;
}

void receiver_init(var_database_t database)
{
	g_var_database = database;
	assert(g_var_database);
}

/*!
 * @brief
 */
void handle_play(HWND hMainWnd)
{
    if (RECEIVER_MCASTJOINED == g_state)
    {
        handle_play_internal(hMainWnd);
        g_state = RECEIVER_MCASTJOINED_PLAYING;
    }
    else if (RECEIVER_INITIAL == g_state)
    {
        handle_play_internal(hMainWnd);
        g_state = RECEIVER_PLAYING;
    }
    else if (RECEIVER_RECEIVING == g_state)
    {
        handle_play_internal(hMainWnd);
        g_state = RECEIVER_RECEIVING_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

/*!
 * @brief
 */
void handle_stop(void)
{
    if (RECEIVER_PLAYING == g_state)
    {
        if  (0 == handle_stop_internal())
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            g_state = RECEIVER_INITIAL;
        }
    }
    else if (RECEIVER_RECEIVING_PLAYING == g_state)
    {
        if  (0 == handle_stop_internal())
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            g_state = RECEIVER_RECEIVING;
        }
    }
    else if (RECEIVER_MCASTJOINED_PLAYING == g_state)
    {
        if  (0 == handle_stop_internal())
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            g_state = RECEIVER_MCASTJOINED;
        }
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}
 
/*!
 * @brief
 */
void handle_rcvstart(void)
{
    if (RECEIVER_MCASTJOINED == g_state)
    {
        handle_rcvstart_internal();
        g_state = RECEIVER_RECEIVING;
    } 
    else if (RECEIVER_MCASTJOINED_PLAYING == g_state)
    {
        handle_rcvstart_internal();
        g_state = RECEIVER_RECEIVING_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

/*!
 * @brief
 */
void handle_rcvstop(void)
{
    if (RECEIVER_RECEIVING_PLAYING == g_state)
    {
        handle_rcvstop_internal();
        g_state = RECEIVER_MCASTJOINED_PLAYING;
    }
    else if (RECEIVER_RECEIVING == g_state)
    {
        handle_rcvstop_internal();
        g_state = RECEIVER_MCASTJOINED;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

/*!
 * @brief
 */
void handle_mcastjoin(void)
{
    if (RECEIVER_INITIAL == g_state)
    {
        handle_mcastjoin_internal();
        g_state = RECEIVER_MCASTJOINED;
    }
    else if (RECEIVER_PLAYING == g_state)
    {
        handle_mcastjoin_internal();
        g_state = RECEIVER_MCASTJOINED_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

/*!
 * @brief
 */
void handle_mcastleave(void)
{
    if (RECEIVER_MCASTJOINED == g_state)
    {
        handle_mcastleave_internal();
        g_state = RECEIVER_INITIAL; 
    }
    else if (RECEIVER_MCASTJOINED_PLAYING == g_state)
    {
        handle_mcastleave_internal();
        g_state = RECEIVER_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

