#include <avr/io.h>
#include "USART.h"
#include <util/delay.h>

int main() {
	MY_USART usart = MY_USART();
	usart.initUSART;

	while (1) {
		char string[20] = "";
		char key = usart.receiveByte();
		while (key >= 0) {
			//sprintf(string, "%s%c", key);
			_delay_ms(100);
		}
	}

	return 0;
}
