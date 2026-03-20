/*
 * globals_var.c
 *
 *  Created on: 14 mars 2026
 *      Author: remyd
 */

#include "globals_var.h"

state_t gState = {OFF, UNLOGGED, 50, {{0},{0},{0}}};

UART_HandleTypeDef	huart2;

SemaphoreHandle_t	xMutexStruct;
SemaphoreHandle_t	xMutexPrint;

SemaphoreHandle_t	xSemaphoreButton;

QueueHandle_t		xQueueUart;
QueueHandle_t		xQueuePrint;
QueueHandle_t		xQueueEncoder;
QueueHandle_t		xQueueLedAndSpeaker;

volatile uint8_t	uart_queue_overflow_flag = 0;
volatile uint8_t 	rx_byte_it = 0;





