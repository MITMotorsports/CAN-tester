#include "util.h"
#include <string.h>
#include "can.h"
#include <stdlib.h>
#include "can_constants.h"
#include "can_utils.h"
#include "ccand_11xx.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define LED_PORT 0
#define LED_PIN 7

#define UART_RX_BUFFER_SIZE 1 

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
    #define DEBUG_Print(str) Chip_UART_SendBlocking(LPC_USART, str, strlen(str))
    #define DEBUG_Write(str, count) Chip_UART_SendBlocking(LPC_USART, str, count)
#else
    #define DEBUG_Print(str)
    #define DEBUG_Write(str, count) 
#endif

#define CONFIGURE_VCU_HEARTBEAT 'v'
#define SEND_DISCHARGE_REQUEST 'd'
#define HELP 'h'

#define SEND_STANDBY_VCU_HEARTBEAT 's'
#define SEND_DISCHARGE_VCU_HEARTBEAT 'd'
#define DONT_SEND_VCU_HEARTBEAT 'n'

#define BAUDRATE 115200

#define CONFIGURE_VCU_HEARTBEAT_HELP_MESSAGE "Enter 's' to send VCU heartbeats with Standby state.\r\nEnter 'd' to send VCU heartbeats with Discharge state.\r\nEnter 'n' to stop sending VCU heartbeats.\r\n"

#define SEND_VCU_HEARTBEAT_STANDBY_MESSAGE "Sending VCU heartbeat with Stanby state\r\n"
#define SEND_VCU_HEARTBEAT_DISCHARGE_MESSAGE "Sending VCU heartbeat with Discharge state\r\n"
#define DONT_SEND_VCU_HEARTBEAT_MESSAGE "Not sending VCU heartbeat\r\n"
#define UNRECOGNIZED_STATE_CONFIGURE_VCU_HEARTBEAT_MESSAGE "Unrecognized state. Please enter 's', 'd', or 'n'.\r\n"

const uint32_t OscRateIn = 12000000;

volatile uint32_t msTicks;

CCAN_MSG_OBJ_T rx_msg;
uint8_t str[100];
uint8_t uart_rx_buf[UART_RX_BUFFER_SIZE];

uint8_t Rx_Buf[8];

enum VCU_STATE {
	STANDBY,
	DISCHARGE,
	NONE
};

enum VCU_STATE VCU_STATE_T = STANDBY;

uint32_t last_bms_heartbeat_time = 0;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void SysTick_Handler(void) {
    msTicks++;
}

/**
 * @details prints contents of UART buffer
 */
static void Print_Buffer(uint8_t* buff, uint8_t buff_size) {
    Chip_UART_SendBlocking(LPC_USART, "0x", 2);
    uint8_t i;
    for(i = 0; i < buff_size; i++) {
        itoa(buff[i], str, 16);
        if(buff[i] < 16) {
            Chip_UART_SendBlocking(LPC_USART, "0", 1);
        }
        Chip_UART_SendBlocking(LPC_USART, str, 2);
    }
}

/**
 * @details prints the state of charge (measured in percentage) to the terminal
 *
 * @param soc_percentage state of charge, measured in percentage
 */
static void print_soc_percentage(uint16_t soc_percentage) {
	const uint8_t soc_digit_count = 8;
        char soc_percentage_string[soc_digit_count];
        const uint8_t base_10 = 10;


	itoa(soc_percentage, soc_percentage_string, base_10);
        DEBUG_Print("BMS SOC Percentage: ");
        DEBUG_Print(soc_percentage_string);
        DEBUG_Print("\r\n");
}

/**
 * @details send a BMS heartbeat
 */
void sendBMSHeartbeat(void) {
	uint8_t data;
	uint8_t length;
	switch (VCU_STATE_T) {
		case STANDBY:
			; //empty statement
			data = 0x1 << 7;
			length = 1;
			CAN_Transmit(VCU_HEARTBEAT__id, &data, length );
			break;
		case DISCHARGE:
			; //empty statement
			data = 0x0 << 7;
                        length = 1;
                        CAN_Transmit(VCU_HEARTBEAT__id, &data, length );
			break;
		case NONE:
			//Do nothing
			break;
		default:
			DEBUG_Print("Invalid VCU state. Should never reach here\r\n");
			break;
	}
}

/**
 * @details reads incoming CAN messages and prints information to the terminal
 */
void Process_CAN_Inputs(void) {	
	uint32_t ret;
	BMS_HEARTBEAT_T bms_heartbeat;
	BMS_DISCHARGE_RESPONSE_T bms_discharge_response;

	ret = CAN_Receive(&rx_msg);

        if (ret == NO_CAN_ERROR) {
        	switch (rx_msg.mode_id) {
                	case BMS_HEARTBEAT__id:
                        	DEBUG_Print("BMS Heartbeat\r\n");
                        	CAN_MakeBMSHeartbeat(&bms_heartbeat, &rx_msg);
                        	switch (bms_heartbeat.state) {
                                	case ____BMS_HEARTBEAT__STATE__INIT:
                                        	DEBUG_Print("BMS State: Init\r\n");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                                        case ____BMS_HEARTBEAT__STATE__STANDBY:
                                                DEBUG_Print("BMS State: Standby\r\n");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                                        case ____BMS_HEARTBEAT__STATE__CHARGE:
                                                DEBUG_Print("BMS State: Charge\r\n");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                               	        case ____BMS_HEARTBEAT__STATE__BALANCE:
                                                DEBUG_Print("BMS State: Balance\r\n");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                                        case ____BMS_HEARTBEAT__STATE__DISCHARGE:
                                                DEBUG_Print("BMS State: Discharge\r\n");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                                        case ____BMS_HEARTBEAT__STATE__ERROR:
                                                DEBUG_Print("BMS State: Error\r\n");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                                        default:
                                                DEBUG_Print("Unexpected BMS State. You should never reach here\r\n");
                                                break;
                                        }
                                        break;

			case BMS_DISCHARGE_RESPONSE__id:
                        	DEBUG_Print("BMS Discharge Response\r\n");
                      	        CAN_MakeBMSDischargeResponse(&bms_discharge_response, &rx_msg);
                                switch (bms_discharge_response.discharge_response) {
	                                case ____BMS_DISCHARGE_RESPONSE__DISCHARGE_RESPONSE__NOT_READY:
                                        	DEBUG_Print("Not Ready\r\n");
                                                break;
                                        case ____BMS_DISCHARGE_RESPONSE__DISCHARGE_RESPONSE__READY:
                                                DEBUG_Print("Ready\r\n");
                                                break;
                                        }
                                        break;

                        case BMS_PACK_STATUS__id:
        	                DEBUG_Print("BMS Pack Status\r\n");
       	                        //TODO
                                break;
                        case BMS_CELL_TEMPS__id:
                                DEBUG_Print("BMS Cell Temp\r\n");
                                //TODO
                       	        break;
                        case BMS_ERRORS__id:
                                DEBUG_Print("BMS Errors\r\n");
                                //TODO
                                break;
                        default:
                        	DEBUG_Print("Unrecognized CAN message\r\n");
		}
	}
}

/**
 * Transmits CAN messages
 */
void Process_CAN_Outputs(void) {
	// TODO
	// read UART
	// if something in the buffer
	//	switch(character)
	//		case bms heartbeat:
	//			block until receiving another character
	//			switch (character received)
	//				case Init
	//					change state to sending Init
	//				case Standby
	//					change state to sending Satndby
	//				...
	//		case bms discharge response
	//			send bms discharge response

	// send bms heartbeat every second

	uint8_t count;
	count = Chip_UART_Read(LPC_USART, uart_rx_buf, UART_RX_BUFFER_SIZE);
	if (count != 0) {
		Chip_UART_SendBlocking(LPC_USART, uart_rx_buf, count);
		DEBUG_Print("\r\n");
		switch (uart_rx_buf[0]) {
			case CONFIGURE_VCU_HEARTBEAT:
				DEBUG_Print(CONFIGURE_VCU_HEARTBEAT_HELP_MESSAGE);
				count = Chip_UART_ReadBlocking(LPC_USART, uart_rx_buf, UART_RX_BUFFER_SIZE);
				Chip_UART_SendBlocking(LPC_USART, uart_rx_buf, count);
				DEBUG_Print("\r\n");
				switch (uart_rx_buf[0]) {
					case SEND_STANDBY_VCU_HEARTBEAT:
						VCU_STATE_T = STANDBY;
						DEBUG_Print(SEND_VCU_HEARTBEAT_STANDBY_MESSAGE);
						break;
					case SEND_DISCHARGE_VCU_HEARTBEAT:
						VCU_STATE_T = DISCHARGE;
						DEBUG_Print(SEND_VCU_HEARTBEAT_DISCHARGE_MESSAGE);
						break;
					case DONT_SEND_VCU_HEARTBEAT:
						VCU_STATE_T = NONE;
						DEBUG_Print(DONT_SEND_VCU_HEARTBEAT_MESSAGE);
						break;
					default:
						DEBUG_Print(UNRECOGNIZED_STATE_CONFIGURE_VCU_HEARTBEAT_MESSAGE);
						break;
				}
				break;
			case SEND_DISCHARGE_REQUEST:
				; //empty statement
				uint8_t discharge_request_bit_position = 7;
				uint8_t data = ____VCU_DISCHARGE_REQUEST__DISCHARGE_REQUEST__ENTER_DISCHARGE << discharge_request_bit_position;
				uint8_t length = 1;
				CAN_Transmit(VCU_DISCHARGE_REQUEST__id, &data, length);
				DEBUG_Print("Sent discharge request\r\n");
				break;
			case HELP:
				DEBUG_Print("Enter 'v' to configure VCU heartbeat. Enter 'd' to send discharge request.\r\n");
				break;
			default:
				DEBUG_Print("unrecognized key\r\n");
				break;
		}
	}

	//Send BMS heartbeat every second
	const uint16_t one_second = 1000;
	if (msTicks - last_bms_heartbeat_time > one_second) {
		sendBMSHeartbeat();
		last_bms_heartbeat_time = msTicks;
	}
}

int main(void) {

	SystemCoreClockUpdate();

	uint32_t reset_can_peripheral_time;
	const uint32_t can_error_delay = 5000;
	bool reset_can_peripheral = false;

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* TXD */
	
	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, BAUDRATE);
	// Configure data width, parity, and stop bits
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	DEBUG_Print("Started up\n\r");
	DEBUG_Print("Enter 'h' for help\r\n");

	CAN_Init(500000);
	
	while (1) {
		//read can message
			//switch (message ID)
				//switch (message)
					//DEBUG_PRINT()
		
		
//		uint8_t count;
//		uint8_t data[1];

       		 if(reset_can_peripheral && msTicks > reset_can_peripheral_time) {
            		DEBUG_Print("Attempting to reset CAN peripheral...\r\n ");
            		CAN_ResetPeripheral();
            		CAN_Init(500000);
            		DEBUG_Print("Reset CAN peripheral. \r\n ");
            		reset_can_peripheral = false;
        	}

		Process_CAN_Inputs();
		Process_CAN_Outputs();

//		if (msTicks % 1000 == 0){
//            		// recieve message if there is a message
//		    	ret = CAN_Receive(&rx_msg);
//            		if(ret == NO_RX_CAN_MESSAGE) {
//                		DEBUG_Print("No CAN message received...\r\n");
//            		} else if(ret == NO_CAN_ERROR) {
//                	DEBUG_Print("Recieved data ");
//                	Print_Buffer(rx_msg.data, rx_msg.dlc);
//                	DEBUG_Print(" from ");
//                	itoa(rx_msg.mode_id, str, 16);
//                	DEBUG_Print(str);
//                	DEBUG_Print("\r\n");
//            		} else {
//                		DEBUG_Print("CAN Error: ");
//                		itoa(ret, str, 2);
//                		DEBUG_Print(str);
//                		DEBUG_Print("\r\n");
//
//                		DEBUG_Print("Will attempt to reset peripheral in ");
//                		itoa(can_error_delay/1000, str, 10);
//                		DEBUG_Print(str);
//                		DEBUG_Print(" seconds.\r\n");
//                		reset_can_peripheral = true;
//                		reset_can_peripheral_time = msTicks + can_error_delay;
//            		}

            		// transmit a message!
//			data[0] = 0xAA;
//			CAN_Transmit(0x600, data, 1);
//		}
        
//		if ((count = Chip_UART_Read(LPC_USART, uart_rx_buf, UART_RX_BUFFER_SIZE)) != 0) {
//			switch (uart_rx_buf[0]) {
//				case 'a':
//					DEBUG_Print("Sending CAN with ID: 0x600\r\n");
//					data[0] = 0xAA;
//					ret = CAN_Transmit(0x600, data, 1);
//                    			if(ret != NO_CAN_ERROR) {
//                        			DEBUG_Print("CAN Error: ");
//					    	itoa(ret, str, 2);
//					    	DEBUG_Print(str);
//                        			DEBUG_Print("\r\n");
//                    			}
//					break;
//				default:
//					DEBUG_Print("Invalid Command\r\n");
//					break;
//			}
//		}
	}
}
