/*
 * task_encoder.c
 *
 *  Created on: 18 mars 2026
 *      Author: remyd
 */

#include "task_encoder.h"

void	set_buffer_encoder(char	*encoder_buffer)
{
	char		q_byte = 0;
	size_t		encoder_index = 0;

	while (1)
	{
		xQueueReceive(xQueueEncoder, &q_byte, portMAX_DELAY);

		if (q_byte != '\0')
			encoder_buffer[encoder_index++] = q_byte;
		else
		{
			encoder_buffer[encoder_index] = q_byte;
			break ;
		}
	}
}

void	task_encoder(void *argument)
{
	char		encoder_buffer[64];
	uint8_t		historic_index = 0;

	static const char	*morse_alphabet[] =
	{
			".-",		// A
			"-...",		// B
			"-.-.",		// C
			"-..",		// D
			".",		// E
			"..-.",		// F
			"--.",		// G
			"....",		// H
			"..",		// I
			".---",		// J
			"-.-",		// K
			".-..",		// L
			"--",		// M
			"-.",		// N
			"---",		// O
			".--.",		// P
			"--.-",		// Q
			".-.",		// R
			"...",		// S
			"-",		// T
			"..-",		// U
			"...-",		// V
			".--",		// W
			"-..-",		// X
			"-.--",		// Y
			"--.."		// Z
	};

	static const char	*morse_num[] =
	{
			"-----",	// 0
			".----",	// 1
			"..---",	// 2
			"...--",	// 3
			"....-",	// 4
			".....",	// 5
			"-....",	// 6
			"--...",	// 7
			"---..",	// 8
			"----."		// 9
	};

	for (;;)
	{
		set_buffer_encoder(encoder_buffer);

		const char	*encoded_transmit[64];
		size_t		i = 0;

		while (encoder_buffer[i])
		{
			if (!isalnum((unsigned char)encoder_buffer[i]) && encoder_buffer[i] != ' ')
			{
				vSendToPrintTask("[ERROR] YOUR COMMAND CAN ONLY CONTAIN ALPHANUMERIC CHAR.\r\n");
				memset(encoder_buffer, 0, strlen(encoder_buffer));
				break ;
			}
			i++;
		}

		i = 0;
		while (encoder_buffer[i])
		{
			if (isalpha((unsigned char)encoder_buffer[i]))
				encoded_transmit[i] = morse_alphabet[encoder_buffer[i] - 'A'];

			else if (isdigit((unsigned char)encoder_buffer[i]))
				encoded_transmit[i] = morse_num[encoder_buffer[i] - '0'];

			else if (encoder_buffer[i] == ' ')
				encoded_transmit[i] = "|";

			i++;
		}
		encoded_transmit[i] = NULL;

		i = 0;
		char	sep = '\r';
		char	end = '\0';
		while (encoded_transmit[i])
		{
			size_t	j = 0;
			while (encoded_transmit[i][j])
				xQueueSend(xQueueLedAndSpeaker, &encoded_transmit[i][j++], portMAX_DELAY);
			xQueueSend(xQueueLedAndSpeaker, &sep, portMAX_DELAY);
			i++;
		}
		xQueueSend(xQueueLedAndSpeaker, &end, portMAX_DELAY);

		if (historic_index == 3)
			historic_index = 0;

		xSemaphoreTake(xMutexStruct, portMAX_DELAY);

		if (!encoder_buffer[0] && gLedAndSpeakerRunning)
			strcpy(gState.HISTORIC[historic_index++], "/ERROR/");
		else
			strcpy(gState.HISTORIC[historic_index++], encoder_buffer);

		xSemaphoreGive(xMutexStruct);

	}
}
