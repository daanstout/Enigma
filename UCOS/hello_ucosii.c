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
#define TASK_STACKSIZE       2048
#define BUF_SIZE 500000			// about 10 seconds of buffer (@ 48K samples/sec)
#define BUF_THRESHOLD 96		// 75% of 128 word buffer
#define TASK1_PRIORITY      10
#define TASK2_PRIORITY		20

// Definition of Task Priorities

#define BREAKCODE	0xF0
// key defines
#define ENTER	'9'		// enter
#define UP		'8'		// keypad 8
#define ESC		'7'		// escape
#define RIGHT	'6'		// keypad 6
#define BKSP	'5'		// backspace
#define LEFT	'4'		// keypad 4
#define SPACE	'3'		// space
#define DOWN	'2'		// keypad 2
#define TAB 	'1'		// Tab

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
//---------------------Variables---------------------
OS_STK * task1_stk[TASK_STACKSIZE];
OS_STK * task2_stk[TASK_STACKSIZE];
//=======================VGA=========================

//---------------------Register----------------------

volatile char * character_buffer = (char *) 0x09000000;	// VGA character buffer
volatile short * pixel_buffer = (short *) 0x08000000;

//--------------------Prototypes---------------------
void VGA_text(int, int, char *);
void VGA_box(int, int, int, int, short);
void clearScreen();

//---------------------Variables---------------------

int y = 5;

//=======================PS2=========================

//---------------------Register----------------------

volatile int * PS2_ptr = (int *) 0x10000100;  // PS/2 port address

//---------------------Defines-----------------------

//--------------------Prototypes---------------------
void PS2();
char getKey();
void select(char c);
void typen(char c);
//void setLetterLijst();
//void readLetterLijst(char c, int rotor);
//void readCijferLijst(char c, int rotor);
//---------------------Variables---------------------

int byte1;
char letter;
char * ascii[10];
char strTemp[2];
char arrowTemp[2];
int PS2_data, RAVAIL;
int rotorNummer;
int count = 5;
int rij;
int letterLijst[5][7];
short cijferLijst[5][7];

//=======================GPIO========================

//---------------------Defines-----------------------

//--------------------Prototypes---------------------
//char getChar(int TXPort, int TXShift);
//void sendChar(char c);
//
//void charToBinary(char c, char *binaryChar[10]);
//char binaryToChar(char c[10]);

//---------------------Variables---------------------
int toggle = 0;
int cycleCount = 0;
char test5;

int countercount = 0;
//alt_up_parallel_port_dev *gpio_dev; //	gpio device

//=====================Functions=====================

void task1(void* pdata) {
	while (1) {
		PS2();
		strTemp[0] = getKey();
		strTemp[1] = '\0';

		if(strTemp[0] == UP || strTemp[0] == DOWN || strTemp[0] == RIGHT || strTemp[0] == LEFT || strTemp[0] == TAB)
			select(strTemp[0]);
		else
			typen(strTemp[0]);

		//printf("%d\n",countercount);
	}
}
//void task2(void *pdata){
//	while(1){
//
//		PS2();
//		select();
//	}
//}

int main(void) {
	VGA_box(0, 0, 160, 240, 0);
	VGA_box(161, 0, 320, 240, 0);
	VGA_box(60, 180, 100, 220, 0x8410);
	VGA_box(140, 180, 180, 220, 0x8410);
	VGA_box(220, 180, 260, 220, 0x8410);
	//	VGA_box(0,0,320,240, 0);

	//gpio_dev = alt_up_parallel_port_open_dev("/dev/Expansion_JP5");	//	DE2-115 gpio
	//alt_up_parallel_port_set_port_direction(gpio_dev, TX1 + TX2 + TX3 + TX4);// set D0 as INPUT
	//INPUT is 1
	//OUTPUT is 0

	OSTaskCreate(task1, (void*) 0, &task1_stk[TASK_STACKSIZE - 1], TASK1_PRIORITY);
//	OSTaskCreate(task2, (void*) 0, &task2_stk[TASK_STACKSIZE - 1], TASK2_PRIORITY);
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

	strTemp[0] = c;
	strTemp[1] = '\0';

	if (count <= 5) {
		count = 5; //30 tekens per regel
	} else if (count >= 35) {
		count = 5;
		y++;
	} else if (y <= 5) {
		y = 5;
	}
	//printf("getKey: %c\n", strTemp[0]);

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
}

void select(char c) {

	arrowTemp[0] = c;
	arrowTemp[1] = '\0';

	if (arrowTemp[0] == TAB) {
		if (rij == 1) {
			rij = 2;
			if (rotorNummer == 1) {
				VGA_box(60, 180, 100, 200, 0x8410);
				VGA_box(60, 200, 100, 220, 0xF800);
			} else if (rotorNummer == 2) {
				VGA_box(140, 180, 180, 200, 0x8410);
				VGA_box(140, 200, 180, 220, 0xF800);
			} else {
				VGA_box(220, 180, 260, 200, 0x8410);
				VGA_box(220, 200, 260, 220, 0xF800);
			}
		} else {
			rij = 1;
			if (rotorNummer == 1) {
				VGA_box(60, 180, 100, 200, 0xF800);
				VGA_box(60, 200, 100, 220, 0x8410);
			} else if (rotorNummer == 2) {
				VGA_box(140, 180, 180, 200, 0xF800);
				VGA_box(140, 200, 180, 220, 0x8410);
			} else {
				VGA_box(220, 180, 260, 200, 0xF800);
				VGA_box(220, 200, 260, 220, 0x8410);
			}
		}
	}

	if (arrowTemp[0] == UP) {
		//rotor ++
		//if(rotor > 26) rotor = 1;
		//if rij 1 -> letters ++
		//if rij 2 -> cijfers ++
	} else if (arrowTemp[0] == DOWN) {
		//rotor--
		//if(rotor < 1) rotor = 26;
		//if rij 1 -> letters --
		//if rij 2 -> cijfers --
	} else if (arrowTemp[0] == LEFT) {
		//een rotor naar links
		if (rotorNummer < 1) {
			rotorNummer = 1;
		} else if (rotorNummer > 1) {
			rotorNummer--;
			if (rotorNummer == 2) {
				if (rij == 1) {
					VGA_box(140, 180, 180, 200, 0xF800);
					VGA_box(220, 180, 260, 200, 0x8410);
				} else {
					VGA_box(140, 200, 180, 220, 0xF800);
					VGA_box(220, 200, 260, 220, 0x8410);
				}
			} else if (rotorNummer == 1) {
				if (rij == 1) {
					VGA_box(60, 180, 100, 200, 0xF800);
					VGA_box(140, 180, 180, 200, 0x8410);
				} else {

					VGA_box(60, 200, 100, 220, 0xF800);
					VGA_box(140, 200, 180, 220, 0x8410);
				}
			}
		}
	} else if (arrowTemp[0] == RIGHT) {
		//een rotor naar rechts
		if (rotorNummer > 3) {
			rotorNummer = 3;
		} else if (rotorNummer < 3) {
			rotorNummer++;
			if (rotorNummer == 2) {
				if (rij == 1) {
					VGA_box(60, 180, 100, 200, 0x8410);
					VGA_box(140, 180, 180, 200, 0xF800);
				} else {

					VGA_box(60, 200, 100, 220, 0x8410);
					VGA_box(140, 200, 180, 220, 0xF800);
				}
			} else if (rotorNummer == 3) {
				if (rij == 1) {
					VGA_box(140, 180, 180, 200, 0x8410);
					VGA_box(220, 180, 260, 200, 0xF800);
				} else {
					VGA_box(140, 200, 180, 220, 0x8410);
					VGA_box(220, 200, 260, 220, 0xF800);
				}
			}
		}

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
		byte1 = PS2_data & 0xFF;
		*PS2_ptr &= 0x00000000;
	}
}

char getKey() {
	if (byte1 != BREAKCODE && byte1 != 0xE0) {//checks if the received scancode is a BREAKCODE indicator, 0xF0
		translate_make_code(KB_ASCII_MAKE_CODE, (char *) byte1, &ascii);//if not, translates it to ascii
		byte1 = 0;
	}

	char tempo = (char *) ascii[0];
	//printf("temp: %c\n", tempo);
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
//void setLetterLijst() {
//	//			5  7
//	letterLijst[0][1] = 0x00403000;
//	letterLijst[0][2] = 0x00403000;
//	letterLijst[0][3] = 0x00403000;
//	letterLijst[0][4] = 0x00403000;
//	letterLijst[0][5] = 0x00403000;
//
//	letterLijst[1][1] = 0x03BF8DFA;
//	letterLijst[1][2] = 0x01B7FCFF;
//	letterLijst[1][3] = 0x0037CCFF;
//	letterLijst[1][4] = 0x02F2CEFF;
//	letterLijst[1][5] = 0x02968DFB;
//
//	letterLijst[2][1] = 0x020FC17F;
//	letterLijst[2][2] = 0x00080100;
//	letterLijst[2][3] = 0x03CFB5B3;
//	letterLijst[2][4] = 0x010A0100;
//	letterLijst[2][5] = 0x033C4B5E;
//
//	letterLijst[3][1] = 0x03BF87F4;
//	letterLijst[3][2] = 0x03B3D68B;
//	letterLijst[3][3] = 0x0037C2C9;
//	letterLijst[3][4] = 0x00F566CB;
//	letterLijst[3][5] = 0x02970DD5;
//
//	letterLijst[4][1] = 0x00403000;
//	letterLijst[4][2] = 0x00403000;
//	letterLijst[4][3] = 0x00403000;
//	letterLijst[4][4] = 0x00403000;
//	letterLijst[4][5] = 0x00403000;
//
//	cijferLijst[1][1] = 0x00B8;
//	cijferLijst[1][2] = 0x0377;
//	cijferLijst[1][3] = 0x0071;
//	cijferLijst[1][4] = 0x0141;
//	cijferLijst[1][5] = 0x022E;
//
//	cijferLijst[2][1] = 0x03EF;
//	cijferLijst[2][2] = 0x0002;
//	cijferLijst[2][3] = 0x03FB;
//	cijferLijst[2][4] = 0x0086;
//	cijferLijst[2][5] = 0x03EF;
//
//	cijferLijst[3][1] = 0x00F0;
//	cijferLijst[3][2] = 0x039D;
//	cijferLijst[3][3] = 0x0215;
//	cijferLijst[3][4] = 0x0379;
//	cijferLijst[3][5] = 0x0016;
//}
//
//void readLetterLijst(char c, int rotor) {
//	int c1, c2, bit;
//	for (c1 = 0; c1 < 5; c1++) {
//		for (c2 = 0; c2 < 7; c2++) {
//			bit = (letterLijst[c1][c2] >> (c - 'a')) & 1;
//			if (bit) {
//				VGA_box(c1 * 2, c2 * 2, c1 * 2 + 1, c2 * 2 + 1, 0xFFFF);
//			}
//		}
//	}
//}
//
//void readCijferLijst(char c, int rotor) {
//	int c1, c2, bit;
//	for (c1 = 0; c1 < 5; c1++) {
//		for (c2 = 0; c2 < 7; c2++) {
//			bit = (cijferLijst[c1][c2] >> (c - '0')) & 1;
//			if (bit) {
//				VGA_box(c1 * 2, c2 * 2, c1 * 2 + 1, c2 * 2 + 1, 0xFFFF);
//			}
//		}
//	}
//}

//char getChar(int TXPort, int TXShift) {
//	int recieved;				//integer that stores the recieved data
//
//	recieved = alt_up_parallel_port_read_data(gpio_dev);//		Read form the GPIO
//
//	recieved &= TXPort;		//		extract TX1 (D24)
//	recieved = recieved >> 25;//shift 25 bits to the right so it's either 1 or 0
//	if (recieved == 1) {			//if a 1 has been received (start-bit)
//		unsigned char charBinary;	//the eventual char that gets returned
//		int i;
//		for (i = 0; i < 8; i++) {		//loop 8 times to get 1 byte
//			OSTimeDlyHMSM(0, 0, 0, 125);//wait 125 milliseconds before receiving the bit
//
//			recieved = alt_up_parallel_port_read_data(gpio_dev);//		Read form the GPIO
//
//			recieved &= TXPort;		//		extract TX1 (D24)
//			recieved = recieved >> TXShift;	//shift 25 bits to the right so it's either 1 or 0
//
//			charBinary = charBinary << 1;//shift charBinary 1 position to the left so bits don't overlap
//			charBinary = (charBinary | recieved);//or the recieved bit with charBinary so it gets added to the char
//			recieved = 0;		//reset recieved
//		}
//		OSTimeDlyHMSM(0, 0, 0, 125);//wait 125 milliseconds for the line to become 0 again so the function doesn't start without there actually being input
//		return charBinary;		//return charBinary
//	} else {
//		return 0;		//incase there was nothing to get, return 0
//	}

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
//}
//void sendChar(char c) {
//	alt_up_parallel_port_write_data(gpio_dev, RX1);
//	OSTimeDlyHMSM(0, 0, 0, 5);
//	int i;
//	int s;
//	for (i = 0; i < 8; i++) {
//		s = (c >> i) & 1;
//		if (s == 1) {
//			alt_up_parallel_port_write_data(gpio_dev, RX1);
//		} else if (s == 0) {
//			alt_up_parallel_port_write_data(gpio_dev, RX0);
//		}
//		OSTimeDlyHMSM(0, 0, 0, 125);
//	}
//	alt_up_parallel_port_write_data(gpio_dev, RX0);
//}
//
//void charToBinary(char c, char *binaryChar[10]) {
//	char temp[10];			//temporary array for storage
//	int i;
//	for (i = 0; i < 8; i++) {//loop 8 times so each bit gets added to the array
//		temp[i] = (((c >> i) & 1) + '0');//shift char c i times to the right, and it with 1 and add char 0 to make it a char
//	}
//	for (i = 0; i < 8; i++) {
//		binaryChar[7 - i] = temp[i];//move the 8 bits from the temporary array to the array given in as a parameter, bit by bit
//	}
//}
//
//char binaryToChar(char c[10]) {
//	unsigned char charBinary;	//char to be returned
//	int i;
//
//	for (i = 0; i < 8; i++) {
//		charBinary = charBinary << 1;//shift charBinary 1 bit to the left so bits don't overlap
//		charBinary = (charBinary | (c[i] - '0'));//or charBinary with the bit at i position and substract char 0 from it to make it an int
//	}
//	return charBinary;		//return charBinary
//}
