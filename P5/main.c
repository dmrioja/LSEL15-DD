/*=================================================================
| Archivo: main.c
| Propósito: Implementación de un reloj péndulo con leds
| Documentación: -
|
| Revisiones:
| Fecha    Nombre          Revisión
| -------- --------------- ----------------------------------------
| 04-05-15 Dani y Diego    Created
| 05-05-15 Diego y Dani    Last release
|
===================================================================*/
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "fsm.h"
#include "task.h"
#include "wiringPi.h"
#include <pthread.h>

#define NUMLEDS 8
/* FSM LEDSHOW STATES */
#define ORIGIN 0   /* When infrared go 1 this is the first no light zone. */
#define SHOW_ZR 1  /* Show image from left to right. */
#define GO_R2L 2   /* Zone without light in the right, time to go to the right border and comes to the image zone again.  */
#define SHOW_ZL 3  /* Show image from right to left. */
#define DRAW 4     /* Use no light zone from image zone to redraw image array. */ 
#define WAIT_ISR 5 /* Wait for the interruption from the infrared line. */

/* TIME REQUIREMENTS */
#define PER 1500
#define DLINE 1500
#define STACKSIZE 1024
#define IMG_SIZE 22
#define H1 0
#define H2 5
#define M1 13
#define M2 18

pthread_mutex_t cerrojo_isrBarrier;

static pthread_t infBarrier_sim;
static pthread_t pth_ledshow;

/* GLOBAL VARIABLES */
int enable = 0;
int remSlices = 7;
int infrared_sens = 0;
int actualBlock = 0;
int blocks[5] = {7,22,6,7,22,1};
int currentSlice = 0;
int globalSlice = 0;
int leds[8] = {GPIO0, GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7};
int image[IMG_SIZE];
int hour[4] = {2,1,0,5}; 
int digits[10][4] =  {
  {0xFF,0x81,0x81,0xFF}, /* 0 */
  {0x21,0x41,0x41,0xFF}, /* 1 */
  {0x8F,0x91,0x91,0xF1}, /* 2 */
  {0x91,0x91,0x91,0xFF}, /* 3 */
  {0xF0,0x10,0x10,0xFF}, /* 4 */
  {0xF1,0x91,0x91,0x8F}, /* 5 */
  {0xF1,0x81,0x81,0x8F}, /* 6 */
  {0x80,0x90,0x90,0xFF}, /* 7 */
  {0xFF,0x91,0x91,0xFF}, /* 8 */
  {0xF0,0x90,0x90,0xFF}}; /* 9 */



/* TIME CALC FUNCTIONS */
/* Time subtraction */
void time_sub(struct timespec *start, struct timespec *end, struct timespec *sub){
	sub->tv_sec = end->tv_sec - start->tv_sec;
	if ((end->tv_nsec - start->tv_nsec) < 0) {
	  sub->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
	}
	else {
	  sub->tv_nsec = end->tv_nsec - start->tv_nsec;
	}
}

/* Time addition */
void time_add(struct timespec *start, struct timespec *end, struct timespec *add){
	add->tv_sec = end->tv_sec + start->tv_sec + (end->tv_nsec/1000000000) + (start->tv_nsec/1000000000);
	add->tv_nsec = (end->tv_nsec%1000000000) + (start->tv_nsec%1000000000);
}

/* Delay until next_activation */
void delay_until(struct timespec *next_activation){
	struct timespec now, delay;
	clock_gettime(CLOCK_MONOTONIC,&now);
	time_sub(&now, next_activation, &delay);
	clock_nanosleep(CLOCK_MONOTONIC,0,&delay,NULL);
}



  /* ISR FUNCTIONS */

static void infrared_isr (void) { infrared_sens = 1; }

static void (*input_isr[]) (void) = {infrared_isr};


/* INPUT FUNCTIONS */
static int checkInfrared(fsm_t *this) {
  struct timespec t0,t1,diff = {0,0};
  clock_gettime(CLOCK_MONOTONIC, &t0);
  while(1) {
    pthread_mutex_lock(&cerrojo_isrBarrier);
    if (infrared_sens == 1) {
      infrared_sens = 0;
      pthread_mutex_unlock(&cerrojo_isrBarrier);
      break;
    }
    pthread_mutex_unlock(&cerrojo_isrBarrier);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  time_sub(&t0,&t1,&diff);
  printf("wait in: %d ns\n", diff.tv_nsec);
  return 1;
}

static int checkSlice(fsm_t *this) {
  if (remSlices > 0) {
    remSlices--;
    return 0;
  }
  else {
    actualBlock++;
    remSlices = blocks[actualBlock];
    return 1;
  }
}


/* OUTPUT FUNCTIONS */
static void showFromPanel(int direction) {
	struct timespec next_activation;
	struct timespec period = {0, 15000};
	clock_gettime(CLOCK_MONOTONIC, &next_activation);
	struct timespec t0,t1,t2,diff = {0,0};
	t0 = next_activation;
	int slice;
	if (direction == 0) {
	  //for (slice = currentSlice; slice < IMG_SIZE; slice++){
	  //clock_gettime(CLOCK_MONOTONIC, &t1);
	  int i;
	  for(i = NUMLEDS-1; i >= 0; i--){
	    digitalWrite(leds[i], (image[currentSlice] >> i) & 0x01 );
	  }
	  printf("Current slice = %d", currentSlice);
	  if (currentSlice + 1 < IMG_SIZE) {
	  currentSlice++;
	  }
	  //	clock_gettime(CLOCK_MONOTONIC, &next_activation);
	  //	time_add(&next_activation, &period, &next_activation);
	  //	clock_gettime(CLOCK_MONOTONIC, &t2);
	  //	time_sub(&t1,&t2,&diff);
	  //	printf("Time writing: %d", diff.tv_nsec);
	  //delay_until(&next_activation);
	  //	printf("\n");
	  //}
	  //currentSlice = IMG_SIZE - 1;
	} else {
	  //	for (slice = currentSlice; slice >= 0; slice--){
	  //		clock_gettime(CLOCK_MONOTONIC, &t1);
	  int i;
	  for(i = NUMLEDS-1; i >= 0; i--){
	    digitalWrite(leds[i], (image[currentSlice] >> i) & 0x01 );
	  }
	  printf("Current slice = %d", currentSlice);
	  if (currentSlice - 1 > 0) {
	  currentSlice--;
	  }
	  //clock_gettime(CLOCK_MONOTONIC, &next_activation);
	  //time_add(&next_activation, &period, &next_activation);
	  //clock_gettime(CLOCK_MONOTONIC, &t2);
	  //time_sub(&t1,&t2,&diff);
	  //printf("Time writing: %d ns", diff.tv_nsec);
	  //delay_until(&next_activation);
	  //printf("\n");
	  //}
	  //currentSlice = 0;
	}
	clock_gettime(CLOCK_MONOTONIC, &t2);
	time_sub(&t0,&t2,&diff);
	printf("IMG writed in: %d ns\n", diff.tv_nsec);
}

static void lightLeds_R(fsm_t *this) {
  showFromPanel(0);
}

static void lightLeds_L(fsm_t *this) {
  showFromPanel(1);
}
static void drawPanel(fsm_t *this) {
  int i;
  for(i = 0; i < 4; i++){
    image[H1 + i] = digits[hour[0]][i];
    image[H2 + i] = digits[hour[1]][i];
    image[M1 + i] = digits[hour[2]][i];
    image[M2 + i] = digits[hour[3]][i];
  }   
  actualBlock = 0;
}

static void doNothing(fsm_t *this) {
  return;
}


/* EXECUTION OF LEDSHOW FSM */
static fsm_trans_t tt_ledshow[] = {
	{ORIGIN, checkSlice, SHOW_ZR, lightLeds_R} ,
	{SHOW_ZR, checkSlice, GO_R2L, doNothing} ,
	{GO_R2L, checkSlice, SHOW_ZL, lightLeds_L} ,
	{SHOW_ZL, checkSlice, DRAW, drawPanel} ,
	{DRAW, checkSlice, WAIT_ISR, doNothing} ,
	{WAIT_ISR, checkInfrared, ORIGIN, doNothing} ,
	{-1, NULL, -1, NULL} ,
};


/* LEDSHOW FSM TASK DEFINTION */
static void *task_ledshow(void *arg) {
	struct timespec next_activation;
	struct timespec *period = task_get_period(pthread_self());
	fsm_t *fsm_ledshow = fsm_new(tt_ledshow);
	fsm_ledshow->current_state = ORIGIN;
	clock_gettime(CLOCK_MONOTONIC, &next_activation);
	int counter = 0;
	printf("periodo %d \n", period->tv_nsec);
	drawPanel(fsm_ledshow);
	while(counter < 1000) {
	  printf("State: %d - RemSlices: %d \n",fsm_ledshow->current_state, remSlices);
		fsm_run(fsm_ledshow);
		time_add(&next_activation, period, &next_activation);
		delay_until(&next_activation);
		counter++;
	}
}

/* LEDSHOW FSM TASK DEFINTION */
static void *task_infBarrier(void *arg) {
	struct timespec next_activation;
	struct timespec *period = task_get_period(pthread_self());
	clock_gettime(CLOCK_MONOTONIC, &next_activation);
	int counter = 0;
	printf("Periodo inf %d \n", period->tv_nsec);
	while(counter < 1000) {
	  pthread_mutex_lock(&cerrojo_isrBarrier);
	  wiringPi_gen_interrupt(GPIO8);
	  pthread_mutex_unlock(&cerrojo_isrBarrier);
	  time_add(&next_activation, period, &next_activation);
	  delay_until(&next_activation);
	  counter++;
	}
}
/* DRAW BASE IMAGE 2 POINTS SEPARATOR */
static void init_image() {
  int i;
  for(i = 0; i < IMG_SIZE; i++) {
      image[i] = 0x00;
  }
  image[10] = 0x28;
  image[11] = 0x28;
}

int main(void){
	/* Variable initialization */
	wiringPiSetup ();
	wiringPiISR(GPIO8, INT_EDGE_RISING, input_isr[0]);
	init_image();
	pth_ledshow = task_new("ledshow", task_ledshow,PER,DLINE,1,STACKSIZE);
	infBarrier_sim = task_new("infBarrierSim", task_infBarrier,111111,111111,1,STACKSIZE);
	
	pthread_join(pth_ledshow, NULL);
	
	
	printf("**********  Fin de ejecucion  **********\n");
	return 0;
	
}
//EOF
