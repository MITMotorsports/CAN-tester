#include "board.h"

#include "chip.h"
#include <string.h>

uint32_t Board_Print(const char *str) {
	return Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
}

uint32_t Board_Println(const char *str) {
	uint32_t count = Board_Print(str);
	return count + Board_Print("\r\n");
}
