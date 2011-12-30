#if !defined BRICK_STATE_4FCEAD32_CA4D_476C_931D_A6F69B5A2BC6
#define BRICK_STATE_4FCEAD32_CA4D_476C_931D_A6F69B5A2BC6

#include "brick_mcast_sm.h"

void IPresolv_context_update_table(struct IPresolv_context* brick);

int context_enough_packets(struct IPresolv_context* context);
int context_is_unique(struct IPresolv_context* context);
int context_is_mine(struct IPresolv_context* context);

#endif /* BRICK_STATE_4FCEAD32_CA4D_476C_931D_A6F69B5A2BC6*/
