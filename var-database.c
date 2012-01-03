/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file var-database.c
 * @author T. Ostaszewski 
 */ 
#include "pcc.h"
#include "var-database.h"
#include "dsoundplay.h"
#include "fifo-circular-buffer.h"

/**
 * @brief 
 */
typedef struct state_longevity_data {
    uint32_t ref_count_;        /**< State variable reference count. */
    void *  p_var_; /**< State variable instance. */
    void (*p_dctor_)(void * );  /**< State dctor */
} state_longevity_data_t;

/*!
 * @brief
 */
struct var_database {
    state_longevity_data_t globals_table_[1];
};

var_database_t var_database_create(void)
{
    struct var_database * p_database = (struct var_database *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(state_longevity_data_t)*GLOBAL_LAST);  
    assert(NULL != p_database);
    return p_database;
} 

void var_database_destroy(var_database_t p_database)
{ 
    HeapFree(GetProcessHeap(), 0, p_database);
}

void add_ref(var_database_t database, global_variable_type_t e_state)
{
    assert(database);
    ++database->globals_table_[e_state].ref_count_;
}

void * get_var(var_database_t database, global_variable_type_t e_state)
{
    assert(database);
    return database->globals_table_[e_state].p_var_;
}

void set_var(var_database_t database, global_variable_type_t e_state, void * p_var)
{
    assert(database);
    database->globals_table_[e_state].p_var_ = p_var;
}

void release_ref(var_database_t database, global_variable_type_t e_state)
{
    assert(database);
    if (database->globals_table_[e_state].ref_count_)
        --database->globals_table_[e_state].ref_count_;     
}

void garbage_collect(var_database_t database)
{
    static global_variable_type_t all_vars[] = {
        GLOBAL_MCAST_CONNECTION,
        GLOBAL_FIFO_QUEUE,
        GLOBAL_RCV_EVENT,
        GLOBAL_RCV_THREAD,
        GLOBAL_WFEX,
        GLOBAL_PLAYER,
        GLOBAL_SENDER_SETTINGS,
        GLOBAL_SENDER_MCAST_CONN,
        GLOBAL_SENDER_MASTER_RIFF,
        GLOBAL_SENDER_STOP_EVENT,
        GLOBAL_SENDER_SEND_PARAMS
    };
    global_variable_type_t const * p_begin = &all_vars[0];
    global_variable_type_t const * p_past_end = &all_vars[sizeof(all_vars)/sizeof(all_vars[0])];
    assert(database);
    for (; p_begin != p_past_end; ++p_begin)
    {
        if (0 == database->globals_table_[*p_begin].ref_count_ && NULL != database->globals_table_[*p_begin].p_var_)
        {
            switch (*p_begin)
            {
                case GLOBAL_FIFO_QUEUE:
                    fifo_circular_buffer_delete((struct fifo_circular_buffer*)database->globals_table_[*p_begin].p_var_);
                    break;
                case GLOBAL_MCAST_CONNECTION:
                case GLOBAL_WFEX:
                    HeapFree(GetProcessHeap(), 0, database->globals_table_[*p_begin].p_var_);
                    break;
                case GLOBAL_RCV_EVENT:
                case GLOBAL_RCV_THREAD:
                    CloseHandle((HANDLE)database->globals_table_[*p_begin].p_var_);
                    break;
                case GLOBAL_PLAYER:
                    dsoundplayer_destroy(database->globals_table_[*p_begin].p_var_);
                    break;
				case GLOBAL_SENDER_SETTINGS:
				case GLOBAL_SENDER_MCAST_CONN:
				case GLOBAL_SENDER_MASTER_RIFF:
				case GLOBAL_SENDER_STOP_EVENT:
				case GLOBAL_SENDER_SEND_PARAMS:
					assert(0);
					break;
				default:
					break;
            }
            database->globals_table_[*p_begin].p_var_ = NULL;
        }
    }
}

