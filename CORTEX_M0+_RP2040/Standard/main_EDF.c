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
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/clocks.h"


#define mainON_BOARD_LED					( PICO_DEFAULT_LED_PIN )
#define TIME_SCALE                          (1000)

/*-----------------------------------------------------------*/

typedef enum
{
    LOGIC_GPIO_0 =  20,
    LOGIC_GPIO_1 =  21,
    LOGIC_GPIO_2 =  22,
    LOGIC_GPIO_3 =  26,
    LOGIC_GPIO_4 =  27,
    LOGIC_GPIO_5 =  28,
    LOGIC_GPIO_6 =  2,
} LogicAnalyzerGPIOS;

/*
 * Called by main when mainCREATE_SIMPLE_EDF_DEMO_ONLY is set to 1 in
 * main.c.
 */
void main_EDF( uint16_t led );

/*
 * The tasks as described in the comments at the top of this file.
 */
void prvTask( void *pvParameters );
static __noinline void ns_delay(uint32_t ns);
void delay_ms(uint32_t ms);
void initLogicGPIO(void);
void vTaskGetRunTimeStatsPrint( void );

/*-----------------------------------------------------------*/

static uint16_t externalLED = mainON_BOARD_LED;
char buffer[configSTATS_BUFFER_MAX_LENGTH];

/*-----------------------------------------------------------*/

void main_EDF( uint16_t led )
{
    printf(" Starting main_EDF.\n");
    externalLED = led;

    initLogicGPIO();

    uint32_t const task1C = 2 * TIME_SCALE;
    uint32_t const task2C = 2 * TIME_SCALE;
    uint32_t const task3C = 3 * TIME_SCALE;

    TaskHandle_t pxCreatedTask;
    xTaskCreateEDF( prvTask, "Task1", configMINIMAL_STACK_SIZE, (void*)&task1C, &pxCreatedTask, 4 * TIME_SCALE, 6 * TIME_SCALE);
    vTaskSetApplicationTaskTag(pxCreatedTask, ( void * ) (1u << LOGIC_GPIO_0));

    xTaskCreateEDF( prvTask, "Task2", configMINIMAL_STACK_SIZE, (void*)&task2C, &pxCreatedTask, 5 * TIME_SCALE, 8 * TIME_SCALE);
    vTaskSetApplicationTaskTag(pxCreatedTask, ( void * ) (1u << LOGIC_GPIO_1));

    xTaskCreateEDF( prvTask, "Task3", configMINIMAL_STACK_SIZE, (void*)&task3C, &pxCreatedTask, 7 * TIME_SCALE, 9 * TIME_SCALE);
    vTaskSetApplicationTaskTag(pxCreatedTask, ( void * ) (1u << LOGIC_GPIO_2));

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

	for( ;; );
}
/*-----------------------------------------------------------*/

void prvTask( void *pvParameters )
{
    TickType_t xInitialWakeTime = xTaskGetTickCount();
    long int num = *(long int *)pvParameters;

	for( ;; )
	{
        delay_ms(num);
        //vTaskGetRunTimeStatsPrint();
        vTaskDoneEDF(&xInitialWakeTime);
	}
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

/*-----------------------------------------------------------*/

void initLogicGPIO(void)
{
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
    gpio_init(LOGIC_GPIO_4);
    gpio_set_dir(LOGIC_GPIO_4, 1);
    gpio_put(LOGIC_GPIO_4, 0);
    gpio_init(LOGIC_GPIO_5);
    gpio_set_dir(LOGIC_GPIO_5, 1);
    gpio_put(LOGIC_GPIO_5, 0);
    gpio_init(LOGIC_GPIO_6);
    gpio_set_dir(LOGIC_GPIO_6, 1);
    gpio_put(LOGIC_GPIO_6, 0);
}

/*-----------------------------------------------------------*/

/* This example demonstrates how a human readable table of run time stats
   information is generated from raw data provided by uxTaskGetSystemState().
   The human readable table is written to pcWriteBuffer. (see the vTaskList()
   API function which actually does just this). */
void vTaskGetRunTimeStatsPrint( void )
{
    char * pcWriteBuffer = buffer;
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    unsigned long ulTotalRunTime, ulStatsAsPercentage;

   /* Make sure the write buffer does not contain a string. */
   *pcWriteBuffer = 0x00;

   /* Take a snapshot of the number of tasks in case it changes while this
      function is executing. */
   uxArraySize = uxTaskGetNumberOfTasks();
   printf("uxArraySize: %lu", uxArraySize);

   /* Allocate a TaskStatus_t structure for each task. An array could be
      allocated statically at compile time. */
   pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

   if( pxTaskStatusArray != NULL )
   {
      /* Generate raw status information about each task. */
      uxArraySize = uxTaskGetSystemState( pxTaskStatusArray,
                                 uxArraySize,
                                 &ulTotalRunTime );

      /* For percentage calculations. */
      ulTotalRunTime /= 100UL;

      /* Avoid divide by zero errors. */
      if( ulTotalRunTime > 0 )
      {
         /* For each populated position in the pxTaskStatusArray array,
            format the raw data as human readable ASCII data. */
         for( x = 0; x < uxArraySize; x++ )
         {
            /* What percentage of the total run time has the task used?
               This will always be rounded down to the nearest integer.
               ulTotalRunTimeDiv100 has already been divided by 100. */
            ulStatsAsPercentage =
                  pxTaskStatusArray[ x ].ulRunTimeCounter / ulTotalRunTime;

            if( ulStatsAsPercentage > 0UL )
            {
                printf("%stt%lutt%lu%%rn \n",
                                 pxTaskStatusArray[ x ].pcTaskName,
                                 pxTaskStatusArray[ x ].ulRunTimeCounter,
                                 ulStatsAsPercentage );
            }
            else
            {
               /* If the percentage is zero here then the task has
                  consumed less than 1% of the total run time. */
               printf("%stt%lutt<1%%rn \n",
                                 pxTaskStatusArray[ x ].pcTaskName,
                                 pxTaskStatusArray[ x ].ulRunTimeCounter );
            }
         }
      }

      /* The array is no longer needed, free the memory it consumes. */
      //vPortFree( pxTaskStatusArray );
   }
}


