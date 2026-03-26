/**
 * @file    utils.h
 * @brief   Public interface for shared utility functions.
 *
 * @details Exposes vSendToPrintTask() — the only sanctioned way to transmit
 *          strings over UART from any task.
 *
 * @author  remyd
 * @date    17 mars 2026
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

# include "globals_var.h"

void	vSendToPrintTask(char *str);

#endif /* INC_UTILS_H_ */
