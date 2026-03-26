/**
 * @file    globals_var.c
 * @brief   Definition and initialization of all global variables.
 *
 * @details Single source of truth for all shared state across tasks.
 *          All variables declared here are declared as extern in globals_var.h
 *          and accessible project-wide.
 *
 * @author  remyd
 * @date    14 mars 2026
 */

#include "globals_var.h"

/* System state — default : OFF, UNLOGGED, SPEED=50ms, empty historic. */
state_t				gState = {OFF, UNLOGGED, 50, {{0},{0},{0}}};

/* UART handle — shared between task_uart, task_print and power_and_logger. */
UART_HandleTypeDef	huart2;

/* Task handle — used by task_uart to notify task_led_and_speaker. */
TaskHandle_t		xTaskLedAndSpeakerHandle;

/* Mutexes. */
SemaphoreHandle_t	xMutexStruct; /* Protects gState from concurrent access.  */
SemaphoreHandle_t	xMutexPrint;  /* Serializes UART transmissions.            */

/* Semaphores. */
SemaphoreHandle_t	xSemaphoreButton; /* Signaled by EXTI ISR on button press. */

/* Queues. */
QueueHandle_t		xQueueUart;         /* ISR -> task_uart : raw received bytes.        */
QueueHandle_t		xQueuePrint;        /* Any task -> task_print : strings to transmit. */
QueueHandle_t		xQueueEncoder;      /* task_uart -> task_encoder : plain text.       */
QueueHandle_t		xQueueLedAndSpeaker;/* task_encoder -> task_led_and_speaker : morse. */

/* Volatile for always reading — written by ISR, read by tasks. */
volatile uint8_t	gLedAndSpeakerRunning    = 0; /* Set when task_led_and_speaker has started transmitting. */
volatile uint8_t	uart_queue_overflow_flag = 0; /* Set by ISR when xQueueUart is full.                    */
volatile uint8_t	rx_byte_it               = 0; /* Staging byte for HAL_UART_Receive_IT.                  */
