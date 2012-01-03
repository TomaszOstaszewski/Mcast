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
    GLOBAL_RCV_THREAD = 3,
    GLOBAL_WFEX = 4,
    GLOBAL_PLAYER = 5,
    GLOBAL_SENDER_SETTINGS = 6,
    GLOBAL_SENDER_MCAST_CONN = 7,
    GLOBAL_SENDER_MASTER_RIFF = 8,
    GLOBAL_SENDER_STOP_EVENT = 9,
    GLOBAL_SENDER_SEND_PARAMS = 10,
	GLOBAL_LAST = 11
} global_variable_type_t;

struct  var_database;

/**
 * @brief  
 */
typedef struct var_database * var_database_t;

/**
 * @brief  
 */
var_database_t var_database_create(void);

/**
 * @brief  
 */
void var_database_destroy(var_database_t p_database);

/**
 * @brief  
 */
void add_ref(var_database_t database, global_variable_type_t e_state);

/**
 * @brief  
 */
void * get_var(var_database_t database, global_variable_type_t e_state);

/**
 * @brief  
 */
void set_var(var_database_t database, global_variable_type_t e_state, void * p_var);

/**
 * @brief  
 */
void release_ref(var_database_t database, global_variable_type_t e_state);

/**
 * @brief  
 */
void garbage_collect(var_database_t database);

#if defined __cplusplus
}
#endif 

#endif /*!defined VAR_DATABASE_37AEB355_DC9D_40B0_85A0_52DB8DDBB221 */
