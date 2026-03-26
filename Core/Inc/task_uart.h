/**
 * @file    task_uart.h
 * @brief   Public interface for the UART task.
 *
 * @details Exposes task_uart() entry point.
 *          Includes all dependencies required by the task.
 *
 * @author  remyd
 * @date    14 mars 2026
 */

#ifndef INC_TASK_UART_H_
#define INC_TASK_UART_H_

# include "globals_var.h"
# include "main.h"

void task_uart(void *argument);

#endif /* INC_TASK_UART_H_ */
