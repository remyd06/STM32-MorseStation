/*
 * task_uart.c
 *
 *  Created on: 14 mars 2026
 *      Author: remyd
 */

#include "task_uart.h"

void	HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART2)
	{

		BaseType_t	xHigherPriorityTaskWoken = pdFALSE;
		if (xQueueSendFromISR(xQueueUart, (const void *)&rx_byte_it, &xHigherPriorityTaskWoken) == errQUEUE_FULL)
			uart_queue_overflow_flag = 1;

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken)
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_byte_it, 1);
	}
}

void	set_buffer(char *rx_buffer)
{
    size_t	rx_index = 0;
    uint8_t rx_byte = 0;

    while (1)
    {
    	HAL_UART_Receive(&huart2, &rx_byte, 1, HAL_MAX_DELAY);

		if (rx_byte != '\r')
		{
			if (rx_index > 63)
			{
				HAL_UART_Transmit(&huart2, (uint8_t *)"[WARNING] MAXIMUM COMMAND SIZE REACHED. 64MAX.\r\n", 48, HAL_MAX_DELAY);
				rx_buffer[63] = '\0';

				while (1)
				{
					HAL_UART_Receive(&huart2, &rx_byte, 1, HAL_MAX_DELAY);
					if (rx_byte == '\r')
						break;
				}

				break ;
			}
			rx_buffer[rx_index++] = rx_byte;
		}
		else
		{
			rx_buffer[rx_index] = '\0';
			break ;
		}
    }
}

void	power_and_logger()
{
	char	rx_buffer[64];

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

void	set_buffer_it(char *rx_buffer)
{
	size_t	rx_byte = 0;
	uint8_t	rx_index = 0;

	while (1)
	{
		if (uart_queue_overflow_flag)
		{
			vSendToPrintTask("[ERROR] UART QUEUE OVERFLOW.\r\n");
			xQueueReset(xQueueUart);
			uart_queue_overflow_flag = 0;
			break ;
		}

		xQueueReceive(xQueueUart, &rx_byte, portMAX_DELAY);
		if (rx_byte != '\r')
		{
			if (rx_index > 63)
			{
				vSendToPrintTask("[ERROR] MAXIMUM COMMAND SIZE REACHED. 64MAX.\r\n");
				memset(rx_buffer, 0, 64);

				while (1)
				{
					xQueueReceive(xQueueUart, &rx_byte, portMAX_DELAY);
					if (rx_byte == '\r')
							break ;
				}
				break ;
			}
			rx_buffer[rx_index++] = rx_byte;
		}
		else
		{
			rx_buffer[rx_index] = '\0';
			break ;
		}
	}
}



void	task_uart(void *argument)
{
//	power_and_logger();

	char	rx_buffer[64];
	for (;;)
	{
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_byte_it, 1);
		set_buffer_it(rx_buffer);

		if (!strcmp(rx_buffer, "OFF"))
			NVIC_SystemReset();

		if (rx_buffer[0] == '#')
		{

			if (!strncmp(&rx_buffer[1], "SPEED ", 6))
			{

				if (strlen(rx_buffer) < 11)
				{
					char	cmd[strlen((rx_buffer) - 7)];
					char buf[16];

					snprintf(buf, sizeof(buf), "SPEED=%d\r\n", gState.SPEED);
					HAL_UART_Transmit(&huart2, (uint8_t *)buf, strlen(buf), HAL_MAX_DELAY);
					strcpy(cmd, &rx_buffer[7]);
					if ((atoi(cmd) >= 75 && atoi(cmd) <= 350))
					{
						xSemaphoreTake(xMutexStruct, portMAX_DELAY);

						gState.SPEED = atoi(cmd);

						xSemaphoreGive(xMutexStruct);
						snprintf(buf, sizeof(buf), "SPEED=%d\r\n", gState.SPEED);
						HAL_UART_Transmit(&huart2, (uint8_t *)buf, strlen(buf), HAL_MAX_DELAY);
					}
					else
						vSendToPrintTask("[ERROR] SPEED MUST BE BETWEEN 75ms and 350ms.\r\n");

				}
				else
					vSendToPrintTask("[ERROR] SPEED MUST BE BETWEEN 75ms and 350ms.\r\n");
			}
			else
				vSendToPrintTask("[ERROR] UNKNOWN COMMAND.\r\n");
		}
		else
		{
			size_t	i = 0;
			while (rx_buffer[i])
				xQueueSend(xQueueEncoder, &rx_buffer[i++], portMAX_DELAY);
			xQueueSend(xQueueEncoder, &rx_buffer[i], portMAX_DELAY);
		}


		memset(rx_buffer, 0, 64);
	}
}

