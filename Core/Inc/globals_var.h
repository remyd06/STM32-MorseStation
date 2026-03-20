/*
 * globals_var.h
 *
 *  Created on: 14 mars 2026
 *      Author: remyd
 */

#ifndef INC_GLOBALS_VAR_H_
#define INC_GLOBALS_VAR_H_

# include "FreeRTOS.h"
# include "cmsis_os.h"
# include "semphr.h"
# include "main.h"
# include "utils.h"
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

extern UART_HandleTypeDef	huart2;

extern TaskHandle_t			xTaskLedAndSpeakerHandle;

extern SemaphoreHandle_t	xMutexStruct;
extern SemaphoreHandle_t	xMutexPrint;

extern SemaphoreHandle_t	xSemaphoreButton;

extern QueueHandle_t		xQueueUart;
extern QueueHandle_t		xQueuePrint;
extern QueueHandle_t		xQueueEncoder;
extern QueueHandle_t		xQueueLedAndSpeaker;

extern volatile uint8_t		gLedAndSpeakerRunning;
extern volatile uint8_t		uart_queue_overflow_flag;
extern volatile uint8_t 	rx_byte_it;

#endif /* INC_GLOBALS_VAR_H_ */
