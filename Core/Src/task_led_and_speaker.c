/**
 * @file    task_led_and_speaker.c
 * @brief   Morse playback task — drives LEDs and speaker from encoded sequences.
 *
 * @details Receives morse-encoded sequences from xQueueLedAndSpeaker (sent by
 *          task_encoder) and plays them on the hardware outputs.
 *
 *          Queue format expected :
 *            - '.' or '-' : morse symbol.
 *            - '|'        : inter-word space.
 *            - '\r'       : end of one morse symbol string (next character follows).
 *            - '\0'       : end of full message.
 *
 *          Timing follows standard morse code rules :
 *            - dot       : 1 unit ON  + 1 unit OFF.
 *            - dash      : 3 units ON + 1 unit OFF.
 *            - inter-symbol gap : 1 unit (already included in ON/OFF cycle).
 *            - inter-letter gap : 3 units.
 *            - inter-word gap   : 7 units (4 here — 3 already added as inter-letter).
 *
 *          After playing the full message, the task loops indefinitely until
 *          xTaskNotify signals a new message is available.
 *
 * @note   1 unit = gState.SPEED (ms), adjustable via #SPEED command in task_uart.
 *
 * @author  remyd
 * @date    20 mars 2026
 */

#include "task_led_and_speaker.h"

/**
 * @brief  Fills encoded_buffer from xQueueLedAndSpeaker until '\0' is received.
 *
 * @details Reads the morse sequence byte by byte.
 *          '\r' signals the end of one symbol string — null-terminates the current
 *          line and moves to the next row of encoded_buffer.
 *          '\0' signals the end of the full message and exits.
 *
 * @param  encoded_buffer  2D destination buffer [64][7].
 *                         Each row holds one morse symbol string, null-terminated.
 */
void	set_buffer_led_and_speaker(char encoded_buffer[][7])
{
	char	q_byte             = 0;
	char	sep                = '\0';
	size_t	encoded_index      = 0;
	size_t	encoded_index_line = 0;

	while (1)
	{
		xQueueReceive(xQueueLedAndSpeaker, &q_byte, portMAX_DELAY);

		if (q_byte != '\r' && q_byte != '\0')
		{
			encoded_buffer[encoded_index_line][encoded_index++] = q_byte;
		}
		else
		{
			if (q_byte == '\0')
			{
				/* End of message — null-terminate last line and exit. */
				encoded_buffer[encoded_index_line][encoded_index] = sep;
				break;
			}
			else
			{
				/* End of symbol — null-terminate current line, move to next. */
				encoded_buffer[encoded_index_line++][encoded_index] = sep;
				encoded_index = 0;
			}
		}
	}
}

/**
 * @brief  LED and speaker task entry point — plays morse sequences on hardware outputs.
 *
 * @details Waits for the first message, then loops forever playing the current message.
 *          After each full playback, checks for a new message via xTaskNotifyWait.
 *          If a notification is received, reloads encoded_buffer and plays the new message.
 *
 *          Hardware outputs toggled on each symbol :
 *            - GPIOB PIN_0 : LED.
 *            - GPIOC PIN_1 : speaker.
 *
 * @note   gLedAndSpeakerRunning is set to 1 on first iteration to allow task_uart
 *         to start sending notifications from the second message onward.
 *
 * @param  argument  Unused FreeRTOS task argument.
 */
void	task_led_and_speaker(void *argument)
{
	char	encoded_buffer[64][7];

	/* Wait for the first message before entering the playback loop. */
	set_buffer_led_and_speaker(encoded_buffer);

	for (;;)
	{
		size_t	encoded_index      = 0;
		size_t	encoded_index_line = 0;

		/* Signal task_uart that this task is now running — enables notifications. */
		gLedAndSpeakerRunning = 1;

		while (encoded_buffer[encoded_index_line][0])
		{
			while (encoded_buffer[encoded_index_line][encoded_index])
			{
				if (encoded_buffer[encoded_index_line][encoded_index] == '.')
				{
					/* Dot : 1 unit ON + 1 unit OFF. */
					HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
					osDelay(gState.SPEED);
					HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
					osDelay(gState.SPEED);
				}
				else if (encoded_buffer[encoded_index_line][encoded_index] == '-')
				{
					/* Dash : 3 units ON + 1 unit OFF. */
					HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
					osDelay(gState.SPEED * 3);
					HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
					osDelay(gState.SPEED);
				}
				else
				{
					/* Inter-word space : 4 units here + 3 inter-letter = 7 total. */
					osDelay(gState.SPEED * 4);
				}

				/* Inter-symbol gap : 1 unit. */
				osDelay(gState.SPEED);
				encoded_index++;
			}

			/* Inter-letter gap : 3 units. */
			osDelay(gState.SPEED * 3);
			encoded_index = 0;
			encoded_index_line++;
		}

		/* Inter-message gap : 7 units. */
		osDelay(gState.SPEED * 7);

		/* Check for new message — non-blocking. Reload buffer if notified. */
		if (xTaskNotifyWait(0, 0, NULL, 0) == pdTRUE)
		{
			memset(encoded_buffer, 0, sizeof(encoded_buffer));
			set_buffer_led_and_speaker(encoded_buffer);
		}
	}
}
