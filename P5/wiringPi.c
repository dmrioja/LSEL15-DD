#include "wiringPi.h"
#include <stdio.h>

static void (*isr[32]) (void);

int wiringPiSetup (void) {
  return 0;
}

int wiringPiISR (int pin, int edgeType, void (*func)(void)) {
  isr[pin] = func;
  return 0;
}

void wiringPi_gen_interrupt (int pin) {
  if (isr[pin])
    (*isr[pin]) ();
}

/* void digitalWrite(int gpio, int value) { */
/* 	char LED = 'x'; */
/* 	switch (gpio){ */
/* 		case GPIO0: LED = '0'; break; */
/* 		case GPIO1: LED = '1'; break; */
/* 		case GPIO2: LED = '2'; break; */
/* 		case GPIO3: LED = '3'; break; */
/* 		case GPIO4: LED = '4'; break; */
/* 		case GPIO5: LED = '5'; break; */
/* 		case GPIO6: LED = '6'; break; */
/* 		case GPIO7: LED = '7'; break; */

/* 	} */
/* 	printf("LED%c-%d  ", LED, value); */
/* } */

void digitalWrite(int gpio, int value) {
  char LED = '0';
  char PW = ' ';
  if(value == 1) {
    PW = 'X';
  } else {
    PW = ' ';
  }
   
	switch (gpio){
		case GPIO0: LED = PW; break;
		case GPIO1: LED = PW; break;
		case GPIO2: LED = PW; break;
		case GPIO3: LED = PW; break;
		case GPIO4: LED = PW; break;
		case GPIO5: LED = PW; break;
		case GPIO6: LED = PW; break;
		case GPIO7: LED = PW; break;
	default:
	  LED = ' ';
	}
	
	//printf(" %c ", LED);
}
