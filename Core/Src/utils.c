/**
 * @file    utils.c
 * @brief   Utility functions shared across all tasks.
 *
 * @author  remyd
 * @date    17 mars 2026
 */

#include "utils.h"

/**
 * @brief  Sends a null-terminated string to task_print via xQueuePrint.
 *
 * @details Pushes each byte of str into xQueuePrint, terminated by '\0'.
 *          xMutexPrint ensures the full string is sent atomically — without it,
 *          concurrent calls from different tasks would interleave their bytes
 *          and corrupt the output.
 *
 * @note   Blocks until the mutex is available and all bytes are enqueued.
 *
 * @param  str  Null-terminated string to send. Must not be NULL.
 */
void	vSendToPrintTask(char *str)
{
	size_t	i = 0;

	xSemaphoreTake(xMutexPrint, portMAX_DELAY);

	while (str[i])
		xQueueSend(xQueuePrint, &str[i++], portMAX_DELAY);
	xQueueSend(xQueuePrint, &str[i], portMAX_DELAY);

	xSemaphoreGive(xMutexPrint);
}
