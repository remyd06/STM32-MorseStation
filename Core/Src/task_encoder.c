/**
 * @file    task_encoder.c
 * @brief   Morse encoding task — converts alphanumeric strings to morse sequences.
 *
 * @details Receives plain text from xQueueEncoder (sent by task_uart),
 *          validates input, encodes each character to its morse equivalent,
 *          and forwards the encoded sequence to xQueueLedAndSpeaker.
 *
 *          Encoding format sent to xQueueLedAndSpeaker :
 *            - Each morse symbol byte ('.', '-', '|') sent individually.
 *            - '\r' separates symbols of different characters.
 *            - '\0' terminates the full message.
 *
 *          The last 3 transmitted messages are stored in gState.HISTORIC.
 *          If the input was invalid, "/ERROR/" is stored instead.
 *
 * @author  remyd
 * @date    18 mars 2026
 */

#include "task_encoder.h"

/**
 * @brief  Fills encoder_buffer from xQueueEncoder until '\0' is received.
 *
 * @details Blocks on xQueueReceive until a full string is available.
 *          '\0' is used as end-of-string sentinel — consistent with task_uart send.
 *
 * @param  encoder_buffer  Destination buffer. Must be at least 64 bytes.
 */
void	set_buffer_encoder(char *encoder_buffer)
{
	char	q_byte        = 0;
	size_t	encoder_index = 0;

	while (1)
	{
		xQueueReceive(xQueueEncoder, &q_byte, portMAX_DELAY);

		if (q_byte != '\0')
			encoder_buffer[encoder_index++] = q_byte;
		else
		{
			encoder_buffer[encoder_index] = q_byte;
			break;
		}
	}
}

/**
 * @brief  Encoder task entry point — validates, encodes and forwards morse sequences.
 *
 * @details Loops forever. Each iteration :
 *
 *          1. Reads a plain text string from xQueueEncoder via set_buffer_encoder().
 *          2. Validates input — only alphanumeric characters and spaces are accepted.
 *             On error, sends an error message and skips encoding.
 *          3. Encodes each character using morse_alphabet[] or morse_num[] lookup tables.
 *             Spaces are encoded as "|" (inter-word separator).
 *          4. Forwards the encoded sequence byte by byte to xQueueLedAndSpeaker,
 *             with '\r' between symbols and '\0' as end-of-message sentinel.
 *          5. Stores the message (or "/ERROR/") in gState.HISTORIC under mutex protection.
 *
 * @note   encoder_buffer must contain uppercase characters — task_uart handles conversion.
 *         morse_alphabet[] is indexed from 'A' (65), morse_num[] from '0' (48).
 *
 * @param  argument  Unused FreeRTOS task argument.
 */
void	task_encoder(void *argument)
{
	char	encoder_buffer[64];
	uint8_t	historic_index = 0;

	/* Morse lookup tables — indexed by character offset from 'A' or '0'. */
	static const char	*morse_alphabet[] =
	{
		".-",    // A
		"-...",  // B
		"-.-.",  // C
		"-..",   // D
		".",     // E
		"..-.",  // F
		"--.",   // G
		"....",  // H
		"..",    // I
		".---",  // J
		"-.-",   // K
		".-..",  // L
		"--",    // M
		"-.",    // N
		"---",   // O
		".--.",  // P
		"--.-",  // Q
		".-.",   // R
		"...",   // S
		"-",     // T
		"..-",   // U
		"...-",  // V
		".--",   // W
		"-..-",  // X
		"-.--",  // Y
		"--.."   // Z
	};

	static const char	*morse_num[] =
	{
		"-----", // 0
		".----", // 1
		"..---", // 2
		"...--", // 3
		"....-", // 4
		".....", // 5
		"-....", // 6
		"--...", // 7
		"---..", // 8
		"----."  // 9
	};

	for (;;)
	{
		set_buffer_encoder(encoder_buffer);

		const char	*encoded_transmit[64];
		size_t		i = 0;

		/* Validate input — only alphanumeric characters and spaces are accepted. */
		while (encoder_buffer[i])
		{
			if (!isalnum((unsigned char)encoder_buffer[i]) && encoder_buffer[i] != ' ')
			{
				vSendToPrintTask("[ERROR] YOUR COMMAND CAN ONLY CONTAIN ALPHANUMERIC CHAR.\r\n");
				memset(encoder_buffer, 0, strlen(encoder_buffer));
				break;
			}
			i++;
		}

		/* Encode each character to its morse equivalent. */
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

		/* Forward encoded sequence to xQueueLedAndSpeaker. */
		i = 0;
		char sep = '\r';
		char end = '\0';
		while (encoded_transmit[i])
		{
			size_t	j = 0;
			while (encoded_transmit[i][j])
				xQueueSend(xQueueLedAndSpeaker, &encoded_transmit[i][j++], portMAX_DELAY);
			xQueueSend(xQueueLedAndSpeaker, &sep, portMAX_DELAY);
			i++;
		}
		xQueueSend(xQueueLedAndSpeaker, &end, portMAX_DELAY);

		/* Store message in historic — circular buffer of 3 entries. */
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
