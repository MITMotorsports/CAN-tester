#include "unity.h"
#include "can_utils.h"
#include "can_constants.h"

// TODO: put code that translates data into a CAN message object in a helper function
// Recompile can_constants.h

/**
 * Testing Strategy:
 * 
 * CAN_MakeBMSHeartBeat
 * - state bits:
 *   - contains a 1 on leftmost and rightmost side
 *   - doesn't contain a 1 on leftmost and rightmost side
 * - soc_percentage bits
 *   - contains a 1 on leftmost and rightmost side
 *   - doesn't contain a 1 on leftmost and rightmost side
 *
 * CAN_MakeBMSDischargeResponse
 * - discharge_reponse
 *   - NOT_READY
 *   - READY
 */

/**
 * Covers: 
 * CAN_MakeBMSHeartBeat
 * - state bits
 *   - doesn't contain a 1 on leftmost and rightmost side
 * - soc_percentage bits
 *   - doesn't contain a 1 on leftmost and rightmost side
 */
void test_CAN_MakeBMSHeartBeat_NoOneBitsLeftmostAndRightmostSide(void) {	
	const uint64_t expected_state = ____BMS_HEARTBEAT__STATE__STANDBY;
	const uint64_t expected_soc_percentage = 1;

	CCAN_MSG_OBJ_T msg_obj;
	msg_obj.mode_id = BMS_HEARTBEAT__id;
	const uint32_t CAN_message_max_bit = 63;
	msg_obj.data_64 = 0 | 
		(expected_state << (CAN_message_max_bit - __BMS_HEARTBEAT__STATE__end)) | 
		(expected_soc_percentage << (CAN_message_max_bit - __BMS_HEARTBEAT__SOC_PERCENTAGE__end));

	BMS_HEARTBEAT_T bms_heartbeat;
	CAN_MakeBMSHeartbeat(&bms_heartbeat, &msg_obj);

	TEST_ASSERT_EQUAL_INT(expected_state, bms_heartbeat.state);
	TEST_ASSERT_EQUAL_INT(expected_soc_percentage, bms_heartbeat.soc_percentage);
}

/**
 * Covers: 
 * CAN_MakeBMSHeartBeat
 * - state bits
 *   - contains a 1 on leftmost and rightmost side
 * - soc_percentage bits
 *   - contains a 1 on leftmost and rightmost side
 */
void test_CAN_MakeBMSHeartBeat_OneBitsLeftmostAndRightmostSide(void) {
        const uint64_t expected_state = ____BMS_HEARTBEAT__STATE__ERROR;
        const uint64_t expected_soc_percentage = 0b1000000001;

        CCAN_MSG_OBJ_T msg_obj;
        msg_obj.mode_id = BMS_HEARTBEAT__id;
        const uint32_t CAN_message_max_bit = 63;
        msg_obj.data_64 = 0 |
                (expected_state << (CAN_message_max_bit - __BMS_HEARTBEAT__STATE__end)) |
                (expected_soc_percentage << (CAN_message_max_bit - __BMS_HEARTBEAT__SOC_PERCENTAGE__end));

        BMS_HEARTBEAT_T bms_heartbeat;
        CAN_MakeBMSHeartbeat(&bms_heartbeat, &msg_obj);

        TEST_ASSERT_EQUAL_INT(expected_state, bms_heartbeat.state);
        TEST_ASSERT_EQUAL_INT(expected_soc_percentage, bms_heartbeat.soc_percentage);
}

/**
 * Covers:
 * CAN_MakeBMSDischargeResponse
 * - discharge response
 *   - READY
 */
void test_CAN_MakeBMSDischargeResponse_One(void) {
        const uint64_t expected_discharge_response = ____BMS_DISCHARGE_RESPONSE__DISCHARGE_RESPONSE__READY;

        CCAN_MSG_OBJ_T msg_obj;
        msg_obj.mode_id = BMS_DISCHARGE_RESPONSE__id;
        const uint32_t CAN_message_max_bit = 63;
        msg_obj.data_64 = 0 |
                (expected_discharge_response << (CAN_message_max_bit - __BMS_DISCHARGE_RESPONSE__DISCHARGE_RESPONSE__end));

        BMS_DISCHARGE_RESPONSE_T bms_discharge_response;
        CAN_MakeBMSDischargeResponse(&bms_discharge_response, &msg_obj);

        TEST_ASSERT_EQUAL_INT(expected_discharge_response, bms_discharge_response.discharge_response);
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test_CAN_MakeBMSHeartBeat_NoOneBitsLeftmostAndRightmostSide);
	RUN_TEST(test_CAN_MakeBMSHeartBeat_OneBitsLeftmostAndRightmostSide);
	RUN_TEST(test_CAN_MakeBMSDischargeResponse_One);
	return UNITY_END();
}

