/*
 * task_led_and_speaker.c
 *
 *  Created on: 20 mars 2026
 *      Author: remyd
 */

#include "task_led_and_speaker.h"

void	set_buffer_led_and_speaker(char	encoded_buffer[][7])
{
	char		q_byte = 0;
	size_t		encoded_index = 0;
	size_t		encoded_index_line = 0;
	char		sep = '\0';

	while (1)
	{
		xQueueReceive(xQueueLedAndSpeaker, &q_byte, portMAX_DELAY);

		if (q_byte != '\r' && q_byte != '\0')
			encoded_buffer[encoded_index_line][encoded_index++] = q_byte;
		else
		{
			if (q_byte == '\0')
			{
				encoded_buffer[encoded_index_line][encoded_index] = q_byte;
				break ;
			}
			else
			{
				encoded_buffer[encoded_index_line++][encoded_index] = sep;
				encoded_index = 0;
			}
		}
	}
}

void	task_led_and_speaker(void *argument)
{
	char	encoded_buffer[64][7];

	set_buffer_led_and_speaker(encoded_buffer);
	for (;;)
	{
		size_t		encoded_index = 0;
		size_t		encoded_index_line = 0;
		gLedAndSpeakerRunning = 1;


		while (encoded_buffer[encoded_index_line][0])
		{
			while (encoded_buffer[encoded_index_line][encoded_index])
			{
				if (encoded_buffer[encoded_index_line][encoded_index] == '.')
				{
					HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
					osDelay(gState.SPEED);
					HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
					osDelay(gState.SPEED);
				}
				else if (encoded_buffer[encoded_index_line][encoded_index] == '-')
				{
					HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
					osDelay(gState.SPEED * 3);
					HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
					osDelay(gState.SPEED);
				}
				else
					osDelay(gState.SPEED * 4); // si espace je sors de la boucle donc ca plus delai 3 = 7

				osDelay(gState.SPEED);
				encoded_index++;
			}
			osDelay(gState.SPEED * 3); // entre lettre
			encoded_index = 0;
			encoded_index_line++;
		}
		osDelay(gState.SPEED * 7);

		if (xTaskNotifyWait(0, 0, NULL, 0) == pdTRUE)
		{
		    memset(encoded_buffer, 0, sizeof(encoded_buffer));
		    set_buffer_led_and_speaker(encoded_buffer);
		}
	}
}
