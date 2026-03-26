/**
 * @file    task_encoder.h
 * @brief   Public interface for the encoder task.
 *
 * @details Exposes task_encoder() entry point.
 *          Receives plain text from xQueueEncoder and forwards
 *          morse-encoded sequences to xQueueLedAndSpeaker.
 *
 * @author  remyd
 * @date    18 mars 2026
 */

#ifndef INC_TASK_ENCODER_H_
#define INC_TASK_ENCODER_H_

# include "globals_var.h"

void	task_encoder(void *argument);

#endif /* INC_TASK_ENCODER_H_ */
