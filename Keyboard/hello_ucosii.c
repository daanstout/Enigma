#include <stdio.h>
#include "includes.h"
#include <altera_up_ps2_keyboard.h>
#include <altera_up_avalon_ps2.h>
#include <altera_avalon_pio_regs.h>
#include "altera_up_avalon_parallel_port.h"
#include "string.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK task1_stk[TASK_STACKSIZE];
OS_STK task2_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define BUF_SIZE 500000			// about 10 seconds of buffer (@ 48K samples/sec)#define BUF_THRESHOLD 96		// 75% of 128 word buffer
#define TASK1_PRIORITY      1
#define TASK2_PRIORITY      2
#define BREAKCODE 0xF0

#define PIN_AD15 *(JP5_ptr) + 0x8
#define PIN_AC15 *(JP5_ptr) + 0x4

#define UP '8'
#define RIGHT '6'
#define LEFT '4'
#define DOWN '2'

// GPIO defines

#define RX1 0x01000000 // D24#define TX1 0x02000000 // D25#define RX2 0x04000000 // D26#define TX2 0x08000000 // D27#define RX3 0x10000000 // D28#define TX3 0x20000000 // D29#define RX4 0x40000000 // D30#define TX4 0x80000000 // D31
alt_up_parallel_port_dev *gpio_dev; //	gpio device
// PS2 daan.
int byte1;
char letter;
char * ascii[10];

volatile int * PS2_ptr = (int *) 0x10000100;  // PS/2 port address
volatile int * JP5_ptr = (int *) 0x10000060;  // JP5 GPIO port address
volatile char * character_buffer = (char *) 0x09000000;	// VGA character buffer

int PS2_data, RAVAIL;

//GPIO

int toggle = 0;

//----------prototypes-----------------------------------
//-------------vga---------------------------------------
void VGA_text(int, int, char *);
void VGA_box(int, int, int, int, short);
//-------------ps2---------------------------------------
void PS2();
char getKey();

//------------GPIO---------------------------------------

//send/recieve
void getJP5();
//void setJP5();

//char/binary

void charToBinary(char c, INT8U *binaryChar[15]);
char binaryToChar(INT8U c[15]);

void task1(void* pdata) {
	char strTemp[2];
	while (1) {
//		PS2();
//
//		VGA_box(0, 0, 319, 239, 0);
//
//		strTemp[0] = getKey();
//
//		getJP5();
//		//setJP5();
//
//		VGA_text(5, 5, strTemp);
//		VGA_box(0, 0, 319, 239, 0);

		INT8U test[15] = { 0, 1, 0, 0, 1, 1, 1, 0 };
		unsigned char testChar = binaryToChar(test);

		printf("%c\n", testChar);

		unsigned char test2 = 'N';
		INT8U *testChar2[15];
		charToBinary(testChar, &testChar2);
		int co;
		for (co = 0; co < 8; co++) {
			printf("%d", testChar2[co]);
		}
		printf("\n");

	}
}

int main(void) {
	VGA_box(0, 0, 319, 239, 0);

	gpio_dev = alt_up_parallel_port_open_dev("/dev/Expansion_JP5");	//	DE2-115 gpio
	alt_up_parallel_port_set_port_direction(gpio_dev, 0x00000001);// set D0 as INPUT
	//INPUT is 1
	//OUTPUT is 0

	OSTaskCreateExt(task1,
	NULL, (void *) &task1_stk[TASK_STACKSIZE - 1], TASK1_PRIORITY,
	TASK1_PRIORITY, task1_stk,
	TASK_STACKSIZE,
	NULL, 0);

	OSStart();
	return 0;
}

void VGA_box(int x1, int y1, int x2, int y2, short pixel_color) {
	int offset, row, col;
	volatile short * pixel_buffer = (short *) 0x08000000;	// VGA pixel buffer

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
		//++offset;
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
	} else if ((tempo >= 'A' && tempo <= 'Z')) {//checks wether or not the received byte is a letter
		if (strlen(ascii) == 1) {
			tempo = tempo + 32;	//makes it a small letter instead of a capital
			return tempo;							//returns the character
		}
	}
	return 0;									// returns null
}

void getJP5() {

	int gpio_values;

	toggle = !toggle;		//
	if (toggle) {
		alt_up_parallel_port_write_data(gpio_dev, 0x00000001);	//	set D0 HIGH
	} else {
		alt_up_parallel_port_write_data(gpio_dev, 0x00000000);	//	set D0 LOW
	}

	gpio_values = alt_up_parallel_port_read_data(gpio_dev);	//		Read form the GPIO

	gpio_values &= 0x00000002;		//		extract D1
	if (gpio_values == 0) {
		printf("Off\n");
	} else {
		printf("On\n");
	}
	//OSTimeDlyHMSM(0, 0, 0, 10);
}

//void setJP5() {
//	//*(JP5_ptr) = 0;
//	IOWR_ALTERA_AVALON_PIO_DATA(PIN_AD15, 'c');
//	char tempor = IORD_ALTERA_AVALON_PIO_DATA(PIN_AC15) & 0xFF;
//	printf("set: %d %c\n", tempor, tempor);
//}

void charToBinary(char c, INT8U *binaryChar[15]) {
	INT8U temp[15];
	int i;
	for (i = 0; i < 8; i++) {
		temp[i] = ((c >> i) & 1);
	}
	for(i = 0; i < 8; i++){
		binaryChar[7 - i] = temp[i];
	}
}

char binaryToChar(INT8U c[15]) {
	unsigned char charBinary;
	int i;
	for (i = 0; i < 8; i++) {
		charBinary = charBinary << 1;
		charBinary = charBinary | c[i];
	}
	return charBinary;
}
