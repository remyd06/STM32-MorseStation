/*
 * globals_var.c
 *
 *  Created on: 14 mars 2026
 *      Author: remyd
 */

#include "globals_var.h"

state_t gState = {OFF, UNLOGGED, 150, {{0},{0},{0}}};

SemaphoreHandle_t	xMutexPrint;
SemaphoreHandle_t	xMutexStruct;
SemaphoreHandle_t	xSemaphoreButton;
SemaphoreHandle_t	xSemaphoreUart;





