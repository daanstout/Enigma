//includes

#include <stdio.h>
#include "includes.h"
#include <altera_up_ps2_keyboard.h>
#include <altera_up_avalon_ps2.h>
#include <altera_avalon_pio_regs.h>
#include "altera_up_avalon_parallel_port.h"
#include "string.h"

//======================tasks========================

//---------------------Defines-----------------------

// Definition of Task Stacks
#define   TASK_STACKSIZE       2048

// Definition of Task Priorities
#define BUF_SIZE 500000			// about 10 seconds of buffer (@ 48K samples/sec)
#define BUF_THRESHOLD 96		// 75% of 128 word buffer
#define TASKMAIN_PRIORITY      1

//---------------------Variables---------------------

OS_STK taskMain_stk[TASK_STACKSIZE];

//=======================VGA=========================

//---------------------Register----------------------

volatile char * character_buffer = (char *) 0x09000000;	// VGA character buffer
volatile short * pixel_buffer = (short *) 0x08000000;

//--------------------Prototypes---------------------

void VGA_text(int, int, char *);
void VGA_box(int, int, int, int, short);
void clearScreen();
void setLetterLijst();
void readLetterLijst(char c, int rotor);

//---------------------Variables---------------------

int y = 5;


//=======================PS2=========================

//---------------------Register----------------------

volatile int * PS2_ptr = (int *) 0x10000100;  // PS/2 port address

//---------------------Defines-----------------------

#define BREAKCODE	0xF0

// Arrow keys defines
#define ENTER	'9'		// enter
#define UP		'8'		// keypad 8
#define ESC		'7'		// escape
#define RIGHT	'6'		// keypad 6
#define BKSP	'5'		// backspace
#define LEFT	'4'		// keypad 4
#define SPACE	'3'		// space
#define DOWN	'2'		// keypad 2
//--------------------Prototypes---------------------

void PS2();
char getKey();

//---------------------Variables---------------------

int byte1;
char letter;
char * ascii[10];
char strTemp[2];
int PS2_data, RAVAIL;

//=======================GPIO========================

//---------------------Defines-----------------------

#define RX0 0x00000000 // all zero
#define TX0 0x00000000 // all zero
#define RX1 0x01000000 // D24
#define TX1 0x02000000 // D25
#define RX2 0x04000000 // D26
#define TX2 0x08000000 // D27
#define RX3 0x10000000 // D28
#define TX3 0x20000000 // D29
#define RX4 0x40000000 // D30
#define TX4 0x80000000 // D31
//--------------------Prototypes---------------------

char getChar(int TXPort, int TXShift);
void sendChar(char c);

void charToBinary(char c, char *binaryChar[10]);
char binaryToChar(char c[10]);

//---------------------Variables---------------------
int toggle = 0;
int cycleCount = 0;
char test5;
alt_up_parallel_port_dev *gpio_dev; //	gpio device

//=====================Functions=====================

void taskMain(void* pdata) {
	int count = 5;
	while (1) {
		PS2();

		VGA_box(0, 0, 319, 239, 0);

		strTemp[0] = getKey();
		strTemp[1] = '\0';

		if (count <= 5) {
			count = 5; //30 tekens per regel
		} else if (count >= 35) {
			count = 5;
			y++;
		} else if (y <= 5) {
			y = 5;
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
				VGA_text(count + 40, y, strTemp);
				strTemp[0] = 0;
				count++;

			}
		}
		VGA_text(5, 5, strTemp);
		VGA_box(0, 0, 319, 239, 0);
	}
}

int main(void) {
	VGA_box(0, 0, 319, 239, 0);
	setLetterLijst();

	gpio_dev = alt_up_parallel_port_open_dev("/dev/Expansion_JP5");	//	DE2-115 gpio
	alt_up_parallel_port_set_port_direction(gpio_dev, TX1 + TX2 + TX3 + TX4);// set D0 as INPUT
	//INPUT is 1
	//OUTPUT is 0

	OSTaskCreateExt(taskMain,
	NULL, (void *) &taskMain_stk[TASK_STACKSIZE - 1], TASKMAIN_PRIORITY,
	TASKMAIN_PRIORITY, taskMain_stk,
	TASK_STACKSIZE,
	NULL, 0);

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

void clearScreen() {
	int i;
	for (i = 0; i <= 60; i++) {
		VGA_text(0, i,
				"                                                                                                        ");
	}
}

void PS2() {
	PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
	RAVAIL = (PS2_data & 0xFFFF0000) >> 16;	// extract the RAVAIL field
	if (RAVAIL > 0) {
		/* always save the last three bytes received */
		byte1 = PS2_data & 0xFF;
	}
}

char getKey() {
	if (byte1 != BREAKCODE) {//checks if the received scancode is a BREAKCODE indicator, 0xF0
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
	} else if ((tempo >= 'A' && tempo <= 'Z')) {//checks wether or not the received byte is a letter
		if (strlen(ascii) == 1) {
			tempo = tempo + 32;	//makes it a small letter instead of a capital
			return tempo;							//returns the character
		}
	}
	return 0;									// returns null
}

char getChar(int TXPort, int TXShift) {
	int recieved;				//integer that stores the recieved data

	recieved = alt_up_parallel_port_read_data(gpio_dev);//		Read form the GPIO

	recieved &= TXPort;		//		extract TX1 (D24)
	recieved = recieved >> 25;//shift 25 bits to the right so it's either 1 or 0
	if (recieved == 1) {			//if a 1 has been received (start-bit)
		unsigned char charBinary;	//the eventual char that gets returned
		int i;
		for (i = 0; i < 8; i++) {		//loop 8 times to get 1 byte
			OSTimeDlyHMSM(0, 0, 0, 125);//wait 125 milliseconds before receiving the bit

			recieved = alt_up_parallel_port_read_data(gpio_dev);//		Read form the GPIO

			recieved &= TXPort;		//		extract TX1 (D24)
			recieved = recieved >> TXShift;	//shift 25 bits to the right so it's either 1 or 0

			charBinary = charBinary << 1;//shift charBinary 1 position to the left so bits don't overlap
			charBinary = (charBinary | recieved);//or the recieved bit with charBinary so it gets added to the char
			recieved = 0;		//reset recieved
		}
		OSTimeDlyHMSM(0, 0, 0, 125);//wait 125 milliseconds for the line to become 0 again so the function doesn't start without there actually being input
		return charBinary;		//return charBinary
	} else {
		return 0;		//incase there was nothing to get, return 0
	}

	/*	to make this function work on the Arduino, you need the following:
	 *
	 *	you need to connect a port on the Arduino to a GPIO port on the DE2-115 board
	 *	set that port as OUTPUT on the Arduino
	 *	set the port as HIGH when you are ready to send a byte over (the number of bits can be increased by increasing the number of times the for loop loops)
	 *	add a 5 millisecond delay, all this delay has to do is make sure the board has the time to recognize there is a start bit
	 *	change the port as fit with the bits needed to be send, a HIGH for a 1 and a LOW for a 0
	 *	add a 125 millisecond delay between setting the port
	 *	afterwards add another 125 millisecond delay to make sure the board has time to read the last bit
	 *	set the port on the Arduino as LOW so the function doesn't start again if the last bit were to be 1
	 *
	 */

//	int gpio_values;
//
//	toggle = !toggle;		//
//	if (toggle) {
//		alt_up_parallel_port_write_data(gpio_dev, 0x00000001);	//	set D0 HIGH
//	} else {
//		alt_up_parallel_port_write_data(gpio_dev, 0x00000000);	//	set D0 LOW
//	}
//
//	gpio_values = alt_up_parallel_port_read_data(gpio_dev);	//		Read form the GPIO
//
//	gpio_values &= 0x00000002;		//		extract D1
//	if (gpio_values == 0) {
//		printf("Off\n");
//	} else {
//		printf("On\n");
//	}
}

void sendChar(char c) {
	alt_up_parallel_port_write_data(gpio_dev, RX1);
	OSTimeDlyHMSM(0, 0, 0, 5);
	int i;
	int s;
	for (i = 0; i < 8; i++) {
		s = (c >> i) & 1;
		if (s == 1) {
			alt_up_parallel_port_write_data(gpio_dev, RX1);
		} else if (s == 0) {
			alt_up_parallel_port_write_data(gpio_dev, RX0);
		}
		OSTimeDlyHMSM(0, 0, 0, 125);
	}
	alt_up_parallel_port_write_data(gpio_dev, RX0);
}

void charToBinary(char c, char *binaryChar[10]) {
	char temp[10];			//temporary array for storage
	int i;
	for (i = 0; i < 8; i++) {//loop 8 times so each bit gets added to the array
		temp[i] = (((c >> i) & 1) + '0');//shift char c i times to the right, and it with 1 and add char 0 to make it a char
	}
	for (i = 0; i < 8; i++) {
		binaryChar[7 - i] = temp[i];//move the 8 bits from the temporary array to the array given in as a parameter, bit by bit
	}
}

char binaryToChar(char c[10]) {
	unsigned char charBinary;	//char to be returned
	int i;

	for (i = 0; i < 8; i++) {
		charBinary = charBinary << 1;//shift charBinary 1 bit to the left so bits don't overlap
		charBinary = (charBinary | (c[i] - '0'));//or charBinary with the bit at i position and substract char 0 from it to make it an int
	}
	return charBinary;		//return charBinary
}

//void VGA_img(int x1, int y1, int x2, int y2) {
//	int offset, row, col;
//	// VGA pixel buffer
//
//	/* assume that the box coordinates are valid */
//	for (row = y1; row <= y2; row++) {
//		col = x1;
//		while (col <= x2) {
//			offset = (row << 9) + col;
//			*(pixel_buffer + offset) = (int) img;// compute halfword address, set pixel
//			++col;
//		}
//	}
//}
