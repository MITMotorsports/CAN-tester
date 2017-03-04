// TODO:
// - write specifications for each method

#include "can_utils.h"

#include <stdlib.h>

#include "ccand_11xx.h"
#include "can_constants.h"
#include "board.h"

//#define DEBUG

/***************************************************************************************
 * Private functions
 * ************************************************************************************/

uint64_t construct_64_bit_can_message(CCAN_MSG_OBJ_T * msg_obj) {
	const uint8_t bytes_in_can_message = 8;
	const uint8_t bits_in_byte = 8;
	uint64_t can_message = 0;
#ifdef DEBUG
	Board_Println("In construct_64_bit_can_message");
	char can_message_str[100];
	char i_str[100];
	char data_i_str[100];
	char left_shift_str[10];
#endif //DEBUG
	uint64_t data_i;
	uint32_t left_shift;
	uint64_t shifted_data_i;
	uint8_t i;
	for (i=0; i<bytes_in_can_message; i++) {
		data_i = msg_obj->data[i];
		left_shift = (bytes_in_can_message - 1 - i)*bits_in_byte;
		shifted_data_i = data_i << left_shift;
		can_message = can_message | shifted_data_i;
	
#ifdef DEBUG
		Board_Print("i: ");
		itoa(i, i_str, 10);
		Board_Println(i_str);

		Board_Print("left_shift: ");
		Board_Println_Int(left_shift, 10);

		Board_Print("shifted_data_i ");
		Board_Println_Int(shifted_data_i, 2);
		
		Board_Print("data_i: ");
		itoa(data_i, data_i_str, 2);
		Board_Println(data_i_str);
		
		itoa(can_message, can_message_str, 2);
		Board_Print("can message: ");
		Board_Println(can_message_str);
#endif //DEBUG
	}
#ifdef DEBUG
	Board_Println("n");
	uint32_t n = 0;
	char n_str[10];
	for (i=0; i<10; i++) {
		Board_Print("left_shift: ");
		left_shift = 10-i-1;
		itoa(left_shift, left_shift_str, 10);
		Board_Println(left_shift_str);

		n = n | (1 << left_shift);
		itoa(n, n_str, 2);
		Board_Println(n_str);
	}
	uint64_t my_int = 1;
	uint64_t my_shifted_int = my_int << (32);
	Board_Print("my_shifted_int: ");
	Board_Println_Int(my_shifted_int, 2);
#endif //DEBUG
	return can_message;
}

/*
 * TODO
 */
uint32_t construct_32_bit_can_message_string_bytes_0_3(CCAN_MSG_OBJ_T * msg_obj) {
	uint32_t can_message_bytes_0_3  = 0;
	const uint8_t max_byte_four_byte_number = 3;
	const uint8_t bits_in_byte = 8;
	uint8_t i;
	for (i=0; i<=max_byte_four_byte_number; i++) {
		can_message_bytes_0_3 = can_message_bytes_0_3 | 
			(msg_obj->data[i] << 
			(max_byte_four_byte_number - i)*bits_in_byte);
	}
	return can_message_bytes_0_3;
}

/***************************************************************************************
 * Public functions
 * ************************************************************************************/

void CAN_MakeBMSHeartbeat(BMS_HEARTBEAT_T * bms_heartbeat, CCAN_MSG_OBJ_T * msg_obj) {
	uint32_t can_message = construct_32_bit_can_message_string_bytes_0_3(msg_obj);

	//get state and soc
	//TODO: don't hard code the masks
	const uint32_t state_mask =          0xE0000000;
	const uint32_t soc_percentage_mask = 0x1FF80000;
	const uint8_t four_byte_number_max_bit = 31;
	const uint32_t state = (can_message & state_mask) >> 
		(four_byte_number_max_bit - __BMS_HEARTBEAT__STATE__end);
	const uint32_t soc_percentage = (can_message & soc_percentage_mask) >> 
		(four_byte_number_max_bit - __BMS_HEARTBEAT__SOC_PERCENTAGE__end);
	
	//construct BMS_HEARTBEAT_T
	bms_heartbeat->state = state;
	bms_heartbeat->soc_percentage = soc_percentage;
}

void CAN_MakeBMSDischargeResponse(BMS_DISCHARGE_RESPONSE_T * bms_discharge_response,
		CCAN_MSG_OBJ_T * msg_obj) {
        const uint32_t CAN_message_highest_bit = 63;
        const uint64_t bms_discharge_response_mask = 0x8000000000000000;
        uint64_t discharge_response = (msg_obj->data_64 & bms_discharge_response_mask) 
		>> (CAN_message_highest_bit - 
		__BMS_DISCHARGE_RESPONSE__DISCHARGE_RESPONSE__end);

        bms_discharge_response->discharge_response = discharge_response;
}
