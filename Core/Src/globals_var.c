/*
 * globals_var.c
 *
 *  Created on: 14 mars 2026
 *      Author: remyd
 */

#include "globals_var.h"

state_t gState = {OFF, UNLOGGED, 150, {{0},{0},{150}}};

UART_HandleTypeDef	huart2;

SemaphoreHandle_t	xMutexStruct;
SemaphoreHandle_t	xMutexPrint;

SemaphoreHandle_t	xSemaphoreButton;

QueueHandle_t		xQueueUart;
QueueHandle_t		xQueuePrint;
QueueHandle_t		xQueueEncoder;

volatile uint8_t	encoder_queue_overflow_flag = 0;
volatile uint8_t	uart_queue_overflow_flag = 0;
volatile uint8_t 	rx_byte_it = 0;





