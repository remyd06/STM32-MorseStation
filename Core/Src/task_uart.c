/**
 * ============================================================================
 * @file    task_uart.c
 * @brief   UART command reception and dispatch task.
 *
 * @details This file handles all UART communication for the MorseStation.
 *          It is structured in two phases :
 *
 *          Phase 1 (pre-RTOS, blocking) :
 *            - Power gate  : system refuses all commands until "ON" is received.
 *            - Auth gate   : login/password loop, only "admin"/"1234" grants access.
 *
 *          Phase 2 (FreeRTOS, interrupt-driven) :
 *            - System commands : prefixed with '#' (e.g. #SPEED <value>).
 *            - Morse messages  : forwarded to task_encoder via xQueueEncoder.
 *
 * @note    All phase-2 reception is interrupt-driven via HAL_UART_RxCpltCallback.
 *          Bytes are pushed into xQueueUart by the ISR and consumed by set_buffer_it().
 *          PuTTY send \r at the end of the input so this code work with only \r.
 *
 * @author  remyd
 * @date    14 mars 2026
 * ============================================================================
 */

#include "task_uart.h"

/**
 * @brief  HAL UART reception complete callback — called by HAL on every received byte.
 *
 * @details Pushes the received byte into xQueueUart for task_uart to consume.
 *          Sets uart_queue_overflow_flag if the queue is full.
 *          Re-arms interrupt reception — without this call, reception stops permanently.
 *          Yields to a higher-priority task if one was woken by the queue send.
 *
 * @param  huart  Pointer to the UART handle that triggered the callback.
 */
void	HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		BaseType_t	xHigherPriorityTaskWoken = pdFALSE;

		if (xQueueSendFromISR(xQueueUart, (const void *)&rx_byte_it, &xHigherPriorityTaskWoken) == errQUEUE_FULL)
			uart_queue_overflow_flag = 1;

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken)
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_byte_it, 1);
	}
}

/**
 * @brief  Fills rx_buffer with bytes received via UART until '\r' is received.
 *
 * @details Blocking implementation using HAL_UART_Receive().
 *          Must only be called before the FreeRTOS scheduler starts.
 *          Enforces a 64-byte maximum — warns the user and flushes the remaining
 *          bytes if the limit is exceeded to re-sync the UART stream.
 *
 * @param  rx_buffer  Destination buffer. Must be at least 64 bytes.
 */
void	set_buffer(char *rx_buffer)
{
	uint8_t rx_byte  = 0;
	size_t  rx_index = 0;

	while (1)
	{
		HAL_UART_Receive(&huart2, &rx_byte, 1, HAL_MAX_DELAY);

		if (rx_byte != '\r')
		{
			if (rx_index > 63)
			{
				HAL_UART_Transmit(&huart2, (uint8_t *)"[WARNING] MAXIMUM COMMAND SIZE REACHED. 64MAX.\r\n", 48, HAL_MAX_DELAY);
				rx_buffer[63] = '\0';

				/* Flush remaining bytes until '\r' to re-sync the UART stream. */
				while (1)
				{
					HAL_UART_Receive(&huart2, &rx_byte, 1, HAL_MAX_DELAY);
					if (rx_byte == '\r')
						break;
				}
				break;
			}
			rx_buffer[rx_index++] = rx_byte;
		}
		else
		{
			rx_buffer[rx_index] = '\0';
			break;
		}
	}
}

/**
 * @brief  Blocking power and authentication gate — runs before the scheduler starts.
 *
 * @details Two sequential phases :
 *
 *          Phase 1 — Power gate :
 *            Loops until "ON" is received. All other input is rejected.
 *            Red LED (PC4) is lit while the system is OFF.
 *
 *          Phase 2 — Authentication :
 *            Loops until valid credentials are entered.
 *            Only username "admin" with password "1234" grants access.
 *            "OFF" is accepted at any prompt and triggers a full system reset.
 *
 * @note   Uses blocking HAL_UART_Receive calls — bypasses the FreeRTOS queue.
 *         Must be called before HAL_UART_Receive_IT() is armed in the main loop.
 */
void	power_and_logger(void)
{
	char	rx_buffer[64];

	/* Phase 1 — Power gate : red LED on until system is turned ON. */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, SET);
	while (gState.STATE == OFF)
	{
		set_buffer(rx_buffer);

		if (!strcmp(rx_buffer, "ON"))
		{
			memset(rx_buffer, 0, sizeof(rx_buffer));
			gState.STATE = ON;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, SET);
		}
		else
		{
			memset(rx_buffer, 0, sizeof(rx_buffer));
			HAL_UART_Transmit(&huart2, (uint8_t *)"[ERROR] PROGRAMM OFF, TURN IT ON.\r\n", 35, HAL_MAX_DELAY);
		}
	}

	/* Phase 2 — Authentication loop. */
	if (gState.STATE == ON)
	{
		while (1)
		{
			HAL_UART_Transmit(&huart2, (uint8_t *)"LOGIN:\r\n", 8, HAL_MAX_DELAY);
			set_buffer(rx_buffer);

			if (!strcmp(rx_buffer, "OFF"))
				NVIC_SystemReset();

			if (!strcmp(rx_buffer, "admin"))
			{
				memset(rx_buffer, 0, sizeof(rx_buffer));
				HAL_UART_Transmit(&huart2, (uint8_t *)"PASSWORD:\r\n", 11, HAL_MAX_DELAY);
				set_buffer(rx_buffer);

				if (!strcmp(rx_buffer, "OFF"))
					NVIC_SystemReset();

				if (!strcmp(rx_buffer, "1234"))
				{
					memset(rx_buffer, 0, sizeof(rx_buffer));
					HAL_UART_Transmit(&huart2, (uint8_t *)"SUCCESSFULLY LOGGED!\r\n", 22, HAL_MAX_DELAY);
					gState.LOGIN = LOGGED;
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, SET);
					break;
				}
				else
				{
					memset(rx_buffer, 0, sizeof(rx_buffer));
					HAL_UART_Transmit(&huart2, (uint8_t *)"[ERROR] WRONG PASSWORD.\r\n", 25, HAL_MAX_DELAY);
				}
			}
			else if (!strcmp(rx_buffer, ""))
			{
				memset(rx_buffer, 0, sizeof(rx_buffer));
				HAL_UART_Transmit(&huart2, (uint8_t *)"[ERROR] EMPTY LOGIN.\r\n", 22, HAL_MAX_DELAY);
			}
			else
			{
				HAL_UART_Transmit(&huart2, (uint8_t *)"[ERROR] USER \"", 15, HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, (uint8_t *)rx_buffer, strlen(rx_buffer), HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2, (uint8_t *)"\" DOES NOT EXIST.\r\n", 19, HAL_MAX_DELAY);
				memset(rx_buffer, 0, sizeof(rx_buffer));
			}
		}
	}
}

/**
 * @brief  Fills rx_buffer from xQueueUart until '\r' is received.
 *
 * @details Interrupt-driven implementation — bytes are consumed from xQueueUart,
 *          which is filled by HAL_UART_RxCpltCallback().
 *          Handles queue overflow : clears the queue and returns an empty buffer.
 *          Enforces a 64-byte maximum — warns the user and flushes remaining bytes
 *          if exceeded to re-sync the stream.
 *
 * @note   Must only be called after the FreeRTOS scheduler has started.
 *
 * @param  rx_buffer  Destination buffer. Must be at least 64 bytes.
 */
void	set_buffer_it(char *rx_buffer)
{
	uint8_t	rx_byte  = 0;
	size_t	rx_index = 0;

	while (1)
	{
		/* Overflow guard — flush queue and abort if the ISR flagged an overflow. */
		if (uart_queue_overflow_flag)
		{
			vSendToPrintTask("[ERROR] UART QUEUE OVERFLOW.\r\n");
			xQueueReset(xQueueUart);
			uart_queue_overflow_flag = 0;

			 /* Flush remaining bytes until '\r' to re-sync the stream. */
			while (1)
			{
				xQueueReceive(xQueueUart, &rx_byte, portMAX_DELAY);
				if (rx_byte == '\r')
					break;
			}
			break;
		}

		xQueueReceive(xQueueUart, &rx_byte, portMAX_DELAY);

		if (rx_byte != '\r')
		{
			if (rx_index > 63)
			{
				vSendToPrintTask("[ERROR] MAXIMUM COMMAND SIZE REACHED. 64MAX.\r\n");
				memset(rx_buffer, 0, 64);

				/* Flush remaining bytes until '\r' to re-sync the stream. */
				while (1)
				{
					xQueueReceive(xQueueUart, &rx_byte, portMAX_DELAY);
					if (rx_byte == '\r')
						break;
				}
				break;
			}
			rx_buffer[rx_index++] = rx_byte;
		}
		else
		{
			rx_buffer[rx_index] = '\0';
			break;
		}
	}
}

/**
 * @brief  Main UART task — command reception and dispatch loop.
 *
 * @details Starts by running the blocking power and authentication gate.
 *          Once authenticated, enters the main FreeRTOS loop.
 *
 *          Two command types are handled :
 *
 *          1. System commands (prefix '#') :
 *             - #SPEED <value> : sets gState.SPEED (25–350 ms), mutex-protected.
 *
 *          2. Morse messages (no prefix) :
 *             - Converted to uppercase (morse tables use 'A'–'Z' indexing).
 *             - Forwarded byte by byte to xQueueEncoder, terminated by '\0'.
 *             - Notifies task_led_and_speaker via xTaskNotify if already running.
 *
 * @note   "OFF" is accepted at any time and triggers a full system reset via NVIC.
 *
 * @param  argument  Unused FreeRTOS task argument.
 */
void task_uart(void *argument)
{
	power_and_logger();

	char	rx_buffer[64];

	for (;;)
	{
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_byte_it, 1);
		set_buffer_it(rx_buffer);

		set_buffer_it(rx_buffer);

		/* If set_buffer_it returns an empty buffer, restart the loop. */
		if (!rx_buffer[0])
		    continue;

		/* Hard reset — "OFF" triggers NVIC system reset at any time. */
		if (!strcmp(rx_buffer, "OFF"))
			NVIC_SystemReset();

		/* If the buffer begin by #, enter in the command handler. */
		if (rx_buffer[0] == '#')
		{
			/* #SPEED <value> : update morse speed, range 25–350 ms. */
			if (!strncmp(&rx_buffer[1], "SPEED ", 6))
			{
				if (strlen(rx_buffer) < 11)
				{
					char cmd[strlen((rx_buffer) - 7)];
					strcpy(cmd, &rx_buffer[7]);

					if (atoi(cmd) >= 25 && atoi(cmd) <= 350)
					{
						xSemaphoreTake(xMutexStruct, portMAX_DELAY);
						gState.SPEED = atoi(cmd);
						xSemaphoreGive(xMutexStruct);
					}
					else
						vSendToPrintTask("[ERROR] SPEED MUST BE BETWEEN 25ms and 350ms.\r\n");
				}
				else
					vSendToPrintTask("[ERROR] SPEED MUST BE BETWEEN 25ms and 350ms.\r\n");
			}
			else
				vSendToPrintTask("[ERROR] UNKNOWN COMMAND.\r\n");
		}
		/* Else, the buffer contains the message to transmit. */
		else
		{
			/* Convert to uppercase — morse tables index from 'A' (65). */
			size_t	i = 0;
			while (rx_buffer[i])
			{
				if (rx_buffer[i] >= 97 && rx_buffer[i] <= 122)
					rx_buffer[i] -= 32;
				i++;
			}

			/* Forward message to encoder, byte by byte, '\0'-terminated. */
			i = 0;
			while (rx_buffer[i])
				xQueueSend(xQueueEncoder, &rx_buffer[i++], portMAX_DELAY);
			xQueueSend(xQueueEncoder, &rx_buffer[i], portMAX_DELAY);

			/* Notify task_led_and_speaker only if already running (skip first message). */
			if (gLedAndSpeakerRunning)
				xTaskNotify(xTaskLedAndSpeakerHandle, 0, eNoAction);
		}

		memset(rx_buffer, 0, 64);
	}
}
