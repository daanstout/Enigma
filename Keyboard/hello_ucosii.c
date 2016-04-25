#include <stdio.h>
#include "includes.h"
#include <altera_up_ps2_keyboard.h>
#include <altera_up_avalon_ps2.h>

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK task1_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define BUF_SIZE 500000			// about 10 seconds of buffer (@ 48K samples/sec)
#define BUF_THRESHOLD 96		// 75% of 128 word buffer
#define breakcode 0xF0
#define TASK1_PRIORITY      1

// PS2 daan.
int byte1;
char letter;
char *ascii[5];
char temp = '0';

volatile int * PS2_ptr = (int *) 0x10000100;  // PS/2 port address
volatile char * character_buffer = (char *) 0x09000000;	// VGA character buffer
int PS2_data, RAVAIL;

//receives data from the keyboard
void PS2() {
	PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
	RAVAIL = (PS2_data & 0xFFFF0000) >> 16;	// extract the RAVAIL field
	if (RAVAIL > 0){
		/* always save the last three bytes received */
		byte1 = PS2_data & 0xFF;
	}
}

//translates the scancode to ASCII-code
char getKey(){
	if(byte1 != breakcode){				//checks if the received scancode is a breakcode indicator, 0xF0
		translate_make_code(KB_ASCII_MAKE_CODE, (char *) byte1, &ascii);			//if not, translates it to ascii
	}
	printf("%s", ascii);
	char tempo = ascii[0];
	if((tempo >='A' && tempo <= 'Z')){			//checks wether or not the received byte is a letter
		tempo = tempo + 32;						//makes it a small letter instead of a capital
		return tempo;							//returns the character
	}
	return 0;									// returns null
}

//prints text to the screen
void VGA_text(int x, int y, char * text_ptr) {
	int offset;

	/* assume that the text string fits on one line */
	offset = (y << 7) + x;
	while (*(text_ptr)) {
		*(character_buffer + offset) = *(text_ptr);	// write to the character buffer
		++text_ptr;
	}
}

//prints boxes to the screen
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

//keeps the whole thing running
void task1(void* pdata) {
	char strTemp[2];
	while (1) {
			PS2();

			VGA_box(0, 0, 319, 239, 0);

			strTemp[0] = getKey();

			VGA_text(5, 5, strTemp);
			VGA_box(0, 0, 319, 239, 0);
	}
}

int main(void) {
	VGA_box(0, 0, 319, 239, 0);

	OSTaskCreateExt(task1,
	NULL, (void *) &task1_stk[TASK_STACKSIZE - 1],
	TASK1_PRIORITY,
	TASK1_PRIORITY, task1_stk,
	TASK_STACKSIZE,
	NULL, 0);

	OSStart();
	return 0;
}
