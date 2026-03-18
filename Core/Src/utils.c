/*
 * utils.c
 *
 *  Created on: 17 mars 2026
 *      Author: remyd
 */

#include "utils.h"

void	vSendToPrintTask(char *str)
{
	size_t	i = 0;

	xSemaphoreTake(xMutexPrint, portMAX_DELAY);			// vu que cest atomique, xQueueSend est thread safe pour un appel mais dans ce cas la elle ne diffrencie pas si une suite dappel fait parti du meme message si cest atomique

	while (str[i])
		xQueueSend(xQueuePrint, &str[i++], portMAX_DELAY);
	xQueueSend(xQueuePrint, &str[i], portMAX_DELAY);

	xSemaphoreGive(xMutexPrint);
}
