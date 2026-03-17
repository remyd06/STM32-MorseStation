/*
 * task_print.c
 *
 *  Created on: 14 mars 2026
 *      Author: remyd
 */

#include "globals_var.h"

void	set_buffer_print(char *rx_buffer)
{
	uint8_t	rx_byte = 0;
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

void	task_print(void *argument)
{
	char	rx_buffer[64];

	for (;;)
	{
		set_buffer_print(rx_buffer);
		printf(rx_buffer);
		memset(rx_buffer, 0, sizeof(rx_buffer));
	}
}
