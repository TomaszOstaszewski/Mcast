/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file var-database.c
 * @author T. Ostaszewski 
 * @date 03-Jan-2012
 */ 
#include "pcc.h"
#include "debug_helpers.h"
#include "var-database.h"

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

void * get_var(var_database_t database, global_variable_type_t e_state)
{
    assert(database);
    if (0 != database->globals_table_[e_state].ref_count_)
    {
        ++database->globals_table_[e_state].ref_count_;
        return database->globals_table_[e_state].p_var_;
    }
    return NULL;
}

int set_var(var_database_t database, global_variable_type_t e_state, void * p_var, CLEANUP_FUNC p_cleanup)
{
    assert(database);
    if (0 == database->globals_table_[e_state].ref_count_)
    {
        ++database->globals_table_[e_state].ref_count_;
        database->globals_table_[e_state].p_var_ = p_var;
        database->globals_table_[e_state].p_dctor_ = p_cleanup;
        return 0;
    }
    return 1;
}

void release_ref(var_database_t database, global_variable_type_t e_state)
{
    assert(database);
    if (database->globals_table_[e_state].ref_count_)
    {
        --database->globals_table_[e_state].ref_count_;     
        if (0 == database->globals_table_[e_state].ref_count_ && NULL != database->globals_table_[e_state].p_dctor_)
        {
            (*database->globals_table_[e_state].p_dctor_)(database->globals_table_[e_state].p_var_);
        }
    }
}

void garbage_collect(var_database_t database)
{
#if 0
    state_longevity_data_t * p_begin = database->globals_table_;
    state_longevity_data_t const * const p_past_end = database->globals_table_ + GLOBAL_LAST;
    assert(database);
    for (; p_begin != p_past_end; ++p_begin)
    {
        if (0 == p_begin->ref_count_ && NULL != p_begin->p_var_)
        {
           (*p_begin->p_dctor_)(p_begin->p_var_);
           p_begin->p_var_ = NULL;
        }
    }
#endif
    debug_outputln("%s "
    "[%2.2u]=%2.2u "
    "[%2.2u]=%2.2u "
    "[%2.2u]=%2.2u "
    "[%2.2u]=%2.2u "
    "[%2.2u]=%2.2u ",
    "GC collect",
/*0*/    GLOBAL_MCAST_CONNECTION, database->globals_table_[GLOBAL_MCAST_CONNECTION].ref_count_,
/*1*/    GLOBAL_FIFO_QUEUE,  database->globals_table_[GLOBAL_FIFO_QUEUE].ref_count_,
/*2*/    GLOBAL_RCV_EVENT,   database->globals_table_[GLOBAL_RCV_EVENT].ref_count_,
/*3*/    GLOBAL_WFEX,        database->globals_table_[GLOBAL_WFEX].ref_count_,
/*4*/    GLOBAL_PLAYER,      database->globals_table_[GLOBAL_PLAYER].ref_count_
    );
}

