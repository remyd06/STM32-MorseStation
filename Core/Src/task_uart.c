/*
 * task_uart.c
 *
 *  Created on: 14 mars 2026
 *      Author: remyd
 */

#include "task_uart.h"

extern UART_HandleTypeDef huart2;

void set_buffer(char *rx_buffer)
{
    uint8_t rx_index = 0;
    uint8_t rx_byte = 0;

    while (1)
    {
    	HAL_UART_Receive(&huart2, &rx_byte, 1, HAL_MAX_DELAY);

        if (rx_byte == '\r' || rx_byte == '\n')
        {
            rx_buffer[rx_index] = '\0';

            if (rx_byte == '\r')
            	HAL_UART_Receive(&huart2, &rx_byte, 1, 10);

            if (!strcmp(rx_buffer, "OFF"))
                    NVIC_SystemReset();
            break;
        }

       if (rx_index < 63)
            rx_buffer[rx_index++] = rx_byte;
    }

		while (rx_byte != '\r' && rx_byte != '\n')
			HAL_UART_Receive(&huart2, &rx_byte, 1, 10);
    	if (rx_byte == '\r')
            HAL_UART_Receive(&huart2, &rx_byte, 1, 10);
}


void task_uart(void *argument)
{
	char	rx_buffer[64];

	while (gState.STATE == OFF)
	{
		set_buffer(rx_buffer);
		if (!strcmp(rx_buffer, "ON"))
		{
			memset(rx_buffer, 0, sizeof(rx_buffer));
			gState.STATE = ON;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, SET);
		}
		else
		{
			memset(rx_buffer, 0, sizeof(rx_buffer));
			HAL_UART_Transmit(&huart2, (uint8_t *)"[ERROR] PROGRAMM OFF, TURN IT ON.\r\n", 35, HAL_MAX_DELAY);
		}
	}


	if (gState.STATE == ON)
	{
		while (1)
		{
			HAL_UART_Transmit(&huart2, (uint8_t *)"LOGIN:\r\n", 8, HAL_MAX_DELAY);
			set_buffer(rx_buffer);

			if (!strcmp(rx_buffer, "admin"))
			{
				memset(rx_buffer, 0, sizeof(rx_buffer));
				HAL_UART_Transmit(&huart2, (uint8_t *)"PASSWORD:\r\n", 11, HAL_MAX_DELAY);
				set_buffer(rx_buffer);

				if (!strcmp(rx_buffer, "1234"))
				{
					memset(rx_buffer, 0, sizeof(rx_buffer));
					HAL_UART_Transmit(&huart2, (uint8_t *)"SUCCESSFULLY LOGGED!\r\n", 22, HAL_MAX_DELAY);
					gState.LOGIN = ON;
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, SET);
					break ;
				}
				else
				{
					memset(rx_buffer, 0, sizeof(rx_buffer));
					HAL_UART_Transmit(&huart2, (uint8_t *)"[ERROR] WRONG PASSWORD.\r\n", 25, HAL_MAX_DELAY);
				}
			}
			else if (!strcmp(rx_buffer, ""))
			{
				HAL_UART_Transmit(&huart2, (uint8_t *)"[ERROR] EMPTY LOGIN.\r\n", 22, HAL_MAX_DELAY);
				memset(rx_buffer, 0, sizeof(rx_buffer));
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

	for (;;)
	{
		vTaskDelay(10);
	}
}

