/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/*!
 * @file var-database.h
 * @author T. Ostaszewski 
 */ 
#if !defined VAR_DATABASE_37AEB355_DC9D_40B0_85A0_52DB8DDBB221
#define VAR_DATABASE_37AEB355_DC9D_40B0_85A0_52DB8DDBB221

#if defined __cplusplus
extern "C" {
#endif 

/*!
 * @brief
 */
typedef enum state_variable { 
    GLOBAL_MCAST_CONNECTION = 0,
    GLOBAL_FIFO_QUEUE = 1,
    GLOBAL_RCV_EVENT = 2,
    GLOBAL_WFEX = 3,
    GLOBAL_PLAYER = 4,
    GLOBAL_SENDER_SETTINGS = 5,
    GLOBAL_SENDER_MCAST_CONN = 6,
    GLOBAL_SENDER_MASTER_RIFF = 7,
    GLOBAL_SENDER_STOP_EVENT = 8,
    GLOBAL_SENDER_SEND_PARAMS = 9,
	GLOBAL_LAST = 10
} global_variable_type_t;

/** @brief Forward declaration */
struct  var_database;

typedef void (*CLEANUP_FUNC)(void *);

/**
 * @brief  
 */
typedef struct var_database * var_database_t;

/**
 * @brief  
 * @return
 */
var_database_t var_database_create(void);

/**
 * @brief  
 * @param
 */
void var_database_destroy(var_database_t p_database);

/**
 * @brief  
 * @param
 * @param
 * @return
 */
void * get_var(var_database_t database, global_variable_type_t var_type);

/**
 * @brief  
 * @param
 * @param
 */
int set_var(var_database_t database, global_variable_type_t var_type, void * p_var, CLEANUP_FUNC p_cleanup);

/**
 * @brief  
 * @param
 * @param
 */
void release_ref(var_database_t database, global_variable_type_t var_type);

/**
 * @brief  
 * @param
 * @param
 */
void garbage_collect(var_database_t database);

#if defined __cplusplus
}
#endif 

#endif /*!defined VAR_DATABASE_37AEB355_DC9D_40B0_85A0_52DB8DDBB221 */
