#include "can_utils.h"
#include "can.h"
#include "can_constants.h"
#include "board.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define DEBUG

#define UART_RX_BUFFER_SIZE 1 

#define CONFIGURE_VCU_HEARTBEAT 'v'
#define SEND_DISCHARGE_REQUEST 'd'
#define HELP 'h'

#define SEND_STANDBY_VCU_HEARTBEAT 's'
#define SEND_DISCHARGE_VCU_HEARTBEAT 'd'
#define DONT_SEND_VCU_HEARTBEAT 'n'

#define BAUDRATE 115200

#define CONFIGURE_VCU_HEARTBEAT_HELP_MESSAGE "Enter 's' to send VCU heartbeats with Standby state.\r\nEnter 'd' to send VCU heartbeats with Discharge state.\r\nEnter 'n' to stop sending VCU heartbeats."

#define SEND_VCU_HEARTBEAT_STANDBY_MESSAGE "Sending VCU heartbeat with Stanby state"
#define SEND_VCU_HEARTBEAT_DISCHARGE_MESSAGE "Sending VCU heartbeat with Discharge state"
#define DONT_SEND_VCU_HEARTBEAT_MESSAGE "Not sending VCU heartbeat"
#define UNRECOGNIZED_KEY_CONFIGURE_VCU_HEARTBEAT_MESSAGE "Unrecognized key. Please enter 's', 'd', or 'n'."

const uint32_t OscRateIn = 12000000;

volatile uint32_t msTicks;

CCAN_MSG_OBJ_T rx_msg;
uint8_t uart_rx_buf[UART_RX_BUFFER_SIZE];

enum VCU_STATE {
	STANDBY,
	DISCHARGE,
	NONE
};

enum VCU_STATE current_vcu_state = STANDBY;

uint32_t last_bms_heartbeat_time = 0; 

bool reset_can_peripheral = false;

/*****************************************************************************
 * Private function
 ****************************************************************************/

void SysTick_Handler(void) {
    msTicks++;
}

/**
 * @details prints the state of charge (measured in percentage) to the terminal
 *
 * @param soc_percentage state of charge, measured in percentage
 */
static void print_soc_percentage(uint16_t soc_percentage) {
        const uint8_t base_10 = 10;
	
        Board_Print("BMS SOC Percentage: ");
        Board_Println_Int(soc_percentage, base_10);
}

void process_can_return(uint32_t can_return) {
	const uint8_t hex_base = 16;
	if (can_return != NO_CAN_ERROR) {
		Board_Print("CAN Error: ");
		Board_Print_Int(can_return, hex_base);
		Board_Print("    ");
		Board_Println("Will attempt to reset CAN peripheral.");
		reset_can_peripheral = true;
	}
}

void process_can_errors(void) {
	const uint8_t hex_base = 16;
	const uint32_t can_error_status = CAN_GetErrorStatus();
	if (can_error_status != 0) {
		Board_Print("CAN Error: ");
		Board_Print_Int(can_error_status, hex_base);
		Board_Print("    ");
		Board_Println("Will attempt to reset CAN peripheral.");
		reset_can_peripheral = true;
	}
}

/**
 * @details sends a VCU heartbeat
 */
void sendVCUHeartbeat(void) {
	const uint8_t byteLength = 8;
	uint8_t data;
	uint8_t length;
	uint32_t can_transmit_return;
	switch (current_vcu_state) {
		case STANDBY:
			; //empty statement
			data = ____VCU_HEARTBEAT__STATE__STANDBY << (byteLength - 1);
			length = 1;
			can_transmit_return = 
				CAN_Transmit(VCU_HEARTBEAT__id, &data, length );
			process_can_return(can_transmit_return);
			process_can_errors();
			break;
		case DISCHARGE:
			; //empty statement
			data = ____VCU_HEARTBEAT__STATE__DISCHARGE << (byteLength - 1);
                        length = 1;
                        can_transmit_return = 
				CAN_Transmit(VCU_HEARTBEAT__id, &data, length );
			process_can_return(can_transmit_return);
			process_can_errors();
			break;
		case NONE:
			//Do nothing
			break;
		default:
			Board_Println("Invalid VCU state. Should never reach here");
			break;
	}
}

/**
 * @details reads incoming CAN messages and prints information to the terminal
 */
void process_can_inputs(void) {	
	uint32_t ret;
	BMS_HEARTBEAT_T bms_heartbeat;
	BMS_DISCHARGE_RESPONSE_T bms_discharge_response;

	ret = CAN_Receive(&rx_msg);

        if (ret == NO_CAN_ERROR) {
        	switch (rx_msg.mode_id) {
                	case BMS_HEARTBEAT__id:
                        	Board_Print("BMS Heartbeat:    ");
                        	CAN_MakeBMSHeartbeat(&bms_heartbeat, &rx_msg);
                        	switch (bms_heartbeat.state) {
                                	case ____BMS_HEARTBEAT__STATE__INIT:
                                        	Board_Print("BMS State: Init    ");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                                        case ____BMS_HEARTBEAT__STATE__STANDBY:
                                                Board_Print("BMS State: Standby    ");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                                        case ____BMS_HEARTBEAT__STATE__CHARGE:
                                                Board_Print("BMS State: Charge    ");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                               	        case ____BMS_HEARTBEAT__STATE__BALANCE:
                                                Board_Print("BMS State: Balance    ");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                                        case ____BMS_HEARTBEAT__STATE__DISCHARGE:
                                                Board_Print("BMS State: Discharge    ");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                                        case ____BMS_HEARTBEAT__STATE__ERROR:
                                                Board_Print("BMS State: Error    ");
                                                print_soc_percentage(bms_heartbeat.soc_percentage);
                                                break;
                                        default:
                                                Board_Print("Unexpected BMS State. You should never reach here");
                                                break;
                                }
                        	break;

			case BMS_DISCHARGE_RESPONSE__id:
                        	Board_Print("BMS Discharge Response    ");
                      	        CAN_MakeBMSDischargeResponse(&bms_discharge_response, &rx_msg);
                                switch (bms_discharge_response.discharge_response) {
	                                case ____BMS_DISCHARGE_RESPONSE__DISCHARGE_RESPONSE__NOT_READY:
                                        	Board_Println("Not Ready");
                                                break;
                                        case ____BMS_DISCHARGE_RESPONSE__DISCHARGE_RESPONSE__READY:
                                                Board_Println("Ready");
                                                break;
                                        }
                                        break;

                        case BMS_PACK_STATUS__id:
        	                Board_Println("BMS Pack Status");
       	                        //TODO
                                break;
                        case BMS_CELL_TEMPS__id:
                                Board_Println("BMS Cell Temp");
                                //TODO
                       	        break;
                        case BMS_ERRORS__id:
                                Board_Println("BMS Errors");
                                //TODO
                                break;
                        default:
                        	Board_Println("Unrecognized CAN message");
		}
	} else if (ret == NO_RX_CAN_MESSAGE) {
		// Do nothing	
	} else {
		process_can_return(ret);
	}
	process_can_errors();
}

void process_keyboard_input(void) {
	uint8_t count;
	count = Chip_UART_Read(LPC_USART, uart_rx_buf, UART_RX_BUFFER_SIZE);
	if (count != 0) {
		Chip_UART_SendBlocking(LPC_USART, uart_rx_buf, count);
		Board_Print("\r\n");
		switch (uart_rx_buf[0]) {
			case CONFIGURE_VCU_HEARTBEAT:
				Board_Println(CONFIGURE_VCU_HEARTBEAT_HELP_MESSAGE);
				count = Chip_UART_ReadBlocking(LPC_USART, uart_rx_buf, UART_RX_BUFFER_SIZE);
				Chip_UART_SendBlocking(LPC_USART, uart_rx_buf, count);
				Board_Print("\r\n");
				switch (uart_rx_buf[0]) {
					case SEND_STANDBY_VCU_HEARTBEAT:
						current_vcu_state = STANDBY;
						Board_Println(SEND_VCU_HEARTBEAT_STANDBY_MESSAGE);
						break;
					case SEND_DISCHARGE_VCU_HEARTBEAT:
						current_vcu_state = DISCHARGE;
						Board_Println(SEND_VCU_HEARTBEAT_DISCHARGE_MESSAGE);
						break;
					case DONT_SEND_VCU_HEARTBEAT:
						current_vcu_state = NONE;
						Board_Println(DONT_SEND_VCU_HEARTBEAT_MESSAGE);
						break;
					default:
						Board_Println(UNRECOGNIZED_KEY_CONFIGURE_VCU_HEARTBEAT_MESSAGE);
						break;
				}
				break;
			case SEND_DISCHARGE_REQUEST:
				; //empty statement
				const uint8_t discharge_request_bit_position = 7;
				uint8_t data = ____VCU_DISCHARGE_REQUEST__DISCHARGE_REQUEST__ENTER_DISCHARGE << discharge_request_bit_position;
				const uint8_t length = 1;
				CAN_Transmit(VCU_DISCHARGE_REQUEST__id, &data, length);
				Board_Println("Sent discharge request");
				break;
			case HELP:
				Board_Println("Enter 'v' to configure VCU heartbeat. Enter 'd' to send discharge request.");
				break;
			default:
				Board_Println("unrecognized key");
				break;
		}
	}
}

void process_can_outputs(void) {
	//Send BMS heartbeat every second
	const uint16_t one_second = 1000;
	if (msTicks - last_bms_heartbeat_time > one_second) {
		sendVCUHeartbeat();
		last_bms_heartbeat_time = msTicks;
	}
}

int main(void) {

	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		Board_Println("Failed SysTick_Config");
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

	CAN_Init(500000);

	Board_Println("Started up");
	Board_Println("Enter 'h' for help");

	while (1) {
       		if(reset_can_peripheral) {
            		Board_Println("Attempting to reset CAN peripheral...");
            		CAN_ResetPeripheral();
            		CAN_Init(500000);
            		Board_Println("Reset CAN peripheral.");
            		reset_can_peripheral = false;
        	}

		process_can_inputs();
		process_keyboard_input();
		process_can_outputs();

	}
}
