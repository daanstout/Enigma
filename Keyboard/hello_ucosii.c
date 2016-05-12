//includes

#include <stdio.h>
#include "includes.h"
#include <altera_up_ps2_keyboard.h>
#include <altera_up_avalon_ps2.h>
#include <altera_avalon_pio_regs.h>
#include "altera_up_avalon_parallel_port.h"
#include "string.h"
//#include "altera_up_avalon_rs232.h"
#include <altera_up_avalon_rs232.h>

//======================tasks========================

//---------------------Defines-----------------------

// Definition of Task Stacks
#define   TASK_STACKSIZE       2048
OS_STK taskMain_stk[TASK_STACKSIZE];

// Definition of Task Priorities
#define BUF_SIZE 500000			// about 10 seconds of buffer (@ 48K samples/sec)#define BUF_THRESHOLD 96		// 75% of 128 word buffer#define TASKMAIN_PRIORITY      1//=======================VGA=========================//---------------------Register----------------------volatile char * character_buffer = (char *) 0x09000000;	// VGA character buffer//--------------------Prototypes---------------------
void VGA_text(int, int, char *);
void VGA_box(int, int, int, int, short);
void setLetterLijst();
void readLetterLijst(char c);

//---------------------Variables---------------------

char letterLijst[26][7][5];

//=======================PS2=========================

//---------------------Register----------------------

volatile int * PS2_ptr = (int *) 0x10000100;  // PS/2 port address

//---------------------Defines-----------------------

#define BREAKCODE	0xF0

// Arrow keys defines
#define UP		'8'
#define RIGHT	'6'
#define LEFT	'4'
#define DOWN	'2'

//--------------------Prototypes---------------------

void PS2();
char getKey();

//---------------------Variables---------------------

int byte1;
char letter;
char * ascii[10];
int PS2_data, RAVAIL;

//=======================GPIO========================

//---------------------Defines-----------------------

#define RX0 0x00000000 // all zero#define TX0 0x00000000 // all zero#define RX1 0x01000000 // D24#define TX1 0x02000000 // D25#define RX2 0x04000000 // D26#define TX2 0x08000000 // D27#define RX3 0x10000000 // D28#define TX3 0x20000000 // D29#define RX4 0x40000000 // D30#define TX4 0x80000000 // D31//--------------------Prototypes---------------------char getChar(int TXPort, int TXShift);void sendChar(char c);void charToBinary(char c, char *binaryChar[10]);char binaryToChar(char c[10]);//---------------------Variables---------------------
int toggle = 0;
int cycleCount = 0;
char test5;
alt_up_parallel_port_dev *gpio_dev; //	gpio device

void alt_up_rs232_enable_read_interrupt(alt_up_rs232_dev *rs232);
void alt_up_rs232_disable_read_interrupt(alt_up_rs232_dev *rs232);



//=====================Functions=====================

void taskMain(void* pdata) {
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
//
//		char test[10] = "01001110";
//		char testChar = binaryToChar(test);
//
//		//unsigned char test2 = 'N';
//		char *testChar2[10];
//		charToBinary(testChar, &testChar2);
//
//		char tempo[10];
//		int i;
//		for (i = 0; i < 8; i++) {
//			tempo[i] = testChar2[i];
//		}
//
//		char testChar3;
//		testChar3 = binaryToChar(tempo);
//		printf("%c %d\n", testChar3, testChar3);

//		test5 = getChar(TX1, 25);
//		printf("%c\n", test5);
//		if (test5 != 0) {
//			OSTimeDlyHMSM(0, 0, 5, 0);
//		}
//		sendChar('N');
		alt_up_parallel_port_write_data(gpio_dev, RX1);
	}
}

int main(void) {
	VGA_box(0, 0, 319, 239, 0);

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

char getChar(int TXPort, int TXShift) {
	int recieved;				//integer that stores the recieved data

	recieved = alt_up_parallel_port_read_data(gpio_dev);//		Read form the GPIO

	recieved &= TXPort;		//		extract TX1 (D24)
	recieved = recieved >> 25;//shift 25 bits to the right so it's either 1 or 0
	if (recieved == 1) {			//if a 1 has been received (start-bit)
		unsigned char charBinary;		//the eventual char that gets returned
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

void setLetterLijst() {
	//letter A
letterLijst[0][1] = {0,0,1,0,0};
letterLijst[0][2] = {0,1,0,1,0};
letterLijst[0][3] = {0,1,1,1,0};
letterLijst[0][4] = {0,1,0,1,0};
letterLijst[0][5] = {0,1,0,1,0};

//letter B
letterLijst[1][1] = {0,1,1,1,0};
letterLijst[1][2] = {0,1,0,1,0};
letterLijst[1][3] = {0,1,1,0,0};
letterLijst[1][4] = {0,1,0,1,0};
letterLijst[1][5] = {0,1,1,1,0};

//letter C
letterLijst[2][1] = {0,1,1,1,0};
letterLijst[2][2] = {0,1,0,0,0};
letterLijst[2][3] = {0,1,0,0,0};
letterLijst[2][4] = {0,1,0,0,0};
letterLijst[2][5] = {0,1,1,1,0};

//letter D
letterLijst[3][1] = {0,1,1,0,0};
letterLijst[3][2] = {0,1,0,1,0};
letterLijst[3][3] = {0,1,0,1,0};
letterLijst[3][4] = {0,1,0,1,0};
letterLijst[3][5] = {0,1,1,0,0};

//letter E
letterLijst[4][1] = {0,1,1,1,0};
letterLijst[4][2] = {0,1,0,0,0};
letterLijst[4][3] = {0,1,1,0,0};
letterLijst[4][4] = {0,1,0,0,0};
letterLijst[4][5] = {0,1,1,1,0};

//letter F
letterLijst[5][1] = {0,1,1,1,0};
letterLijst[5][2] = {0,1,0,0,0};
letterLijst[5][3] = {0,1,1,0,0};
letterLijst[5][4] = {0,1,0,0,0};
letterLijst[5][5] = {0,1,0,0,0};

//letter G
letterLijst[6][1] = {0,1,1,1,0};
letterLijst[6][2] = {0,1,0,0,0};
letterLijst[6][3] = {0,1,0,1,0};
letterLijst[6][4] = {0,1,0,1,0};
letterLijst[6][5] = {0,1,1,1,0};

//letter H
letterLijst[7][1] = {0,1,0,1,0};
letterLijst[7][2] = {0,1,0,1,0};
letterLijst[7][3] = {0,1,1,1,0};
letterLijst[7][4] = {0,1,0,1,0};
letterLijst[7][5] = {0,1,0,1,0};

//letter I
letterLijst[8][1] = {0,1,1,1,0};
letterLijst[8][2] = {0,0,1,0,0};
letterLijst[8][3] = {0,0,1,0,0};
letterLijst[8][4] = {0,0,1,0,0};
letterLijst[8][5] = {0,1,1,1,0};

//letter J
letterLijst[9][1] = {0,0,0,1,0};
letterLijst[9][2] = {0,0,0,1,0};
letterLijst[9][3] = {0,0,0,1,0};
letterLijst[9][4] = {0,1,0,1,0};
letterLijst[9][5] = {0,0,1,0,0};

//letter K
letterLijst[10][1] = {0,1,0,1,0};
letterLijst[10][2] = {0,1,0,1,0};
letterLijst[10][3] = {0,1,1,0,0};
letterLijst[10][4] = {0,1,0,1,0};
letterLijst[10][5] = {0,1,0,1,0};

//letter L
letterLijst[11][1] = {0,1,0,0,0};
letterLijst[11][2] = {0,1,0,0,0};
letterLijst[11][3] = {0,1,0,0,0};
letterLijst[11][4] = {0,1,0,0,0};
letterLijst[11][5] = {0,1,1,0,0};

//letter M
letterLijst[12][1] = {1,0,0,0,1};
letterLijst[12][2] = {1,1,0,1,1};
letterLijst[12][3] = {1,0,1,0,1};
letterLijst[12][4] = {1,0,0,0,1};
letterLijst[12][5] = {1,0,0,0,1};

//letter N
letterLijst[13][1] = {1,0,0,0,1};
letterLijst[13][2] = {1,1,0,0,1};
letterLijst[13][3] = {1,0,1,0,1};
letterLijst[13][4] = {1,0,0,1,1};
letterLijst[13][5] = {1,0,0,0,1};

//letter O
letterLijst[14][1] = {0,0,1,0,0};
letterLijst[14][2] = {0,1,0,1,0};
letterLijst[14][3] = {0,1,0,1,0};
letterLijst[14][4] = {0,1,0,1,0};
letterLijst[14][5] = {0,0,1,0,0};

//letter P
letterLijst[15][1] = {0,1,1,0,0};
letterLijst[15][2] = {0,1,0,1,0};
letterLijst[15][3] = {0,1,1,0,0};
letterLijst[15][4] = {0,1,0,0,0};
letterLijst[15][5] = {0,1,0,0,0};

//letter Q
letterLijst[16][1] = {0,1,1,1,0};
letterLijst[16][2] = {0,1,0,1,0};
letterLijst[16][3] = {0,1,1,1,0};
letterLijst[16][4] = {0,0,1,0,0};
letterLijst[16][5] = {0,0,0,1,0};

//letter R
letterLijst[17][1] = {0,1,1,1,0};
letterLijst[17][2] = {0,1,0,1,0};
letterLijst[17][3] = {0,1,1,1,0};
letterLijst[17][4] = {0,1,1,0,0};
letterLijst[17][5] = {0,1,0,1,0};

//letter S
letterLijst[18][1] = {0,1,1,1,0};
letterLijst[18][2] = {0,1,0,0,0};
letterLijst[18][3] = {0,1,1,1,0};
letterLijst[18][4] = {0,0,0,1,0};
letterLijst[18][5] = {0,1,1,1,0};

//letter T
letterLijst[19][1] = {0,1,1,1,0};
letterLijst[19][2] = {0,0,1,0,0};
letterLijst[19][3] = {0,0,1,0,0};
letterLijst[19][4] = {0,0,1,0,0};
letterLijst[19][5] = {0,0,1,0,0};

//letter U
letterLijst[20][1] = {0,1,0,1,0};
letterLijst[20][2] = {0,1,0,1,0};
letterLijst[20][3] = {0,1,0,1,0};
letterLijst[20][4] = {0,1,0,1,0};
letterLijst[20][5] = {0,1,1,1,0};

//letter V
letterLijst[21][1] = {0,1,0,1,0};
letterLijst[21][2] = {0,1,0,1,0};
letterLijst[21][3] = {0,1,0,1,0};
letterLijst[21][4] = {0,1,0,1,0};
letterLijst[21][5] = {0,0,1,0,0};

//letter W
letterLijst[22][1] = {1,0,0,0,1};
letterLijst[22][2] = {1,0,0,0,1};
letterLijst[22][3] = {1,0,1,0,1};
letterLijst[22][4] = {1,1,0,1,1};
letterLijst[22][5] = {1,0,0,0,1};

//letter X
letterLijst[23][1] = {0,1,0,1,0};
letterLijst[23][2] = {0,1,0,1,0};
letterLijst[23][3] = {0,0,1,0,0};
letterLijst[23][4] = {0,1,0,1,0};
letterLijst[23][5] = {0,1,0,1,0};

//letter Y
letterLijst[24][1] = {0,1,0,1,0};
letterLijst[24][2] = {0,1,0,1,0};
letterLijst[24][3] = {0,0,1,0,0};
letterLijst[24][4] = {0,0,1,0,0};
letterLijst[24][5] = {0,0,1,0,0};

//letter Z
letterLijst[25][1] = {0,1,1,1,0};
letterLijst[25][2] = {0,0,0,1,0};
letterLijst[25][3] = {0,0,1,0,0};
letterLijst[25][4] = {0,1,0,0,0};
letterLijst[25][5] = {0,1,1,1,0};

}

void readLetterLijst(char c, int rotor) {
	int c1, c2;
	for (c1 = 0; c1 < 5; c1++) {
		for (c2 = 0; c2 < 7; c2++) {
			if (letterLijst[c - 'a'][c1][c2] == 1) {
				//drawStuff;
			}
		}
	}
}
