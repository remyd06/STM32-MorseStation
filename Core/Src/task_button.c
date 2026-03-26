/**
 * @file    task_button.c
 * @brief   Button task — displays system state on B1 press.
 *
 * @details Pressing B1 (PC13) trigger an EXTI ISR which gives xSemaphoreButton.
 * 			A debouncer is here to avoid parasite push.
 *          task_button waits on this semaphore and prints the current system state
 *          via vSendToPrintTask() under mutex protection.
 *
 *          State displayed on button press :
 *            - Current morse speed (gState.SPEED, in ms).
 *            - Last 3 transmitted messages (gState.HISTORIC).
 *
 * @note   The ISR only gives the semaphore if the system is ON and LOGGED.
 *         Pressing the button while OFF or UNLOGGED has no effect.
 *
 * @author  remyd
 * @date    25 mars 2026
 */

#include "task_button.h"

/* Timestamp of the last button press — used for debouncing and timing in EXTI context */
volatile uint32_t	last_press_tick = 0;

/**
 * @brief  EXTI ISR callback — triggered on B1 falling edge (PC13).
 *
 * @details	Check if the push is higher than DEBOUNCE_DELAY.
 * 			Gives xSemaphoreButton to wake task_button.
 *          Only fires if the system is ON and the user is LOGGED.
 *          Yields immediately if a higher-priority task was woken.
 *
 * @param  GPIO_Pin  Pin that triggered the interrupt.
 */
void	HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_13 && gState.STATE == ON && gState.LOGIN == LOGGED)
	{
		uint32_t now = HAL_GetTick();
		if ((now - last_press_tick) > DEBOUNCE_DELAY)
		{
			last_press_tick = now;
			BaseType_t	xHigherPriorityTaskWoken = pdFALSE;

			xSemaphoreGiveFromISR(xSemaphoreButton, &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}

/**
 * @brief  Button task entry point — prints system state on each button press.
 *
 * @details Blocks on xSemaphoreButton indefinitely.
 *          On each press, acquires xMutexStruct and prints :
 *            - gState.SPEED  : current morse unit duration in ms.
 *            - gState.HISTORIC : last 3 transmitted messages, skips empty slots.
 *
 * @note   gState access is mutex-protected to prevent race conditions with task_uart.
 *         spd_buffer is sized 8 to safely hold any uint16_t value (max 65535).
 *
 * @param  argument  Unused FreeRTOS task argument.
 */
void	task_button(void *argument)
{
	for (;;)
	{
		if (xSemaphoreTake(xSemaphoreButton, portMAX_DELAY) == pdTRUE)
		{
			xSemaphoreTake(xMutexStruct, portMAX_DELAY);

			vSendToPrintTask("\r\n");

			/* Print current morse speed. */
			char	spd_buffer[8];
			itoa(gState.SPEED, spd_buffer, 10);
			vSendToPrintTask("SPEED = ");
			vSendToPrintTask(spd_buffer);
			vSendToPrintTask("\r\n");

			/* Print transmission historic — skip empty slots. */
			vSendToPrintTask("TRANSMIT HISTORIC =\r\n");

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
