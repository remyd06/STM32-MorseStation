/**
 * @file    task_print.c
 * @brief   UART print task — serializes all transmissions through a single task.
 *
 * @details Any task that needs to print via UART calls vSendToPrintTask() which
 *          pushes bytes into xQueuePrint. This task consumes the queue and
 *          transmits via HAL_UART_Transmit, avoiding concurrent UART access.
 *
 * @author  remyd
 * @date    14 mars 2026
 */

#include "globals_var.h"

/**
 * @brief  Fills rx_buffer from xQueuePrint until '\0' is received.
 *
 * @details Blocks on xQueueReceive until a full string is available.
 *          '\0' is used as end-of-string sentinel — consistent with vSendToPrintTask().
 *
 * @param  rx_buffer  Destination buffer. Must be at least 64 bytes.
 */
void	set_buffer_print(char *rx_buffer)
{
	uint8_t	rx_byte  = 0;
	uint8_t	rx_index = 0;

	while (1)
	{
		xQueueReceive(xQueuePrint, &rx_byte, portMAX_DELAY);

		if (rx_byte == '\0')
		{
			rx_buffer[rx_index] = '\0';
			break;
		}
		rx_buffer[rx_index++] = rx_byte;
	}
}

/**
 * @brief  Print task entry point — transmits strings received from xQueuePrint.
 *
 * @details Loops forever, consuming strings from xQueuePrint via set_buffer_print()
 *          and transmitting them over USART2.
 *          Centralizing all UART transmissions here prevents race conditions
 *          between tasks.
 *
 * @param  argument  Unused FreeRTOS task argument.
 */
void	task_print(void *argument)
{
	char	rx_buffer[64];

	for (;;)
	{
		set_buffer_print(rx_buffer);
		HAL_UART_Transmit(&huart2, (uint8_t *)rx_buffer, strlen(rx_buffer), HAL_MAX_DELAY);
		memset(rx_buffer, 0, sizeof(rx_buffer));
	}
}
