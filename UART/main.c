#include <stdio.h>
#include "includes.h"
#include <altera_up_ps2_keyboard.h>
#include <altera_up_avalon_ps2.h>
#include "string.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
#define BUF_SIZE 500000			// about 10 seconds of buffer (@ 48K samples/sec)
#define BUF_THRESHOLD 96		// 75% of 128 word buffer
OS_STK * crypt_stk[TASK_STACKSIZE];
OS_STK * task1_stk[TASK_STACKSIZE];
OS_STK * task2_stk[TASK_STACKSIZE];
OS_STK * task3_stk[TASK_STACKSIZE];
OS_STK * rotateTask_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define CRYPT_PRIORITY      7
#define TASK1_PRIORITY      10
#define TASK2_PRIORITY		11
#define TASK3_PRIORITY		8
#define ROTATETASK_PRIORITY	6

// UART component main adress
#define UART_0 0x08200000

// key defines
#define ENTER	'9'		// enter
#define UP		'1'		// keypad 8
#define ESC		'8'		// escape
#define RIGHT	'2'		// keypad 6
#define BKSP	'7'		// backspace
#define LEFT	'3'		// keypad 4
#define SPACE	'6'		// space
#define DOWN	'4'		// keypad 2
#define TAB 	'5'		// Tab
#define FRAME 0x2AFA
#define ACHTERGROND 0x0000
#define ROTOR 0xADDF
#define ROTOR_SELECT 0x501F
#define BREAKCODE	0xF0

OS_EVENT * messageBox;
OS_EVENT * ControlBlock;
OS_EVENT * sem_hardware;
OS_EVENT * sem_enigma;
OS_EVENT * sem_rotate;
OS_EVENT * sem_rotated;

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
void VGA_text_clear_all(char * character_buffer);
void VGA_text_clear(char * character_buffer, int x1, int y1, int x2, int y2);

volatile int * PS2_ptr = (int *) 0x10000100;  // PS/2 port address
volatile char * character_buffer = (char *) 0x09000000;	// VGA character buffer
volatile short * pixel_buffer = (short *) 0x08000000;

char * messages[40];
char command[40] = "";
int byte1;
char letter;
char * ascii[10];
char strTemp[2];
char arrowTemp[2];
char d[2];
char e[2];
int PS2_data;
int RAVAIL = 0;
int rotorNumber = 1;
int ringinstelling;
char rotorStand;
int count = 5;
int rij = 1;
int letterLijst[5][7];
short cijferLijst[5][7];
int ringInstellingen[4] = { 1, 1, 1, 1 };
char rotorPositions[4] = { 'a', 'a', 'a', 'a' };
int y = 5;
int x1, x2, x3, x4, x5, x6;

// Gets a character from a arduino
char getChar(volatile int *UART_ID) {
	int data;
	// Reads the RS232_UART data register
	data = *(UART_ID);
	// Checks RVALID to see if there is new data
	if (data & 0x00008000) {
		char character = data & 0xFF;
		// Checks if the character is a letter, a '/', a '1' or a '0'
		if (((character >= 'a') && (character <= 'z')) || (character == '/')
				|| (character == '0') || (character == '1')) {
			return character;
		}
	} else {
		return '\0';
	}
	return '\0';
}

// Sends a character to a arduino
void putChar(volatile int *UART_ID, char c) {
	int control;
	// Reads the RS232_UART control register
	control = *(UART_ID + 4);
	// If there is space in the write register, writes the character
	if (control & 0x00FF0000) {
		*(UART_ID) = c;
	}
}

// Sends a command to a arduino
void sendCommand(char string[]) {
	int i;
	for (i = 0; i < strlen(string); i++) {
		putChar(UART_0, string[i]);
	}
}

// Gets a command from a arduino
void getCommand() {
	memset(command, 0, strlen(command));
	command[0] = '\0';
	INT8U finished = 0;
	INT8U characterCount = 0;
	char currentChar = '\0';

	// Checks if the end character has been send
	while (!finished) {
		currentChar = getChar(UART_0);
		// Checks if the character isn't '\0'
		if (currentChar != '\0') {
			// Checks if the character is higher than a 'a' or if the character is '0', '1' or a '/'
			if ((currentChar >= 'a') || (currentChar == '/')
					|| (currentChar == '1') || (currentChar == '0')) {
				// If the character is a '/' the message has ended
				if (currentChar == '/') {
					// Checks if the message that is received has more than 0 characters
					if (characterCount > 0) {
						finished = 1;
					}
				} else {
					// Adds the character to the message string
					command[characterCount] = currentChar;
					characterCount++;
				}
			}
		}
		currentChar = '\0';
	}
	// Clears the command string
	command[characterCount] = '\0';
	command[0] = command[strlen(command) - 1];
	command[1] = '\0';
}

// Gets a letter from a arduino
void getLetter(char string2[], char character) {
	char string[20];
	sprintf(string, "%s%c/", string2, character);
	sendCommand(string);
	getCommand();
}

// Puts a character through the Enigma Machine
void cryptTask(void* pdata) {
	INT8U err;
	while (1) {
		char input[20];
		OSTimeDlyHMSM(0, 0, 0, 50);
		// Checks if there is a character available
		sprintf(input, "%s", OSMboxPend(messageBox, 0, &err));

		// Checks if the input is not empty
		if (input[0] != 0) {
			// Checks if the character is a letter
			if ((input[0] >= 'a') && (input[0] <= 'z')) {

				// Makes the rotate task run
				OSSemPost(sem_rotate);
				OSTimeDlyHMSM(0, 0, 0, 100);
				// Continues after the rotors have rotated
				OSSemPend(sem_rotated, 0, &err);
				char string[20];

				// Letter goes through the plugboard and the rotors for the first time
				getLetter("getPluggedLetter", input[0]);
				getLetter("getLetter 1", command[0]);
				getLetter("getLetter 2", command[0]);
				getLetter("getLetter 3", command[0]);
				getLetter("getLetter 4", command[0]);

				// Letter gets reflected through the rotors and the plugboard again
				getLetter("getReflector ", command[0]);
				getLetter("getReverseLetter 4", command[0]);
				getLetter("getReverseLetter 3", command[0]);
				getLetter("getReverseLetter 2", command[0]);
				getLetter("getReverseLetter 1", command[0]);
				getLetter("getPluggedLetter ", command[0]);

			}
		}
	}
}

// Rotates a rotor if necessary
void rotateTask(void * pdata) {
	INT8U err;
	INT8U rotateOne = 0;
	INT8U rotateTwo = 0;

	while (1) {
		// Waits until a character has been entered in the crypt task
		OSSemPend(sem_rotate, 0, &err);
		sendCommand("getTriggered 1/");
		getCommand();

		// Checks if the outer-right rotor is triggered
		if (command[0] == '1') {
			rotateOne = 1;
		}

		sendCommand("getTriggered 2/");
		getCommand();

		// Checks if the rotor left of the outer-right rotor is triggered
		if (command[0] == '1') {
			rotateTwo = 1;
		}

		// rotates the rotor next to the rotor that was triggered
		if (rotateOne) {
			// Rotates the left rotor on the right side of the greek rotor
			sendCommand("rotateForward 2/");

			rotateOne = 0;
			rotorPositions[2]++;

			// Rotates the visible rotor-position
			if (rotorPositions[2] > 'z') {
				rotorPositions[2] = 'a';
			}

		} else if (rotateTwo) {
			// Rotates the rotor left from the outer-right rotor
			sendCommand("rotateForward 3/");

			rotateTwo = 0;
			rotorPositions[1]++;

			// Rotates the visible rotor-position
			if (rotorPositions[1] > 'z') {
				rotorPositions[1] = 'a';
			}
		}

		// Rotates the outer-right rotor
		sendCommand("rotateForward 1/");
		rotorPositions[3]++;
		// Rotates the visible rotor-position
		if (rotorPositions[3] > 'z') {
			rotorPositions[3] = 'a';
		}

		// Updates the rotor positions on the screen
		char string[2];
		int i;
		for (i = 3; i > 1; i--) {
			sprintf(string, "%c", rotorPositions[i]);
			printRotorStand(string, i + 1);
		}
		// Makes the crypt task continue
		OSSemPost(sem_rotated);
	}
}

// Gets the character from the PS2 keyboard
void task1(void* pdata) {
	while (1) {
		PS2();
		strTemp[0] = getKey();
		strTemp[1] = '\0';
		OSTimeDlyHMSM(0, 0, 0, 100);
	}
}

// Rotates the rotors when a rotor-position is changed from the GUI
void task2(void * pdata) {
	INT8U err;
	while (1) {
		// Checks if the arrow up or down on the numpad is pressed
		if (strTemp[0] >= '1' || strTemp[0] <= '5') {
			select(strTemp[0]);
			printRinginstelling(d, rotorNumber);
			printRotorStand(e, rotorNumber);
			d[0] = '\0';
			e[0] = '\0';

			OSTimeDlyHMSM(0, 0, 0, 100);
		}
		OSSemPost(sem_enigma);
		OSTimeDlyHMSM(0, 0, 0, 50);
	}
}

// Sends a character to the Crypt task if a letter is typed in
void task3(void *pdata) {
	INT8U err;

	while (1) {
		OSSemPend(sem_enigma, 0, &err);
		// Checks if the character is not a arrow key
		if (strTemp[0] < '1' || strTemp[0] > '5') {
			// Sends a letter to the Crypt task
			char string[30];
			sprintf(string, "%s", strTemp);
			err = OSQPost(ControlBlock, string);
			err = OSMboxPost(messageBox, OSQPend(ControlBlock, 0, &err));
			typen(strTemp[0]);
		}

		OSTimeDlyHMSM(0, 0, 0, 100);
	}
}

// The main function initializes the system and starts the tasks
int main(void) {

	OSInit();								// Initialize uCOS-II.

	sem_hardware = OSSemCreate(1);
	sem_rotate = OSSemCreate(0);
	sem_rotated = OSSemCreate(0);
	sem_enigma = OSSemCreate(1);

	ControlBlock = OSQCreate(&messages, 40);
	messageBox = OSMboxCreate(NULL);
	VGA_text_clear_all(character_buffer);

	VGA_box(0, 0, 160, 240, ACHTERGROND);
	VGA_box(161, 0, 320, 240, ACHTERGROND);

	VGA_box(0, 160, 320, 240,FRAME);
	VGA_box(155 ,0 ,165 , 240, FRAME);
	VGA_box(0 ,0 ,10 , 240, FRAME);
	VGA_box(0 ,0 ,320 , 10, FRAME);
	VGA_box(310 ,0 ,320 , 240, FRAME);

	VGA_box(20, 200, 60, 220, ROTOR);
	VGA_box(100, 180, 140, 220, ROTOR);
	VGA_box(180, 180, 220, 220, ROTOR);
	VGA_box(260, 180, 300, 220, ROTOR);
	VGA_box(20, 180, 60, 200, ROTOR_SELECT);

	VGA_text(3, 1, "Input tekst:");
	VGA_text(42, 1, "Output tekst:");

	// Prints the default rotor-positions and ring-settings on the screen
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

// Draws a box on the screen
void VGA_box(int x1, int y1, int x2, int y2, short pixel_color) { // 320x240
	int offset, row, col;
	
	// Loops per row through the buffer
	for (row = y1; row <= y2; row++) { 
		col = x1;
		// Loops per row per column thought the buffer
		while (col <= x2) { 
			// Calculates the offset
			offset = (row << 9) + col;  
			// Fills the buffer with the chosen colour
			*(pixel_buffer + offset) = pixel_color;	
			++col;
		}
	}
}

// Draws text on the screen
void VGA_text(int x, int y, char * text_ptr) { // 80x60
	int offset;

	// Calculates the offset
	offset = (y << 7) + x; 
	while (*(text_ptr)) { 

		// Writes a character to the buffer
		*(character_buffer + offset) = *(text_ptr);	
		++text_ptr; 
		// Increases the offset so the next character doesn't overwrite the previous one
		++offset; 
	}
}

// Makes you able to type on the screen
void typen(char c) {
	INT8U err;
	strTemp[0] = c;
	strTemp[1] = '\0';

	// Every line always starts on x = 5
	if (count <= 5) { 
		count = 5;
	}
	// Every line end on x = 35
	if (count >= 35) { 
		count = 5;
		y++;
	}
	// Lines start at y = 5
	if (y <= 5) { 
		y = 5;
	}
	// Lines end at y = 35 so the characters don't overwrite the rotors
	if (y > 39) { 
		y = 39;
	}

	// Checks for escape so the screen can be cleared
	if (strTemp[0] == ESC) { 
		clearScreen();
		count = 5;
		y = 5;
	} else if (strTemp[0] == BKSP) {
		// Backspace decreases the x-value, when at the beginning of a line also decreases the y-value
		if (count == 5 && y > 5) { 
			y--;
			count = 35;
		}
		count--;

		VGA_text(count, y, " ");
		// Instead of removing a character, places a space over it
		VGA_text(count + 40, y, " ");
		// Places a space at the next x-value
	} else if (strTemp[0] == SPACE) { 
		VGA_text(count, y, " ");
		VGA_text(count + 40, y, " "); 
		count++;
		// Increases the y-value
	} else if (strTemp[0] == ENTER) {  
		count = 5;
		y++;
	} else {
		if (strTemp[0] != 0) {
			// Prints the character the user typed in
			VGA_text(count, y, strTemp); 
			OSTimeDlyHMSM(0, 0, 0, 75);
			// Prints the encrypted character
			VGA_text(count + 40, y, command); 
			strTemp[0] = 0;
			count++;
		}
	}
	OSTimeDlyHMSM(0, 0, 0, 10);

}

// Changes the rotor position
void changeRotor(char knop) {
	char tempo[30];
	// Checks if the rotor-position or the ring-setting has to increase or decrease
	if (knop == UP) {
		// Checks which if the rotor-position is selected or the rotor-setting
		if (rij == 1) {
			rotorPositions[rotorNumber - 1]++;

			// Rotates the visible rotorPosition
			if (rotorPositions[rotorNumber - 1] > 'z') {
				rotorPositions[rotorNumber - 1] = 'a';
			}

			// Rotates the rotor forward
			sprintf(tempo, "rotateForward %d/", 5 - rotorNumber);
			printf("%s\n", tempo);
			sendCommand(tempo);
			sprintf(e, "%c", rotorPositions[rotorNumber - 1]);
		} else if (rij == 2) {
			ringInstellingen[rotorNumber - 1]++;

			// Rotates the ring-setting
			if (ringInstellingen[rotorNumber - 1] > 26) {
				ringInstellingen[rotorNumber - 1] = 1;
			}

			// Changes the ring-setting
			sprintf(tempo, "changeRingCfg %d%c/", 5 - rotorNumber,
			ringInstellingen[rotorNumber - 1] + 'a' - 1);
			sendCommand(tempo);
			sprintf(d, "%d", ringInstellingen[rotorNumber - 1]);
		}
	} else if (knop == DOWN) {
		// Checks which if the rotor-position is selected or the rotor-setting
		if (rij == 1) {
			rotorPositions[rotorNumber - 1]--;

			// Rotates the visible rotorPosition
			if (rotorPositions[rotorNumber - 1] < 'a') {
				rotorPositions[rotorNumber - 1] = 'z';
			}

			// Rotates the rotor backwards
			sprintf(tempo, "rotateBackwards %d/", 5 - rotorNumber);
			sendCommand(tempo);
			sprintf(e, "%c", rotorPositions[rotorNumber - 1]);
		} else if (rij == 2) {
			ringInstellingen[rotorNumber - 1]--;

			// Rotates the ring-setting
			if (ringInstellingen[rotorNumber - 1] < 1) {
				ringInstellingen[rotorNumber - 1] = 26;
			}

			// Changes the ring-setting
			sprintf(tempo, "changeRingCfg %d%c/", 5 - rotorNumber,
			ringInstellingen[rotorNumber - 1] + 'a' - 1);
			sendCommand(tempo);
			sprintf(d, "%d", ringInstellingen[rotorNumber - 1]);
		}
	}
	OSTimeDlyHMSM(0, 0, 0, 75);
}

// Function for the selection and changing the configuration for the rotors
void select(char c) { 
	arrowTemp[0] = c;
	arrowTemp[1] = '\0';

	// Failsafe for the selector not getting higher than 4
	if (rotorNumber > 4) { 
		rotorNumber = 4;
	}

	// Failsafe for the selector not getting lower than 1
	if (rotorNumber < 1) {
		rotorNumber = 1;
	}

	// Pressing TAB makes the row selection switch
	if (arrowTemp[0] == TAB) { 
		setX();
		if (rij == 1) {
			rij = 2;
			VGA_box(x1, 180, x2, 200, ROTOR);
			VGA_box(x1, 200, x2, 220, ROTOR_SELECT);
		} else {
			rij = 1;
			VGA_box(x1, 180, x2, 200, ROTOR_SELECT);
			VGA_box(x1, 200, x2, 220, ROTOR);
		}
	}
	// Increase the rotorPosition of the current rotor
	if (arrowTemp[0] == UP) { 
		changeRotor(UP);
	// Decrease the configuration of the current rotor
	} else if (arrowTemp[0] == DOWN) {
		changeRotor(DOWN);
	// Moves the selector to the left
	} else if (arrowTemp[0] == LEFT) { 
		// Extra failsafe for the selector not getting lower than 1
		if (rotorNumber < 1) { 
			rotorNumber = 1;
		} else if (rotorNumber >= 1) {
			rotorNumber--;
			if (rotorNumber >= 1) {
				setX();
				if (rij == 1) {
					VGA_box(x1, 180, x2, 200, ROTOR_SELECT);
					VGA_box(x3, 180, x4, 200, ROTOR);
				} else {
					VGA_box(x1, 200, x2, 220, ROTOR_SELECT);
					VGA_box(x3, 200, x4, 220, ROTOR);
				}
			}
		}
	// Moves the selector one to the right
	} else if (arrowTemp[0] == RIGHT) { 
		// Extra failsafe for the selector not getting higher than 4
		if (rotorNumber > 4) { 
			rotorNumber = 4;
		} else if (rotorNumber <= 4) {
			rotorNumber++;
			if (rotorNumber < 5) {
				setX();
				if (rij == 1) {
					VGA_box(x5, 180, x6, 200, ROTOR);
					VGA_box(x1, 180, x2, 200, ROTOR_SELECT);
				} else {
					VGA_box(x5, 200, x6, 220, ROTOR);
					VGA_box(x1, 200, x2, 220, ROTOR_SELECT);
				}
			}
		}
	}
}

// A function to calculate the place to paint the rotors
void setX() { 
	x1 = (80 * rotorNumber) - 60;
	x2 = (40 * rotorNumber) + 20 + (40 * (rotorNumber - 1));

	x3 = (80 * (rotorNumber + 1)) - 60;
	x4 = (40 * (rotorNumber + 1)) + 20 + (40 * rotorNumber);

	x5 = (80 * (rotorNumber - 1)) - 60;
	x6 = (40 * (rotorNumber - 1)) + 20 + (40 * (rotorNumber - 2));
}

// Prints a line of spaces to "empty" the screen, but doesn't wipe the rotor-configuration
void clearScreen() {  
	int i;
	for (i = 2; i <= 44; i++) {
		VGA_text(0, i,
				"                                                                                                        ");
	}
}

// Clears the whole character buffer
void VGA_text_clear_all(char * character_buffer){ 
	VGA_text_clear(character_buffer, 0, 0, 80, 59);
}

void VGA_text_clear(char * character_buffer, int x1, int y1, int x2, int y2){
    int offset, row, col;

    for (row = y1; row <= y2; row++)
    {
        col = x1;
        while (col <= x2)
        {
            offset = (row << 7) + col;
			// Compute halfword address, set pixel
            *(character_buffer + offset) = ' '; 
            ++col;
        }
    }
}

void PS2() {
	// Reads the data register in the PS/2 port
	PS2_data = *(PS2_ptr);	
	// Extracts the RAVAIL field
	RAVAIL = (PS2_data & 0xFFFF0000) >> 16;	
	if (RAVAIL > 0) {

		byte1 = PS2_data & 0xFF;
		// Clears the buffer
		*PS2_ptr &= 0x00000000; 

		OSTimeDlyHMSM(0, 0, 0, 10);

	}
}

char getKey() {
	// Checks if the received scancode is a BREAKCODE indicator, 0xF0
	if (byte1 != BREAKCODE && byte1 != 0xE0) {
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
	} else if ((tempo >= 'A' && tempo <= 'Z')) {//checks whether or not the received byte is a letter
		if (strlen(ascii) == 1) {
			tempo = tempo + 32;	//makes it a small letter instead of a capital
			return tempo;
		}
	}
	return 0;
}

void printRinginstelling(char c[2], int rotor) { //prints the "ring-instelling"
	int x1;
	x1 = (20 * (rotor - 1)) + 10;
	VGA_text(x1, 52, c);
	if (ringInstellingen[0] < 10) {//because 0-9 is 1 character, when the configuration is below 10 we have to remove the second character from the screen
		VGA_text(11, 52, " ");
	}
	if (ringInstellingen[1] < 10) {
		VGA_text(31, 52, " ");
	}
	if (ringInstellingen[2] < 10) {
		VGA_text(51, 52, " ");
	}
	if (ringInstellingen[3] < 10) {
		VGA_text(71, 52, " ");
	}
}

void printRotorStand(char c[2], int rotor) { //prints the "Rotor-stand"
	int x1;
	x1 = (20 * (rotor - 1)) + 10;
	VGA_text(x1, 47, c);
}
