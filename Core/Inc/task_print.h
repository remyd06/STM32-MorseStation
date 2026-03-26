/**
 * @file    task_print.h
 * @brief   Public interface for the print task.
 *
 * @details Exposes task_print() entry point.
 *          All UART transmissions must go through this task via vSendToPrintTask().
 *
 * @author  remyd
 * @date    14 mars 2026
 */

#ifndef INC_TASK_PRINT_H_
#define INC_TASK_PRINT_H_

# include "globals_var.h"

void task_print(void *argument);

#endif /* INC_TASK_PRINT_H_ */
