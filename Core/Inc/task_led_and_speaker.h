/**
 * @file    task_led_and_speaker.h
 * @brief   Public interface for the LED and speaker task.
 *
 * @details Exposes task_led_and_speaker() entry point.
 *          Plays morse sequences received from xQueueLedAndSpeaker
 *          on the onboard LEDs and speaker.
 *
 * @author  remyd
 * @date    20 mars 2026
 */

#ifndef INC_TASK_LED_AND_SPEAKER_H_
#define INC_TASK_LED_AND_SPEAKER_H_

# include "globals_var.h"

void	task_led_and_speaker(void *argument);

#endif /* INC_TASK_LED_AND_SPEAKER_H_ */
