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

/**
 * @brief  
 */
static state_longevity_data_t globals_table[GLOBAL_LAST];

/**
 * @brief  
 */
void add_ref(global_variable_type_t e_state)
{
    ++globals_table[e_state].ref_count_;     
}

/**
 * @brief  
 */
void * get_var(global_variable_type_t e_state)
{
    return globals_table[e_state].p_var_;
}

void garbage_collect(void)
{
    static global_variable_type_t all_vars[] = {
        GLOBAL_MCAST_CONNECTION,
        GLOBAL_FIFO_QUEUE,
        GLOBAL_RCV_EVENT,
        GLOBAL_RCV_THREAD,
        GLOBAL_WFEX,
    };
    global_variable_type_t const * p_begin = &all_vars[0];
    global_variable_type_t const * p_past_end = &all_vars[sizeof(all_vars)/sizeof(all_vars[0])];
    for (; p_begin != p_past_end; ++p_begin)
    {
        if (0 == globals_table[*p_begin].ref_count_ && NULL != globals_table[*p_begin].p_var_)
        {
            switch (*p_begin)
            {
                case GLOBAL_FIFO_QUEUE:
                    fifo_circular_buffer_delete((struct fifo_circular_buffer*)globals_table[*p_begin].p_var_);
                    break;
                case GLOBAL_MCAST_CONNECTION:
                case GLOBAL_WFEX:
                    HeapFree(GetProcessHeap(), 0, globals_table[*p_begin].p_var_);
                    break;
                case GLOBAL_RCV_EVENT:
                case GLOBAL_RCV_THREAD:
                    CloseHandle((HANDLE)globals_table[*p_begin].p_var_);
                    break;
                case GLOBAL_PLAYER:
                    dsoundplayer_destroy(globals_table[*p_begin].p_var_);
                    break;
            }
            globals_table[*p_begin].p_var_ = NULL;
        }
    }
}

/**
 * @brief  
 */
void * set_var(global_variable_type_t e_state, void * p_var)
{
    return globals_table[e_state].p_var_ = p_var;
}

/**
 * @brief  
 */
void release_ref(global_variable_type_t e_state)
{
    if ( globals_table[e_state].ref_count_)
        --globals_table[e_state].ref_count_;     
}
