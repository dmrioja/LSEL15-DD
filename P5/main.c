/*=================================================================
| Archivo: main.c
| Propósito: Implementación de un reloj péndulo con leds
| Documentación: -
|
| Revisiones:
| Fecha    Nombre          Revisión
| -------- --------------- ----------------------------------------
| 04-06-15 Dani y Diego    Created
| 16-06-15 Diego y Dani    Last release
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
#include "screen.h"

#define NUMLEDS 8
/* FSM LEDSHOW STATES */
#define SHOW_ZRL 0  /* Show image from left to right. */

/* TIME REQUIREMENTS */
#define PER 111111
#define DLINE 111111
#define STACKSIZE 1024
#define IMG_SIZE 22
#define H1 0
#define H2 5
#define M1 13
#define M2 18

pthread_mutex_t cerrojo_isrBarrier;
static pthread_t pth_infBarrier_sim;
static pthread_t pth_ledshow;

/* GLOBAL VARIABLES */
int infrared_sens = 0;
int currentSlice = 0;
int leds[8] = {GPIO0, GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7};
int image[IMG_SIZE];
long maxTask = 0;
/*LA HORA QUE QUEREMOS PINTAR 21:34 */
int hour[4] = {2,1,3,4}; 

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
/* La funcion de entrada del unico estado de la fsm realiza una espera
   activa sobre el terminal de interrupcion de la barrera de
   infrarrojos que sera activada por otra tarea que se lanza a 9Hz, de
   este modo se sincroniza HW y SW, la interrupcion se ha simulado con
   otra task.
*/
static int checkInfrared(fsm_t *this) {
	struct timespec t0,t1,diff = {0,0};
	//clock_gettime(CLOCK_MONOTONIC, &t0);
	while(1) {
		pthread_mutex_lock(&cerrojo_isrBarrier);
		if (infrared_sens == 1) {
			infrared_sens = 0;
			pthread_mutex_unlock(&cerrojo_isrBarrier);
			break;
		}
		pthread_mutex_unlock(&cerrojo_isrBarrier);
	}
	//clock_gettime(CLOCK_MONOTONIC, &t1);
	//time_sub(&t0,&t1,&diff);
	//printf("Waiting for ISR: %lu ns\n", diff.tv_nsec);
	return 1;
}

/* REDRAW FUNCTIONS */
static void drawPanel(fsm_t *this) {
	int i;
	for(i = 0; i < 4; i++){
		image[H1 + i] = digits[hour[0]][i];
		image[H2 + i] = digits[hour[1]][i];
		image[M1 + i] = digits[hour[2]][i];
		image[M2 + i] = digits[hour[3]][i];
	}   
}


/* OUTPUT FUNCTIONS */
static void showFromPanel(fsm_t *this) {
	
	struct timespec next_activation;
	struct timespec period = {0, 1543210};
	struct timespec beforeR = {0, 7*1543210};
	struct timespec fromR2L = {0, 14*1543210};
	int slice;
	struct timespec t0,t1,t2,diff = {0,0};
	/* En este tramo se espera sin dibujar nada hasta llegar a la
	 * zona de pintado, centrada. Se recorren 7 de las 36 slice en
	 * las que se ha dividido la zona visible del reloj, esto
	 * equivale a pasar 1/9*1/2*1/36*7 = 7*1543210 ns sin dibujar
	 * nada.
	 */
	clock_gettime(CLOCK_MONOTONIC, &next_activation);
	t0 = next_activation;
	time_add(&next_activation, &beforeR, &next_activation);
	delay_until(&next_activation);
	//time_sub(&t0,&next_activation,&diff);
	//printf("INIT R: %lu \n", diff.tv_nsec);
	/* A partir de aqui se empieza a dibujar hacia la derecha, son
	 * 22 slices con el periodo indicado arriba.
	 */
	for (slice = currentSlice; slice < IMG_SIZE; slice++){
	  	//clock_gettime(CLOCK_MONOTONIC, &t1);
		int i;
		for(i = NUMLEDS-1; i >= 0; i--){
			digitalWrite(leds[i], (image[slice] >> i) & 0x01 );
		}
		//printf(" Current slice = %lu", slice);
		
		//clock_gettime(CLOCK_MONOTONIC, &t2);
		//time_sub(&t1,&t2,&diff);
		time_add(&next_activation, &period, &next_activation);
		//printf(" Time writing: %lu", diff.tv_nsec);
		delay_until(&next_activation);
		//printf("\n");
	}
	currentSlice = IMG_SIZE - 1;
	/* Una vez se ha terminado de pintar hay que dejar un tiempo
	 * para llegar hasta el final de la zona visible y volver para
	 * volver a dibujar, esta vez hacia la izquierda */
	
	//time_sub(&t0,&next_activation,&diff);
	//printf("END R: %lu \n", diff.tv_nsec);
	time_add(&next_activation, &fromR2L, &next_activation);
	delay_until(&next_activation);
	//time_sub(&t0,&next_activation,&diff);
	//printf("INIT L: %lu \n", diff.tv_nsec);
	/* Comenzamos la escritura hacia la izquierda, en el
	 * archivo de test se puede apreciar que por cada
	 * ejecucion del fsm_run aparecen dos dibujos
	 * opuestos, eso significa el recorrido a izquierdas o
	 * derechas */
	for (slice = currentSlice; slice >= 0; slice--){
		//clock_gettime(CLOCK_MONOTONIC, &t1);
		int i;
		for(i = NUMLEDS-1; i >= 0; i--){
			digitalWrite(leds[i], (image[slice] >> i) & 0x01 );
		}
		//printf(" Current slice = %lu", slice);
		
		//clock_gettime(CLOCK_MONOTONIC, &t2);
		//time_sub(&t1,&t2,&diff);
		time_add(&next_activation, &period, &next_activation);
		//printf(" Time writing: %lu ns", diff.tv_nsec);
		delay_until(&next_activation);
		//printf("\n");
	}
	currentSlice = 0;
	/* Una pintado a izquierdas, procedemos a repintar el mapa
	 * interno de la imagen a pintar, en este caso, la hora, para
	 * la X solo habria que sustituir la función de pintado y
	 * dejar fija la X sin necesidad de pintar */
	
	//time_sub(&t0,&next_activation,&diff);
	//printf("END L AND IMG REDRAW: %lu \n", diff.tv_nsec);
	drawPanel(this);
	clock_gettime(CLOCK_MONOTONIC, &t1);
	time_sub(&t0,&t1,&diff);
	if(diff.tv_nsec > maxTask){
		maxTask = diff.tv_nsec;
	}
		
	printf("END IMG REDRAW : %lu \n", diff.tv_nsec);
	/* Se acaba de repintar y acabamos la ejecucion, en este punto
	 * se cumplen los plazos de tiempo ya que normalmente la
	 * ejecución de toda esta función tarda en torno a 105ms en el ordenador, nos
	 * queda tiempo para domir la ejecución, despertar y esperar a
	 * la interrupción por el infrarrojo y sincronizarnos con el
	 * HW, otra opción habría sido implementar un reactor donde la
	 * ejecución de esta función vendría impuesta por la línea de
	 * interrupción de los infrarrojos.*/
}

/* EXECUTION OF LEDSHOW FSM */
static fsm_trans_t tt_ledshow[] = {
  {SHOW_ZRL, checkInfrared, SHOW_ZRL, showFromPanel},
  {-1, NULL, -1, NULL}
};


/* LEDSHOW FSM TASK DEFINTION */
static void *task_ledshow(void *arg) {
	struct timespec next_activation;
	struct timespec *period = task_get_period(pthread_self());
	struct timespec t0,t1,diff = {0,0};
	fsm_t *fsm_ledshow = fsm_new(tt_ledshow);
	fsm_ledshow->current_state = SHOW_ZRL;
	int counter = 0;
	printf("periodo %lu \n", period->tv_nsec);
	drawPanel(fsm_ledshow);
	clock_gettime(CLOCK_MONOTONIC, &next_activation);
	while(counter < 100) {
		//clock_gettime(CLOCK_MONOTONIC, &t0);
		fsm_run(fsm_ledshow);
		//clock_gettime(CLOCK_MONOTONIC, &t1);
		//printf("FSM Exec Time: %lu \n", diff.tv_nsec);
		//printf("#######################################\n");
		time_add(&next_activation, period, &next_activation);
		delay_until(&next_activation);
		counter++;
	}
	printf("MAX TIME OF EXECUTION: %lu",maxTask);
}

/* LEDSHOW FSM TASK DEFINTION */
static void *task_infBarrier(void *arg) {
	struct timespec next_activation;
	struct timespec *period = task_get_period(pthread_self());
	clock_gettime(CLOCK_MONOTONIC, &next_activation);
	int counter = 0;
	printf("Periodo inf %lu \n", period->tv_nsec);
	while(counter < 120) {
	  pthread_mutex_lock(&cerrojo_isrBarrier);
	  wiringPi_gen_interrupt(GPIO8); /* LANZAMOS LA INTERRUPCION*/
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
	/* INICIALIZAMOS EL PANEL DE IMAGEN */
	init_image();

	pth_ledshow = task_new("ledshow", task_ledshow,PER,DLINE,1,STACKSIZE);
	pth_infBarrier_sim = task_new("infBarrierSim", task_infBarrier,111111,111111,1,STACKSIZE);

	pthread_join(pth_ledshow, NULL);
	pthread_join(pth_infBarrier_sim, NULL);
			
	printf("\n**********  Fin de ejecucion  **********\n");

	return 0;
	
}
//EOF
