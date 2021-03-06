#include <stdint.h>
#include "chip.h"
#include "ccand_11xx.h"

typedef struct {
	uint8_t state;
	uint16_t soc_percentage;
} BMS_HEARTBEAT_T;

/**
 * @details translates a CCAN_MSG_OBJ_T into a BMS_HEARTBEAT_T
 *
 * @param bms_heartbeat datatype that is mutated to store data in msg_obj
 * @param msg_obj datatype containing information about a CAN message
 */
void CAN_MakeBMSHeartbeat(BMS_HEARTBEAT_T * bms_heartbeat, CCAN_MSG_OBJ_T * msg_obj);

typedef struct {
	uint8_t discharge_response;
} BMS_DISCHARGE_RESPONSE_T;

/**
 * @details translates a CCAN_MSG_OBJ_T into a BMS_DISCHARGE_RESPONSE_T
 *
 * @param bms_discharge_response datatype that is mutated to store data in msg_obj
 * @param msg_obj datatype containing information about a CAN message
 */
void CAN_MakeBMSDischargeResponse(BMS_DISCHARGE_RESPONSE_T * bms_discharge_response, CCAN_MSG_OBJ_T * msg_obj);
