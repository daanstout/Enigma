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
OS_STK task1_stk[TASK_STACKSIZE];
OS_STK task2_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY      1
#define TASK2_PRIORITY      2

#define UART_1 0x08200000
#define UART_0 0x08200008

char command[40] = "";

char getChar(volatile int *UART_ID) {
	int data;
	data = *(UART_ID); // read the RS232_UART data register
	if (data & 0x00008000) { // check RVALID to see if there is new data
		char character = data & 0xFF;
		if (((character >= 'a') && (character <= 'z')) || (character == '/')) {
			return character;
		}
	} else {
		return '\0';
	}
	return '\0';
}

void putChar(volatile int *UART_ID, char c) {
	int control;
	control = *(UART_ID + 4); // read the RS232_UART control register
	if (control & 0x00FF0000) { // if space, write character, else ignore
		*(UART_ID) = c;
	}
}

void sendCommand(volatile int *UART_ID, char string[]) {
	int i;
	for (i = 0; i < strlen(string); i++) {
		putChar(UART_ID, string[i]);
	}
}

void getCommand(volatile int *UART_ID) {
	memset(command, 0, strlen(command));
	command[0] = '\0';
	INT8U finished = 0;
	INT8U characterCount = 0;
	char currentChar = '\0';

	while (!finished) {
		currentChar = getChar(UART_ID);
		if (currentChar != '\0') {
			if ((currentChar >= 'a') || (currentChar = '/')) {
				if (currentChar == '/') {
					if (characterCount > 0) {
						finished = 1;
					}
				} else {
					command[characterCount] = currentChar;
					characterCount++;
				}
			}
		}
		currentChar = '\0';
	}
	command[characterCount] = '\0';
	command[0] = command[strlen(command) - 1];
	command[1] = '\0';
}

/* Prints "Hello World" and sleeps for three seconds */
void task1(void* pdata) {
	INT8U characterCount = 0;
	while (1) {
		OSTimeDlyHMSM(0, 0, 3, 0);
		volatile int uartComponent = UART_0;
//    printf("Hello from task1\n");

		printf("Send\n");
		char string[20];
		sendCommand(uartComponent, "getPluggedLetter a/");
		getCommand(uartComponent);
		printf("Na plugboard: %s\n", command);

		sprintf(string, "getLetter 1%c/", command[0]);
		sendCommand(uartComponent, string);
		getCommand(uartComponent);
		printf("Na rotor een: %s\n", command);

		sprintf(string, "getLetter 2%c/", command[0]);
		sendCommand(uartComponent, string);
		getCommand(uartComponent);
		printf("Na rotor twee: %s\n", command);

		sprintf(string, "getLetter 3%c/", command[0]);
		sendCommand(uartComponent, string);
		getCommand(uartComponent);
		printf("Na rotor drie: %s\n", command);

		sprintf(string, "getLetter 4%c/", command[0]);
		sendCommand(uartComponent, string);
		getCommand(uartComponent);
		printf("Na rotor vier: %s\n", command);

		sprintf(string, "getReflector %c/", command[0]);
		sendCommand(uartComponent, string);
		getCommand(uartComponent);
		printf("Na de reflector: %s\n", command);

		//printf("Commando: %s\n", command);
		if (strcmp(command, "hallo") == 0) {
			sendCommand(uartComponent, "test/");

			memset(command, 0, strlen(command));
			command[0] = '\0';
			characterCount = 0;
		} else if (strlen(command) == 1) {
			printf("Result Commando: %s\n", command);
		}

	}
}
/* Prints "Hello World" and sleeps for three seconds */
void task2(void* pdata) {
	while (1) {
		//printf("Hello from task2\n");
		OSTimeDlyHMSM(0, 0, 3, 0);
	}
}
/* The main function creates two task and starts multi-tasking */
int main(void) {

	OSTaskCreateExt(task1,
	NULL, (void *) &task1_stk[TASK_STACKSIZE - 1],
	TASK1_PRIORITY,
	TASK1_PRIORITY, task1_stk,
	TASK_STACKSIZE,
	NULL, 0);

	OSTaskCreateExt(task2,
	NULL, (void *) &task2_stk[TASK_STACKSIZE - 1],
	TASK2_PRIORITY,
	TASK2_PRIORITY, task2_stk,
	TASK_STACKSIZE,
	NULL, 0);
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
