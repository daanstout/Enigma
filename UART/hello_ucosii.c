/*************************************************************************
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.      *
* All rights reserved. All use of this software and documentation is     *
* subject to the License Agreement located at the end of this file below.*
**************************************************************************
* Description:                                                           *
* The following is a simple hello world program running MicroC/OS-II.The * 
* purpose of the design is to be a very simple application that just     *
* demonstrates MicroC/OS-II running on NIOS II.The design doesn't account*
* for issues such as checking system call return codes. etc.             *
*                                                                        *
* Requirements:                                                          *
*   -Supported Example Hardware Platforms                                *
*     Standard                                                           *
*     Full Featured                                                      *
*     Low Cost                                                           *
*   -Supported Development Boards                                        *
*     Nios II Development Board, Stratix II Edition                      *
*     Nios Development Board, Stratix Professional Edition               *
*     Nios Development Board, Stratix Edition                            *
*     Nios Development Board, Cyclone Edition                            *
*   -System Library Settings                                             *
*     RTOS Type - MicroC/OS-II                                           *
*     Periodic System Timer                                              *
*   -Know Issues                                                         *
*     If this design is run on the ISS, terminal output will take several*
*     minutes per iteration.                                             *
**************************************************************************/


#include <stdio.h>
#include "includes.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY      1
#define TASK2_PRIORITY      2

void get_char(INT8U *character, INT8U *available, INT8U *valid)
{
volatile int * RS232_UART_ptr = (int *) 0x08200000; // RS232 UART address
int data;
data = *(RS232_UART_ptr); // read the RS232_UART data register
*available = (INT8U)((data & 0x00FF0000) >> 16);
*valid = (INT8U)((data & 0x00008000) >> 8);
if (data & 0x00008000) { // check RVALID to see if there is new data
	*character = ((INT8U) data & 0xFF);
} else {
	*character = '\0';
}
}

void put_char(char c)
{
volatile int * RS232_UART_ptr = (int *) 0x08200000; // RS232 UART address
int control;

// HET CONTROL REGISTER KLOPT NIET

control = *(RS232_UART_ptr + 4); // read the RS232_UART control register
if (control & 0x00FF0000) { // if space, write character, else ignore
	*(RS232_UART_ptr) = c;
	printf("Printed: %c", c);
}
}

/* Prints "Hello World" and sleeps for three seconds */
void task1(void* pdata)
{
	        INT8U command[40] = "";
	        INT8U characterCount = 0;
  while (1)
  { 
    //printf("Hello from task1\n");
    volatile int * uart_ptr;
        uart_ptr = (int *) 0x08200000;

        INT8U ravail = 0;
        INT8U character = '\0';
        INT8U rvalid = 0;
        get_char(&character, &ravail, &rvalid);
        if((ravail > 0) && (character != '\0') && (rvalid)) {
        	if((character >= 'a') && (character <= 'z')) {
        		printf("Karakter: %c Decimaal van karakter: %d\n", character, character);
    			printf("Aantal karakters beschikbaar: %d\n", ravail);
    			command[characterCount] = character;
    			characterCount++;
    			printf("Command: %s\n", command);
        	}
        	if(strcmp(command, "hallo") == 0) {
        		OSTimeDlyHMSM(0,0,2,0);
        	        	put_char('t');
        	        	put_char('e');
        	        	put_char('s');
        	        	put_char('t');
        	        	put_char('/');


        	        	volatile int * RS232_UART_ptr = (int *) 0x08200000; // RS232 UART address
        	        	        int control;
        	        	        control = *(RS232_UART_ptr + 4);
        	        	        if (control & 0x00FF0000) {
        	        	        	printf("Plek om te verzenden: %d\n", ((control & 0x00FF0000) >> 16));
        	        	        }

        	        	memset(command, 0, strlen(command));
        	        	command[0] = '\0';
        	        	characterCount = 0;
        	}
        }




        //putchar('a');
        //putchar('/');
    //OSTimeDlyHMSM(0, 0, 3, 0);
  }
}
/* Prints "Hello World" and sleeps for three seconds */
void task2(void* pdata)
{
  while (1)
  { 
    //printf("Hello from task2\n");
    OSTimeDlyHMSM(0, 0, 3, 0);
  }
}
/* The main function creates two task and starts multi-tasking */
int main(void)
{
  
  OSTaskCreateExt(task1,
                  NULL,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
              
               
  OSTaskCreateExt(task2,
                  NULL,
                  (void *)&task2_stk[TASK_STACKSIZE-1],
                  TASK2_PRIORITY,
                  TASK2_PRIORITY,
                  task2_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
  OSStart();
  return 0;
}


/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/
