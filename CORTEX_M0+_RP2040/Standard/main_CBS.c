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
 * main_CBS() creates two CBS tasks. It then starts the scheduler.
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
void main_CBS( uint16_t led );

/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvTask1( void *pvParameters );
static void prvTask2( void *pvParameters );
static void prvJob1( void *pvParameters );

/*-----------------------------------------------------------*/

static uint16_t externalLED = mainON_BOARD_LED;

/*-----------------------------------------------------------*/

void main_CBS( uint16_t led )
{
    printf(" Starting main_CBS.\n");
    externalLED = led;

    uint16_t task1DeadlineMS = 100;
    uint16_t task1PeriodMS = 1000;
    uint16_t task2DeadlineMS = 200;
    uint16_t task2PeriodMS = 2000;

    xTaskCreateEDF( prvTask1, "Task1", configMINIMAL_STACK_SIZE, NULL, NULL, task1DeadlineMS, task1PeriodMS);

    xTaskCreateEDF( prvTask2, "Task2", configMINIMAL_STACK_SIZE, NULL, NULL, task2DeadlineMS, task2PeriodMS);

    UBaseType_t indexCBS;
    UBaseType_t maxBudget = 100;
    UBaseType_t serverPeriod = 1000;
 
    xTaskCreateCBS( "CBS Task", configMINIMAL_STACK_SIZE, NULL, &indexCBS, maxBudget, serverPeriod);

    xTaskCreateJobCBS( prvJob1, NULL, indexCBS);
    xTaskCreateJobCBS( prvJob1, NULL, indexCBS);
    xTaskCreateJobCBS( prvJob1, NULL, indexCBS);
    xTaskCreateJobCBS( prvJob1, NULL, indexCBS);

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


        // Task execution time
        TickType_t currentTick = xTaskGetTickCount();
        while(xTaskGetTickCount() - currentTick < pdMS_TO_TICKS(500))
        {
            // delay 0.5 second
            //..this doesnt rellr wor ktho, i need a delay that requries the
            // process to stay in the task
        }

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

        // Task execution time
        TickType_t currentTick = xTaskGetTickCount();
        while(xTaskGetTickCount() < pdMS_TO_TICKS(1250) + currentTick)
        {
            // delay 1.25 second
        }

        gpio_xor_mask( 1u << mainON_BOARD_LED );
        printf("prvTask2 DONE\n");
        vTaskDoneEDF(&xInitialWakeTime);
	}
}
/*-----------------------------------------------------------*/

static void prvJob1( void *pvParameters )
{
	( void ) pvParameters;

    printf("prvJob1 Start\n");

    //TickType_t currentTick = xTaskGetTickCount();
    uint64_t inc = 0;
    while(inc < (1 << 3))
    {
        // delay 0.5 second
        inc++;
        printf("prvJob1  %llu \n", inc);
    }

    printf("prvJob1 DONE\n");

}
/*-----------------------------------------------------------*/
