#include "can_utils.h"
#include "ccand_11xx.h"
#include "can_constants.h"

void CAN_MakeBMSHeartbeat(BMS_HEARTBEAT_T * bms_heartbeat, CCAN_MSG_OBJ_T * msg_obj) {
	//get state
	//get soc_percentage
	//construct BMS_HEARTBEAT_T
	const uint32_t CAN_message_highest_bit = 63;
	const uint64_t soc_percentage_mask = 0x1FFC000000000000;
	uint64_t state = 
		msg_obj->data_64 >> (CAN_message_highest_bit - __BMS_HEARTBEAT__STATE__end);
	uint64_t soc_percentage = (msg_obj->data_64 & soc_percentage_mask) >> 
		(CAN_message_highest_bit - __BMS_HEARTBEAT__SOC_PERCENTAGE__end);

	bms_heartbeat->state = state;
	bms_heartbeat->soc_percentage = soc_percentage;
}

void CAN_MakeBMSDischargeResponse(BMS_DISCHARGE_RESPONSE_T * bms_discharge_response, CCAN_MSG_OBJ_T * msg_obj) {
        const uint32_t CAN_message_highest_bit = 63;
        const uint64_t bms_discharge_response_mask = 0x8000000000000000;
        uint64_t discharge_response = (msg_obj->data_64 & bms_discharge_response_mask) >> 
		(CAN_message_highest_bit - __BMS_DISCHARGE_RESPONSE__DISCHARGE_RESPONSE__end);

        bms_discharge_response->discharge_response = discharge_response;
}

