/*
 * task_button.c
 *
 *  Created on: 25 mars 2026
 *      Author: remyd
 */

#include "task_button.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13 && gState.STATE == ON && gState.LOGIN == LOGGED)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xSemaphoreButton, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void	task_button(void *argument)
{
	for (;;)
	{
		if (xSemaphoreTake(xSemaphoreButton, portMAX_DELAY) == pdTRUE)
		{
			xSemaphoreTake(xMutexStruct, portMAX_DELAY);

			vSendToPrintTask("\r\n");

			char	spd_buffer[4];
			itoa(gState.SPEED, spd_buffer, 10);
			vSendToPrintTask("SPEED = ");
			vSendToPrintTask(spd_buffer);
			vSendToPrintTask("\r\n");

			vSendToPrintTask("TRANSMIT HISTORIC = ");
			vSendToPrintTask("\r\n");
			if (gState.HISTORIC[0][0])
			{
				vSendToPrintTask(gState.HISTORIC[0]);
				vSendToPrintTask("\r\n");
			}
			if (gState.HISTORIC[1][0])
			{
				vSendToPrintTask(gState.HISTORIC[1]);
				vSendToPrintTask("\r\n");
			}
			if (gState.HISTORIC[2][0])
			{
				vSendToPrintTask(gState.HISTORIC[2]);
				vSendToPrintTask("\r\n");
			}

			xSemaphoreGive(xMutexStruct);
		}
	}
}
