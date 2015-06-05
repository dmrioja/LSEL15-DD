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

/* FSM LEDSHOW STATES */
#define SHOW_R 0
#define SHOW_L 1
#define DRAW 2

/* FSM LEDSHOW TIMERS milliseconds */
#define SECTOR_TIMER 800 
#define CUCHARA_TIMER 800
#define AZUCAR_TIMER 400
#define CAFE_TIMER 5000
#define LECHE_TIMER 6000
#define PITIDO_TIMER 600

/* TIME REQUIREMENTS */
#define PER_MON 100
#define DLINE_MON 100
#define PER_MAQ 100
#define DLINE_MAQ 100
#define STACKSIZE 1024

int timers[6] = {VASO_TIMER, CUCHARA_TIMER, AZUCAR_TIMER,
		 CAFE_TIMER, LECHE_TIMER, PITIDO_TIMER};

/* GLOBAL VARIABLES */
int moneda = 0;
int botonDevolver = 0;
int botonCafe = 0;
int fin = 0;
int cuenta = 0;
int timer = 0;
int endTest = 0;

/* TIMER FUNCTIONS */
static void timer_isr (union sigval arg) {
	timer = 1;
}

static void timer_run (int ms) {
	timer_t timerid;
	struct itimerspec value;
	struct sigevent se;
	se.sigev_notify = SIGEV_THREAD;
	se.sigev_value.sival_ptr = &timerid;
	se.sigev_notify_function = timer_isr;
	se.sigev_notify_attributes = NULL;
	value.it_value.tv_sec = ms / 1000;
	value.it_value.tv_nsec = (ms % 1000) * 1000000;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;
	timer_create(CLOCK_REALTIME, &se, &timerid);
	timer_settime(timerid, 0, &value, NULL);
}


/* MONEDERO INPUT FUNCTIONS */
static int checkCuenta(fsm_t *this) {
	if (moneda != 0) {
		cuenta += moneda;
		moneda = 0;
		return 1;
	} else {
		return 0;
	}
}
static int checkBotonDevolver(fsm_t *this) {
	if (botonDevolver != 0) {
		return 1;
	} else {
		return 0;
	}
}

static int checkBotonCafe(fsm_t *this) {
	if((botonCafe != 0) && (cuenta >= 50)) {
		cuenta -= 50;
		return 1;
	} 
	return 0;
}

static int checkFin(fsm_t *this) {
	return fin;
}

static int checkNoFin(fsm_t *this){
	if (fin == 0) {
		return 1;
	}	
	return 0;
}

/* MAQCAFE INPUT FUNCTIONS */

static int checkTimer(fsm_t *this) {
	
	return timer;

}

/* OUTPUT SIGNAL GENERATIONS */

static void doDevolver(fsm_t *this) {
  printf(" >> Cambio %d cent ", cuenta);
	cuenta = 0;
}

static void finOn(fsm_t *this) {
	fin = 1;
}

static void finOff(fsm_t *this) {
	fin = 0;
}

static void restartTimer(fsm_t *this){
/* El primer estado en el que se usa el timer es 1, 
   el array comienza en 0 */
	timer = 0;
	timer_run(timers[this->current_state-1]);
}

static void doNothing(fsm_t *this) {
	return;
}


/* TIME CALC FUNCTIONS USING TIMESPEC*/
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
void delay_until(struct timeval *next_activation){
	struct timeval now, delay;
	gettimeofday (&now, NULL);
	timeval_sub (&delay, next_activation, &now);
	select (0, NULL, NULL, NULL, &delay);
}


/* EXECUTION OF MAQUINA CAFE FSM */

static fsm_trans_t tt_monedero[] = {
	{REPOSO, checkCuenta, REPOSO , doNothing} , 
	{REPOSO,  checkBotonDevolver, REPOSO , doDevolver} ,  
	{REPOSO,  checkBotonCafe, SIRVIENDO , finOff} ,
	{SIRVIENDO, checkFin, REPOSO , doDevolver} ,
	{SIRVIENDO, checkCuenta, SIRVIENDO , doNothing} ,
	{-1, NULL, -1, NULL} ,
};

static fsm_trans_t tt_maqcafe[] = {  
	{IDLE, checkFin, IDLE , doNothing} ,  
	{IDLE, checkNoFin, VASO , restartTimer} ,  
	{VASO, checkTimer, CUCHARA , restartTimer} , 
	{CUCHARA, checkTimer, AZUCAR , restartTimer} , 
	{AZUCAR, checkTimer, CAFE , restartTimer} ,  
	{CAFE, checkTimer, LECHE , restartTimer} ,  
	{LECHE, checkTimer, PITIDO , restartTimer} , 
	{PITIDO, checkTimer, IDLE , finOn} ,
	{-1, NULL, -1, NULL} ,
};


/* MONEDERO FSM TASK DEFINTION */
static void task_monedero(struct event_handler_t* this) {
	struct timeval period = { 0, 100000 };
	fsm_run(monedero);
	timeval_add(&this->next_activation, &this->next_activation, &period);
}


/* MAQUINA CAFE FSM TASK DEFINTION */
static void task_maqcafe(struct event_handler_t* this) {
	struct timeval period = { 0, 100000 };
	fsm_run(maqcafe);
	timeval_add(&this->next_activation, &this->next_activation, &period);
}

int main(void){
	/* Variable initialization */
	moneda = 0;
	cuenta = 0;
	botonDevolver = 0;
	botonCafe = 0;
	fin = 1; /*Fin tiene que empezar en 1 para poner 0 cuando este sirviendo*/

	monedero = fsm_new(tt_monedero);
	maqcafe = fsm_new(tt_maqcafe);

	monedero->current_state = 0;
	maqcafe->current_state = 0;
	
	EventHandler mon,mcaf;
	reactor_init ();
		
	event_handler_init (&mon, 1, (eh_func_t) task_monedero);
	reactor_add_handler (&mon);
	
	event_handler_init (&mcaf, 2, (eh_func_t) task_maqcafe);
	reactor_add_handler (&mcaf);
 
   
	while (scanf("%d %d %d", &moneda, &botonCafe, &botonDevolver) == 3) {
		/* INTERFAZ */
		printf("\n>>> Mon: %d - Caf: %d - Dev: %d - Cue: %d - Fin: %d - Est1: %d - Est2: %d ", moneda, botonCafe, botonDevolver, cuenta, fin, monedero->current_state, maqcafe->current_state);

		reactor_handle_events ();

	}


	printf("\n**********  Fin de ejecucion  **********\n");
	return 0;
	
}
//EOF
