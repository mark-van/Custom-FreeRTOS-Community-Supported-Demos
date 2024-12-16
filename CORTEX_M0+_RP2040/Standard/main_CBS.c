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
#include "timers.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"

/* Library includes. */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define mainON_BOARD_LED					( PICO_DEFAULT_LED_PIN )
#define NUM_TIMERS                          ( 3 )
#define MAX_NO_OVERFLOW                     (0xffffffff / 1000)
#define TIME_SCALE                          (100)
/*-----------------------------------------------------------*/

typedef enum
{
    LOGIC_GPIO_0 =  20,
    LOGIC_GPIO_1 =  21,
    LOGIC_GPIO_2 =  22,
    LOGIC_GPIO_3 =  26,
} LogicAnalyzerGPIOS;

TimerHandle_t xTimers[ NUM_TIMERS ];
UBaseType_t indexCBS;
long int num[ NUM_TIMERS ];
static uint8_t jobIndex;

/*
 * Called by main when mainCREATE_SIMPLE_EDF_DEMO_ONLY is set to 1 in
 * main.c.
 */
void main_CBS( uint16_t led );

/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvTask1( void *pvParameters );
static void prvJob( void *pvParameters );
void vTimerJobCallback( TimerHandle_t xTimer );
uint64_t nonBlockingDelay( void );
uint64_t delayValue;
static __noinline void ns_delay(uint32_t ns);
void delay_ms(uint32_t ms);

/*-----------------------------------------------------------*/

static uint16_t externalLED = mainON_BOARD_LED;

/*-----------------------------------------------------------*/

void main_CBS( uint16_t led )
{
    printf(" Starting main_CBS.\n");
    externalLED = led;

    gpio_init(LOGIC_GPIO_0);
    gpio_set_dir(LOGIC_GPIO_0, 1);
    gpio_put(LOGIC_GPIO_0, 0);
    gpio_init(LOGIC_GPIO_1);
    gpio_set_dir(LOGIC_GPIO_1, 1);
    gpio_put(LOGIC_GPIO_1, 0);
    gpio_init(LOGIC_GPIO_2);
    gpio_set_dir(LOGIC_GPIO_2, 1);
    gpio_put(LOGIC_GPIO_2, 0);
    gpio_init(LOGIC_GPIO_3);
    gpio_set_dir(LOGIC_GPIO_3, 1);
    gpio_put(LOGIC_GPIO_3, 0);

    uint16_t task1DeadlineMS = 3 * TIME_SCALE;
    uint16_t task1PeriodMS = 3 * TIME_SCALE;

    TaskHandle_t pxCreatedTask;
    xTaskCreateEDF( prvTask1, "Task1", configMINIMAL_STACK_SIZE, NULL, &pxCreatedTask, task1DeadlineMS, task1PeriodMS);
    vTaskSetApplicationTaskTag(pxCreatedTask, ( void * ) (1u << LOGIC_GPIO_0));

    UBaseType_t maxBudget = 2 * TIME_SCALE;
    UBaseType_t serverPeriod = 7 * TIME_SCALE;
 
    xTaskCreateCBS( "CBS Task", configMINIMAL_STACK_SIZE, &pxCreatedTask, &indexCBS, maxBudget, serverPeriod);
    vTaskSetApplicationTaskTag(pxCreatedTask, ( void * ) (1u << LOGIC_GPIO_1));

    xTimers[0] = xTimerCreate   ( 
                                    "3", // provide number indicating computation time of job
                                    pdMS_TO_TICKS(2 * TIME_SCALE),
                                    pdFALSE,
                                    ( void * ) 0,
                                    vTimerJobCallback
                                );

    xTimers[1] = xTimerCreate   ( 
                                    "2", // provide number indicating computation time of job
                                    pdMS_TO_TICKS(7 * TIME_SCALE),
                                    pdFALSE,
                                    ( void * ) 0,
                                    vTimerJobCallback
                                );

    xTimers[2] = xTimerCreate   ( 
                                    "1", // provide number indicating computation time of job
                                    pdMS_TO_TICKS(17 * TIME_SCALE),
                                    pdFALSE,
                                    ( void * ) 0,
                                    vTimerJobCallback
                                );

    printf("About to start scheduler\n");
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
        delay_ms(2 * TIME_SCALE);
        vTaskDoneEDF(&xInitialWakeTime);
	}
}

/*-----------------------------------------------------------*/

static void prvJob( void *pvParameters )
{
    long int num = *(long int *)pvParameters;
    delay_ms(num * TIME_SCALE);
}

/*-----------------------------------------------------------*/

void vTimerJobCallback( TimerHandle_t xTimer )
{
    char *endptr;
    num[jobIndex] = strtol(pcTimerGetName(xTimer), &endptr, 10);
    xTaskCreateJobCBS( prvJob, &num[jobIndex], indexCBS);
    jobIndex++;
}

/*-----------------------------------------------------------*/

void delay_ms(uint32_t ms) 
{
    for (int k = 0; k < ms; k++)
    {
        ns_delay(1000000);
    }
}

/*-----------------------------------------------------------*/

static __noinline void ns_delay(uint32_t ns) {
    // cycles = ns * clk_sys_hz / 1,000,000,000
    uint32_t cycles = ns * (clock_get_hz(clk_sys) >> 16u) / (1000000000u >> 16u);
    busy_wait_at_least_cycles(cycles);
}