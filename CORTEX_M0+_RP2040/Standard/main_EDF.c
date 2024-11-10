/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 *
 * main_EDF() creates two EDF tasks. It then starts the scheduler.
 *
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Library includes. */
#include <stdio.h>
#include "hardware/gpio.h"


#define mainON_BOARD_LED					( PICO_DEFAULT_LED_PIN )

/*-----------------------------------------------------------*/

/*
 * Called by main when mainCREATE_SIMPLE_EDF_DEMO_ONLY is set to 1 in
 * main.c.
 */
void main_EDF( uint16_t led );

/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvTask1( void *pvParameters );
static void prvTask2( void *pvParameters );

/*-----------------------------------------------------------*/

static uint16_t externalLED = mainON_BOARD_LED;

/*-----------------------------------------------------------*/

void main_EDF( uint16_t led )
{
    printf(" Starting main_EDF.\n");
    externalLED = led;

    uint16_t task1DeadlineMS = 100;
    uint16_t task1PeriodMS = 1000;
    uint16_t task2DeadlineMS = 200;
    uint16_t task2PeriodMS = 2000;


    xTaskCreateEDF( prvTask1, "Task1", configMINIMAL_STACK_SIZE, NULL, NULL, task1DeadlineMS, task1PeriodMS);

    xTaskCreateEDF( prvTask2, "Task2", configMINIMAL_STACK_SIZE, NULL, NULL, task2DeadlineMS, task2PeriodMS);

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvTask1( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

    TickType_t xInitialWakeTime = xTaskGetTickCount();

	for( ;; )
	{
        printf("prvTask1 Start\n");
        gpio_xor_mask( 1u << externalLED );
        printf("prvTask1 DONE\n");
        vTaskDoneEDF(&xInitialWakeTime);
	}
}
/*-----------------------------------------------------------*/

static void prvTask2( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

    TickType_t xInitialWakeTime = xTaskGetTickCount();

	for( ;; )
	{
        printf("prvTask2 Start\n");
        gpio_xor_mask( 1u << mainON_BOARD_LED );
        printf("prvTask2 DONE\n");
        vTaskDoneEDF(&xInitialWakeTime);
	}
}
/*-----------------------------------------------------------*/
