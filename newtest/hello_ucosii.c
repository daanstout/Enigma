#include <stdio.h>
#include "includes.h"
#include <altera_up_ps2_keyboard.h>
#include <altera_up_avalon_ps2.h>
//#include <altera_avalon_pio_regs.h>
//#include "altera_up_avalon_parallel_port.h"
#include "string.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
#define BUF_SIZE 500000			// about 10 seconds of buffer (@ 48K samples/sec)#define BUF_THRESHOLD 96		// 75% of 128 word bufferOS_STK * crypt_stk[TASK_STACKSIZE];OS_STK * task1_stk[TASK_STACKSIZE];OS_STK * task2_stk[TASK_STACKSIZE];OS_STK * task3_stk[TASK_STACKSIZE];OS_STK * rotateTask_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define CRYPT_PRIORITY      6
#define TASK1_PRIORITY      10
#define TASK2_PRIORITY		11
#define TASK3_PRIORITY		8
#define ROTATETASK_PRIORITY	7

#define UART_0 0x08200000

#define BREAKCODE	0xF0
// key defines
#define ENTER	'9'		// enter#define UP		'1'		// keypad 8#define ESC		'8'		// escape#define RIGHT	'2'		// keypad 6#define BKSP	'7'		// backspace#define LEFT	'3'		// keypad 4#define SPACE	'6'		// space#define DOWN	'4'		// keypad 2#define TAB 	'5'		// Tab//#define RX0 0x00000000 // all zero//#define TX0 0x00000000 // all zero//#define RX1 0x01000000 // D24//#define TX1 0x02000000 // D25//#define RX2 0x04000000 // D26
//#define TX2 0x08000000 // D27
//#define RX3 0x10000000 // D28
//#define TX3 0x20000000 // D29
//#define RX4 0x40000000 // D30
//#define TX4 0x80000000 // D31

OS_EVENT * messageBox;
OS_EVENT * ControlBlock;
OS_EVENT * sem_hardware;
OS_EVENT * sem_enigma;
OS_EVENT * sem_rotate;
OS_EVENT * sem_rotated;

char * messages[40];
char command[40] = "";

void PS2();
char getKey();
void select(char c);
void typen(char c);
void printRinginstelling(char c[2], int roror);
void printRotorStand(char c[2], int rotor);
void VGA_text(int, int, char *);
void VGA_box(int, int, int, int, short);
void clearScreen();
void setX();
void changeRotor(char knop);

volatile int * PS2_ptr = (int *) 0x10000100;  // PS/2 port address
volatile char * character_buffer = (char *) 0x09000000;	// VGA character buffer
volatile short * pixel_buffer = (short *) 0x08000000;

int byte1;
char letter;
char * ascii[10];
char strTemp[2];
char arrowTemp[2];
char d[2];
char e[2];
int PS2_data;
int RAVAIL = 0;
int rotorNummer = 1;
int ringinstelling;
char rotorStand;
int count = 5;
int rij = 1;
int letterLijst[5][7];
short cijferLijst[5][7];
int ringInstellingen[4] = { 1, 1, 1, 1 };
char rotorStanden[4] = { 'a', 'a', 'a', 'a' };
int y = 5;
int x1, x2, x3, x4, x5, x6;

char getChar(volatile int *UART_ID) {
	int data;
	data = *(UART_ID); // read the RS232_UART data register
	if (data & 0x00008000) { // check RVALID to see if there is new data
		char character = data & 0xFF;
		if (((character >= 'a') && (character <= 'z')) || (character == '/')
				|| (character == '0') || (character == '1')) {
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

void sendCommand(char string[]) {
	int i;
	for (i = 0; i < strlen(string); i++) {
		putChar(UART_0, string[i]);
	}
}

void getCommand() {
	memset(command, 0, strlen(command));
	command[0] = '\0';
	INT8U finished = 0;
	INT8U characterCount = 0;
	char currentChar = '\0';
	while (!finished) {
		currentChar = getChar(UART_0);
		if (currentChar != '\0') {
			if ((currentChar >= 'a') || (currentChar == '/')
					|| (currentChar == '1') || (currentChar == '0')) {
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

void getLetter(char string2[], char character) {
	char string[20];
	sprintf(string, "%s%c/", string2, character);
	sendCommand(string);
	getCommand();
}

/* Prints "Hello World" and sleeps for three seconds */
void cryptTask(void* pdata) {
	INT8U err;
	while (1) {
		char input[20];
		OSTimeDlyHMSM(0, 0, 0, 50);
		sprintf(input, "%s", OSMboxPend(messageBox, 0, &err));

		if (input[0] != 0) {
			if ((input[0] >= 'a') && (input[0] <= 'z')) {
				// continues after the rotors have rotated
				OSSemPost(sem_rotate);
				OSTimeDlyHMSM(0, 0, 0, 100);
				OSSemPend(sem_rotated, 0, &err);
				char string[20];

				getLetter("getPluggedLetter", input[0]);
				//printf("Na plugboard: %s\n", command);

				getLetter("getLetter 1", command[0]);
				//printf("Na rotor een: %s\n", command);

				getLetter("getLetter 2", command[0]);
				//printf("Na rotor twee: %s\n", command);

				getLetter("getLetter 3", command[0]);
				//printf("Na rotor drie: %s\n", command);

				getLetter("getLetter 4", command[0]);
				//printf("Na rotor vier: %s\n", command);

				getLetter("getReflector ", command[0]);
				//printf("Na reflector: %s\n", command);

				getLetter("getReverseLetter 4", command[0]);
				//printf("Na reverse vier: %s\n", command);

				getLetter("getReverseLetter 3", command[0]);
				//printf("Na reverse drie: %s\n", command);

				getLetter("getReverseLetter 2", command[0]);
				//printf("Na reverse twee: %s\n", command);

				getLetter("getReverseLetter 1", command[0]);
				//printf("Na reverse een: %s\n", command);

				getLetter("getPluggedLetter ", command[0]);
				//printf("Na laatste plugboard: %s\n", command);

//				char tempor[30];
//				sprintf(tempor, "getRotorLetter 1/");
//				sendCommand(tempor);
//				getCommand();
//				//getLetter("getRotorLetter 1", command[0]);
//				printf("rotorLetter: %c\n", command[0]);
			}
		}
	}
}

void rotateTask(void * pdata) {
	INT8U err;
	INT8U rotateOne = 0;
	INT8U rotateTwo = 0;

	while (1) {
		OSSemPend(sem_rotate, 0, &err);
		sendCommand("getTriggered 1/");
		getCommand();
		//printf("COMMAND: %s\n", command);
		if (command[0] == '1') {
			rotateOne = 1;
			printf("TRIGGERED ROTOR 2");
		}

		sendCommand("getTriggered 2/");
		getCommand();

		if (command[0] == '1') {
			rotateTwo = 1;
		}

		if (rotateOne) {
			sendCommand("rotateForward 2/");
			rotateOne = 0;

			rotorStanden[2]++;

			if (rotorStanden[2] > 'z') {
				rotorStanden[2] = 'a';
			}

		} else if (rotateTwo) {
			sendCommand("rotateForward 3/");
			rotateTwo = 0;
			rotorStanden[1]++;

			if (rotorStanden[1] > 'z') {
				rotorStanden[1] = 'a';
			}
		}

		sendCommand("rotateForward 1/");
		rotorStanden[3]++;

		if (rotorStanden[3] > 'z') {
			rotorStanden[3] = 'a';
		}

		// Updates the rotor positions on the screen
		char string[2];
		int i;
		for (i = 3; i > 1; i--) {
			sprintf(string, "%c", rotorStanden[i]);
			printRotorStand(string, i + 1);
		}

		OSSemPost(sem_rotated);
	}
}

void task1(void* pdata) {

	while (1) {
		PS2();
		strTemp[0] = getKey();
		strTemp[1] = '\0';
		OSTimeDlyHMSM(0, 0, 0, 100);
	}
}

void task2(void * pdata) {
	INT8U err;

	while (1) {
		//OSSemPend(sem_enigma, 0, &err);

		if (strTemp[0] >= '1' || strTemp[0] <= '5') {
			select(strTemp[0]);
			printRinginstelling(d, rotorNummer);
			printRotorStand(e, rotorNummer);
			d[0] = '\0';
			e[0] = '\0';

			OSTimeDlyHMSM(0, 0, 0, 100);
			OSSemPost(sem_enigma);
		}
		OSTimeDlyHMSM(0, 0, 0, 50);

	}
}

void task3(void *pdata) {
	INT8U err;

	while (1) {
		OSSemPend(sem_enigma, 0, &err);
		if (strTemp[0] < '1' || strTemp[0] > '5') {
			char string[30];
			sprintf(string, "%s", strTemp);
			err = OSQPost(ControlBlock, string);
			err = OSMboxPost(messageBox, OSQPend(ControlBlock, 0, &err));
			typen(strTemp[0]);
		}

		//OSSemPost(sem_enigma);
		OSTimeDlyHMSM(0, 0, 0, 100);
	}
}

/* The main function creates two task and starts multi-tasking */
int main(void) {

	OSInit();								// Initialize uCOS-II.

	sem_hardware = OSSemCreate(1);
	sem_rotate = OSSemCreate(0);
	sem_rotated = OSSemCreate(0);
	sem_enigma = OSSemCreate(1);

	ControlBlock = OSQCreate(&messages, 40);
	messageBox = OSMboxCreate(NULL);

	VGA_box(0, 0, 160, 240, 0);
	VGA_box(161, 0, 320, 240, 0);

	VGA_box(20, 200, 60, 220, 0x8410);
	VGA_box(100, 180, 140, 220, 0x8410);
	VGA_box(180, 180, 220, 220, 0x8410);
	VGA_box(260, 180, 300, 220, 0x8410);
	VGA_box(20, 180, 60, 200, 0xF800);

	printRinginstelling("1", 1);
	printRinginstelling("1", 2);
	printRinginstelling("1", 3);
	printRinginstelling("1", 4);

	printRotorStand("a", 1);
	printRotorStand("a", 2);
	printRotorStand("a", 3);
	printRotorStand("a", 4);

	OSTaskCreate(task1, (void*) 0, &task1_stk[TASK_STACKSIZE - 1],
	TASK1_PRIORITY);
	OSTaskCreate(task2, (void*) 0, &task2_stk[TASK_STACKSIZE - 1],
	TASK2_PRIORITY);
	OSTaskCreate(task3, (void*) 0, &task3_stk[TASK_STACKSIZE - 1],
	TASK3_PRIORITY);
	OSTaskCreate(cryptTask, (void*) 0, &crypt_stk[TASK_STACKSIZE - 1],
	CRYPT_PRIORITY);
	OSTaskCreate(rotateTask, (void*) 0, &rotateTask_stk[TASK_STACKSIZE - 1],
	ROTATETASK_PRIORITY);

	OSStart();
	return 0;
}

void VGA_box(int x1, int y1, int x2, int y2, short pixel_color) { //320x240
	int offset, row, col;
	// VGA pixel buffer

	/* assume that the box coordinates are valid */
	for (row = y1; row <= y2; row++) {
		col = x1;
		while (col <= x2) {
			offset = (row << 9) + col;
			*(pixel_buffer + offset) = pixel_color;	// compute halfword address, set pixel
			++col;
		}
	}
}

void VGA_text(int x, int y, char * text_ptr) {
	int offset;

	/* assume that the text string fits on one line */
	offset = (y << 7) + x;
	while (*(text_ptr)) {

		*(character_buffer + offset) = *(text_ptr);	// write to the character buffer
		++text_ptr;
		++offset;
	}
}

void typen(char c) {
	INT8U err;
	strTemp[0] = c;
	strTemp[1] = '\0';

	if (count <= 5) {
		count = 5;
	} else if (count >= 35) {
		count = 5;
		y++;
	} else if (y <= 5) {
		y = 5;
	} else if (y >= 40) {
		y = 40;
	}

	if (strTemp[0] == ESC) {
		clearScreen();
		count = 5;
		y = 5;
	} else if (strTemp[0] == BKSP) {
		if (count == 5 && y > 5) {
			y--;
			count = 35;
		}
		count--;
		VGA_text(count, y, " ");
		VGA_text(count + 40, y, " ");
	} else if (strTemp[0] == SPACE) {
		VGA_text(count, y, " ");
		VGA_text(count + 40, y, " ");
		count++;
	} else if (strTemp[0] == ENTER) {
		count = 5;
		y++;
	} else {
		if (strTemp[0] != 0) {
			VGA_text(count, y, strTemp);
			//printf("string: %s %c\n", command, command[0]);
			OSTimeDlyHMSM(0, 0, 0, 65);
			VGA_text(count + 40, y, command);
			strTemp[0] = 0;
			count++;
		}
	}
}

void changeRotor(char knop) {
	char tempo[30];
	if (knop == UP) {
		if (rij == 1) {
			rotorStanden[rotorNummer - 1]++;
			if (rotorStanden[rotorNummer - 1] > 'z') {
				rotorStanden[rotorNummer - 1] = 'a';
			}
			sprintf(tempo, "rotateForward %d/", 5 - rotorNummer);
			printf("%s\n", tempo);
			sendCommand(tempo);
			sprintf(e, "%c", rotorStanden[rotorNummer - 1]);
		} else if (rij == 2) {
			ringInstellingen[rotorNummer - 1]++;
			if (ringInstellingen[rotorNummer - 1] > 26) {
				ringInstellingen[rotorNummer - 1] = 1;
			}
			sprintf(tempo, "changeRingCfg %d%c/", 5 - rotorNummer,
					ringInstellingen[rotorNummer - 1] + 'a' - 1);
			sendCommand(tempo);
			printf("%s\n", tempo);
			sprintf(d, "%d", ringInstellingen[rotorNummer - 1]);
		}
	} else if (knop == DOWN) {
		if (rij == 1) {
			rotorStanden[rotorNummer - 1]--;
			if (rotorStanden[rotorNummer - 1] < 'a') {
				rotorStanden[rotorNummer - 1] = 'z';
			}
			sprintf(tempo, "rotateBackwards %d/", 5 - rotorNummer);
			sendCommand(tempo);
			sprintf(e, "%c", rotorStanden[rotorNummer - 1]);
		} else if (rij == 2) {
			ringInstellingen[rotorNummer - 1]--;
			if (ringInstellingen[rotorNummer - 1] < 1) {
				ringInstellingen[rotorNummer - 1] = 26;
			}
			sprintf(tempo, "changeRingCfg %d%c/", 5 - rotorNummer,
					ringInstellingen[rotorNummer - 1] + 'a' - 1);
			sendCommand(tempo);
			printf("%s\n", tempo);
			sprintf(d, "%d", ringInstellingen[rotorNummer - 1]);
		}
	}
//	char tempor[30];
//	sprintf(tempor, "getRotorLetter 1/");
//	sendCommand(tempor);
//	getCommand();
//	//getLetter("getRotorLetter 1", command[0]);
//	printf("rotorLetter: %c\n", command[0]);
	OSTimeDlyHMSM(0, 0, 0, 75);
}

void select(char c) {
	arrowTemp[0] = c;
	arrowTemp[1] = '\0';

	if (rotorNummer > 4) {
		rotorNummer = 4;
	}

	if (rotorNummer < 1) {
		rotorNummer = 1;
	}

	if (arrowTemp[0] == TAB) {
		setX();
		if (rij == 1) {
			rij = 2;
			VGA_box(x1, 180, x2, 200, 0x8410);
			VGA_box(x1, 200, x2, 220, 0xF800);
		} else {
			rij = 1;
			VGA_box(x1, 180, x2, 200, 0xF800);
			VGA_box(x1, 200, x2, 220, 0x8410);
		}
	}
	if (arrowTemp[0] == UP) {
		changeRotor(UP);
	} else if (arrowTemp[0] == DOWN) {
		changeRotor(DOWN);
	} else if (arrowTemp[0] == LEFT) {
		if (rotorNummer < 1) {
			rotorNummer = 1;
		} else if (rotorNummer >= 1) {
			rotorNummer--;
			if (rotorNummer >= 1) {
				setX();
				if (rij == 1) {
					VGA_box(x1, 180, x2, 200, 0xF800);
					VGA_box(x3, 180, x4, 200, 0x8410);
				} else {
					VGA_box(x1, 200, x2, 220, 0xF800);
					VGA_box(x3, 200, x4, 220, 0x8410);
				}
			}
		}
	} else if (arrowTemp[0] == RIGHT) {
		if (rotorNummer > 4) {
			rotorNummer = 4;
		} else if (rotorNummer <= 4) {
			rotorNummer++;
			if (rotorNummer < 5) {
				setX();
				if (rij == 1) {
					VGA_box(x5, 180, x6, 200, 0x8410);
					VGA_box(x1, 180, x2, 200, 0xF800);
				} else {
					VGA_box(x5, 200, x6, 220, 0x8410);
					VGA_box(x1, 200, x2, 220, 0xF800);
				}
			}
		}
	}
}

void setX() {
	x1 = (80 * rotorNummer) - 60;
	x2 = (40 * rotorNummer) + 20 + (40 * (rotorNummer - 1));

	x3 = (80 * (rotorNummer + 1)) - 60;
	x4 = (40 * (rotorNummer + 1)) + 20 + (40 * rotorNummer);

	x5 = (80 * (rotorNummer - 1)) - 60;
	x6 = (40 * (rotorNummer - 1)) + 20 + (40 * (rotorNummer - 2));
}

void clearScreen() {
	int i;
	for (i = 0; i <= 44; i++) {
		VGA_text(0, i,
				"                                                                                                        ");
	}
}

void PS2() {
	PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
	RAVAIL = (PS2_data & 0xFFFF0000) >> 16;	// extract the RAVAIL field
	if (RAVAIL > 0) {

		byte1 = PS2_data & 0xFF;
		*PS2_ptr &= 0x00000000;

		OSTimeDlyHMSM(0, 0, 0, 10);

	}
}

char getKey() {
	if (byte1 != BREAKCODE && byte1 != 0xE0) {//checks if the received scancode is a BREAKCODE indicator, 0xF0
		translate_make_code(KB_ASCII_MAKE_CODE, (char *) byte1, &ascii);//if not, translates it to ascii
		byte1 = 0;
	}

	char tempo = ascii[0];

	if (strcmp(ascii, "KP 8") == 0) {
		return UP;
	} else if (strcmp(ascii, "KP 2") == 0) {
		return DOWN;
	} else if (strcmp(ascii, "KP 6") == 0) {
		return RIGHT;
	} else if (strcmp(ascii, "KP 4") == 0) {
		return LEFT;
	} else if (strcmp(ascii, "ESC") == 0) {
		return ESC;
	} else if (strcmp(ascii, "BKSP") == 0) {
		return BKSP;
	} else if (strcmp(ascii, "SPACE") == 0) {
		return SPACE;
	} else if (strcmp(ascii, "ENTER") == 0) {
		return ENTER;
	} else if (strcmp(ascii, "TAB") == 0) {
		return TAB;
	} else if ((tempo >= 'A' && tempo <= 'Z')) {//checks wether or not the received byte is a letter
		if (strlen(ascii) == 1) {
			tempo = tempo + 32;	//makes it a small letter instead of a capital
			return tempo;							//returns the character
		}
	}
	return 0;									// returns null
}

void printRinginstelling(char c[2], int rotor) { //d
	int x1;
	x1 = (20 * (rotor - 1)) + 10;
	VGA_text(x1, 51, c);
	if (ringInstellingen[0] < 10) {
		VGA_text(11, 51, " ");
	}
	if (ringInstellingen[1] < 10) {
		VGA_text(31, 51, " ");
	}
	if (ringInstellingen[2] < 10) {
		VGA_text(51, 51, " ");
	}
	if (ringInstellingen[3] < 10) {
		VGA_text(71, 51, " ");
	}
}

void printRotorStand(char c[2], int rotor) { //e
	int x1;
	x1 = (20 * (rotor - 1)) + 10;
	VGA_text(x1, 45, c);
}
