/**
 * @file    task_button.h
 * @brief   Public interface for the button task.
 *
 * @details Exposes task_button() entry point.
 *          Pressing B1 (PC13) triggers an EXTI ISR that wakes task_button
 *          via xSemaphoreButton, displaying the current system state over UART.
 *
 * @author  remyd
 * @date    25 mars 2026
 */

#ifndef INC_TASK_BUTTON_H_
#define INC_TASK_BUTTON_H_

# include "globals_var.h"

void	task_button(void *argument);

#endif /* INC_TASK_BUTTON_H_ */
