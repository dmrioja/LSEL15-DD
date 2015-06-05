#ifndef WIRINGPI_H
#define WIRINGPI_H

int wiringPiSetup (void);
int wiringPiISR (int pin, int edgeType, void (*isr)(void));

#define GPIO8 17

#define INT_EDGE_SETUP   0
#define INT_EDGE_FALLING 1
#define INT_EDGE_RISING  2
#define INT_EDGE_BOTH    3

/* interrupt simulation */
void wiringPi_gen_interrupt (int pin);

/* GPIO MANAGEMENT */

#define GPIO0 0
#define GPIO1 1
#define GPIO2 2 
#define GPIO3 3 
#define GPIO4 4
#define GPIO5 5 
#define GPIO6 6
#define GPIO7 7

void digitalWrite(int gpio, int value);

#endif
