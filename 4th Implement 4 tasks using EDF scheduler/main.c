/*
 * FreeRTOS V202112.00
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
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>	

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "queue.h"						

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

//Tasks Prototypes
void Button_1_Monitor( void * pvParameters );
void Button_2_Monitor( void * pvParameters );
void Periodic_Transmitter (void * pvParameters );
void Uart_Receiver (void * pvParameters );
void Load_1_Simulation ( void * pvParameters );
void Load_2_Simulation ( void * pvParameters );

//Handlers Declarations
TaskHandle_t Button_1_Handler = NULL;
TaskHandle_t Button_2_Handler = NULL;
TaskHandle_t Transmitter_Handler = NULL;
TaskHandle_t Receiver_Handler = NULL;
TaskHandle_t Load1_Handler = NULL;
TaskHandle_t Load2_Handler = NULL;
QueueHandle_t Button1_Queue = NULL;
QueueHandle_t Button2_Queue = NULL;
QueueHandle_t Periodic_Transmitter_Queue = NULL;			   

//Variables for Run-time analysis
uint32_t Tin_Button1;
uint32_t Tin_Button2;
uint32_t Tin_Periodic_Transmitter;
uint32_t Tin_UART_Reciever;
uint32_t Tin_Load1;
uint32_t Tin_Load2;

uint32_t Tout_Button1;
uint32_t Tout_Button2;
uint32_t Tout_Periodic_Transmitter;
uint32_t Tout_UART_Reciever;
uint32_t Tout_Load1;
uint32_t Tout_Load2;

uint32_t Total_execution_time;
uint32_t CPU_Load;

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	//Queues that hold messages
	Button1_Queue = xQueueCreate(1, sizeof(char));
	Button2_Queue = xQueueCreate(1, sizeof(char));
	Periodic_Transmitter_Queue = xQueueCreate(14, sizeof(char));
	
    /* Create Tasks here */
	//Button_1_Monitor Task
	xTaskPeriodicCreate(
        Button_1_Monitor,        /* Function that implements the task. */
        "Button1",     					/* Text name for the task. */
        100,      			        	/* Stack size in words, not bytes. */
        ( void * ) 0,         		/* Parameter passed into the task. */
        1,						            /* Priority at which the task is created. */
        &Button_1_Handler		    /* Used to pass out the created task's handle. */
        ,50);      				    /* Task Periodicity */
	
	//Button_2_Monitor Task
	xTaskPeriodicCreate(
        Button_2_Monitor,        /* Function that implements the task. */
        "Button2",     					/* Text name for the task. */
        100,      			        	/* Stack size in words, not bytes. */
        ( void * ) 0,         		/* Parameter passed into the task. */
        1,						            /* Priority at which the task is created. */
        &Button_2_Handler		    /* Used to pass out the created task's handle. */
        ,50);      				    /* Task Periodicity */
		
	//Periodic_Transmitter Task
	xTaskPeriodicCreate(
        Periodic_Transmitter,        /* Function that implements the task. */
        "Transmitter",     					/* Text name for the task. */
        100,      			        	/* Stack size in words, not bytes. */
        ( void * ) 0,         		/* Parameter passed into the task. */
        1,						            /* Priority at which the task is created. */
        &Transmitter_Handler		    /* Used to pass out the created task's handle. */
        ,100);      				    /* Task Periodicity */
		
	//Uart_Receiver Task
	xTaskPeriodicCreate(
        Uart_Receiver,        /* Function that implements the task. */
        "Receiver",     					/* Text name for the task. */
        100,      			        	/* Stack size in words, not bytes. */
        ( void * ) 0,         		/* Parameter passed into the task. */
        1,						            /* Priority at which the task is created. */
        &Receiver_Handler		    /* Used to pass out the created task's handle. */
        ,20);      				    /* Task Periodicity */
		
	//Load_1_Simulation Task
	xTaskPeriodicCreate(
        Load_1_Simulation,        /* Function that implements the task. */
        "Load1",     					/* Text name for the task. */
        100,      			        	/* Stack size in words, not bytes. */
        ( void * ) 0,         		/* Parameter passed into the task. */
        1,						            /* Priority at which the task is created. */
        &Load1_Handler		    /* Used to pass out the created task's handle. */
        ,10);      				    /* Task Periodicity */
	
	//Load_2_Simulation Task
	xTaskPeriodicCreate(
        Load_2_Simulation,        /* Function that implements the task. */
        "Load2",     					/* Text name for the task. */
        100,      			        	/* Stack size in words, not bytes. */
        ( void * ) 0,         		/* Parameter passed into the task. */
        1,						            /* Priority at which the task is created. */
        &Load2_Handler		    /* Used to pass out the created task's handle. */
        ,100);      				    /* Task Periodicity */	
		
	
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;  
}
//Application Tick Hook to monitor every tick
void vApplicationTickHook (void)
{
	//Make a rising edge to monitor the task's beginning
	GPIO_write(PORT_0 , PIN0, PIN_IS_HIGH);
	
	//Make a falling edge to monitor the task's end
	GPIO_write(PORT_0 , PIN0, PIN_IS_LOW);
}	

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);
	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/

//Tasks Implementations
//Button_1_Monitor Task
void Button_1_Monitor( void * pvParameters )
{	
	TickType_t xLastWakeTime = xTaskGetTickCount();
	pinState_t Button_1_CurState;
				
	//Read GPIO Input for previous buttom 1 state
	pinState_t Button_1_PrevState = GPIO_read(PORT_1, PIN0);
	signed char Edge_Flag = '0';

	for( ;; )
	{
		//Make a rising edge to monitor the task's beginning
		//GPIO_write(PORT_0 , PIN1, PIN_IS_HIGH);
	
		//Read GPIO Input for current buttom 1 state
		Button_1_CurState = GPIO_read(PORT_1 , PIN0);
		
		//Check if there is an Edge found
		if(Button_1_CurState != Button_1_PrevState)
		{
			//An Edge is found
			if(Button_1_CurState == PIN_IS_HIGH)
			{
				//Positive edge
				Edge_Flag = '+';
			}
			else
			{
				//Negative edge
				Edge_Flag = '-';
			}
		}
		else
		{
			//There is no edge found yet
			Edge_Flag = '0';
		}
	
		//Save the Edge state to be sent to the consumer task
		xQueueOverwrite(Button1_Queue, &Edge_Flag); 
		                         
		//Update the previous buttom 1 state to equal to the current one
		Button_1_PrevState = Button_1_CurState;	                                          
	                                          
		//Make a falling edge to monitor the task's end
		//GPIO_write(PORT_0 , PIN1, PIN_IS_LOW);
											  
		//Delay Button_1_Monitor for 50ms
		vTaskDelayUntil(&xLastWakeTime , 50);
	}
}

//Button_2_Monitor Task
void Button_2_Monitor( void * pvParameters )
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	pinState_t Button_2_CurState;
	
	//Read GPIO Input for previous buttom 2 state
	pinState_t Button_2_PrevState = GPIO_read(PORT_1, PIN1);
	signed char Edge_Flag = '0';

	for( ;; )
	{
		//Make a rising edge to monitor the task's beginning
		//GPIO_write(PORT_0 , PIN2, PIN_IS_HIGH);
	
		//Read GPIO Input for current buttom 2 state
		Button_2_CurState = GPIO_read(PORT_1 , PIN1);
		
		//Check if there is an Edge found
		if(Button_2_CurState != Button_2_PrevState)
		{
			//An Edge is found
			if(Button_2_CurState == PIN_IS_HIGH)
			{
				//Positive edge
				Edge_Flag = '+';
			}
			else
			{
				//Negative edge
				Edge_Flag = '-';
			}
		}
		else
		{
			//There is no edge found yet
			Edge_Flag = '0';
		}
	
		//Save the Edge state to be sent to the consumer task
		xQueueOverwrite(Button2_Queue, &Edge_Flag); 
		                         
		//Update the previous buttom 2 state to equal to the current one
		Button_2_PrevState = Button_2_CurState;	                                          
	                                          
		//Make a falling edge to monitor the task's end
		//GPIO_write(PORT_0 , PIN2, PIN_IS_LOW);
											  
		//Delay Button_2_Monitor for 50ms
		vTaskDelayUntil(&xLastWakeTime , 50);
	}
}

//Periodic_Transmitter Task
void Periodic_Transmitter(void * pvParameters )
{
	uint32_t counter = 0;
	TickType_t xLastWakeTime = xTaskGetTickCount();
		
	char Periodic_String[14];
	strcpy(Periodic_String, "\n100ms string");
	
	for( ; ; )
	{
		//Make a rising edge to monitor the task's beginning
		//GPIO_write(PORT_0 , PIN3, PIN_IS_HIGH);
		
		//send preiodic string every 100ms to the consumer task to be send to UART
		for(counter=0 ; counter<14 ; counter++)
		{
			xQueueSend(Periodic_Transmitter_Queue, (Periodic_String + counter), 100);
		}
		
		//Make a falling edge to monitor the task's end
		//GPIO_write(PORT_0 , PIN3, PIN_IS_LOW);
		
		//Delay Periodic_Transmitter for 100ms
		vTaskDelayUntil(&xLastWakeTime, 100);
	}
}

//Uart_Receiver Task
void Uart_Receiver(void * pvParameters )
{
	uint32_t counter = 0;
	TickType_t xLastWakeTime = xTaskGetTickCount();
		
	signed char Button1_edge;
	signed char Button2_edge;
	char Periodic_String[14];
		
	for( ; ; )
	{
		//Make a rising edge to monitor the task's beginning
		//GPIO_write(PORT_0 , PIN4, PIN_IS_HIGH);
		
		//Receive Button 1 State from Button_1_Monitor to check edge flag
		if( xQueueReceive( Button1_Queue, &Button1_edge, 0))
		{
			if(Button1_edge == '+')
			{
				//Positive Edge is Received, Send message to UART
				xSerialPutChar(Button1_edge);
				vSerialPutString( (signed char *) "ve Edge in Button1", 20);
			}
			else if(Button1_edge == '-')
			{
				//Negative Edge is Received, Send message to UART
				xSerialPutChar(Button1_edge);
				vSerialPutString( (signed char *) "ve Edge in Button1", 20);
			}
			else
			{
				//do nothing
			}
		}
		
		//Receive Button 2 State from Button_2_Monitor to check edge flag
		if( xQueueReceive( Button2_Queue, &Button2_edge, 0))
		{
			if(Button2_edge == '+')
			{
				//Positive Edge is Received, Send message to UART
				xSerialPutChar(Button2_edge);
				vSerialPutString( (signed char *) "ve Edge in Button2", 20);
			}
			else if(Button2_edge == '-')
			{
				//Negative Edge is Received, Send message to UART
				xSerialPutChar(Button2_edge);
				vSerialPutString( (signed char *) "ve Edge in Button2", 20);
			}
			else
			{
				//do nothing
			}
		}
		
		//Receive Periodic String from Periodic_Transmitter to send it to UART
		if( uxQueueMessagesWaiting(Periodic_Transmitter_Queue) != 0 )
		{
			//read preiodic string every 100ms to be send to UART
			for( counter=0 ; counter<14 ; counter++)
			{
				xQueueReceive(Periodic_Transmitter_Queue, (Periodic_String + counter), 0);
			}
			
			vSerialPutString( (signed char *) Periodic_String, 14);
			xQueueReset(Periodic_Transmitter_Queue);
		}

		//Make a falling edge to monitor the task's end
		//GPIO_write(PORT_0 , PIN4, PIN_IS_LOW);
		
		//Delay Uart_Receiver for 20ms
		vTaskDelayUntil(&xLastWakeTime, 20);
	}
}

//Load_1_Simulation Task
void Load_1_Simulation ( void * pvParameters )
{
	uint32_t counter = 0;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	
	//Using XTAL = 12Mhz, 1ms = (12*1000*1000)/(1000) = 12000
	uint32_t _1ms = 12000;
	uint32_t Execution_time = 5 * _1ms; //(5 * _1ms)
		
	for( ; ; )
	{		
		//Make a rising edge to monitor the task's beginning
		//GPIO_write(PORT_0 , PIN5, PIN_IS_HIGH);
		
		//Represents 5ms of delay (Execution time)
		for( counter=0 ; counter <= Execution_time ; counter++)
		{
			//do nothing
		}

		//Make a falling edge to monitor the task's end
		//GPIO_write(PORT_0 , PIN5, PIN_IS_LOW);
		
		//Delay Load_1_Simulation for 10ms
		vTaskDelayUntil(&xLastWakeTime, 10);
	}
}

//Load_2_Simulation Task
void Load_2_Simulation ( void * pvParameters )
{
	uint32_t counter = 0;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	
	//Using XTAL = 12Mhz, 1ms = (12*1000*1000)/(1000) = 12000
	uint32_t _1ms = 12000;
	uint32_t Execution_time = 12 * _1ms; //(12 * _1ms)
		
	for( ; ; )
	{		
		//Make a rising edge to monitor the task's beginning
		//GPIO_write(PORT_0 , PIN6, PIN_IS_HIGH);
		
		//Represents 12ms of delay (Execution time)
		for( counter=0 ; counter <= Execution_time ; counter++)
		{
			//do nothing
		}

		//Make a falling edge to monitor the task's end
		//GPIO_write(PORT_0 , PIN6, PIN_IS_LOW);
		
		//Delay Load_2_Simulation for 100ms
		vTaskDelayUntil(&xLastWakeTime, 100);
	}
}
