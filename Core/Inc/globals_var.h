/*
 * globals_var.h
 *
 *  Created on: 14 mars 2026
 *      Author: remyd
 */

#ifndef INC_GLOBALS_VAR_H_
#define INC_GLOBALS_VAR_H_

# include "FreeRTOS.h"
# include "semphr.h"
# include <string.h>
# include <stdlib.h>
# include <ctype.h>
# include <stdio.h>

typedef	enum
{
	OFF = 0,
	ON = 1,

}	power_t;

typedef	enum
{
	LOGGED = 0,
	UNLOGGED = 1,

}	login_t;

typedef	struct
{
	power_t		STATE;
	login_t		LOGIN;
	uint16_t	SPEED;
	char		HISTORIC[3][64];
}	state_t;

extern state_t				gState;

extern SemaphoreHandle_t	xMutexStruct;

extern SemaphoreHandle_t	xSemaphoreButton;

extern QueueHandle_t		xQueueUart;
extern QueueHandle_t		xQueuePrint;

extern volatile uint8_t		uart_queue_overflow_flag;
extern volatile uint8_t 	rx_byte_it;

#endif /* INC_GLOBALS_VAR_H_ */
