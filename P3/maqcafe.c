/*=================================================================
| Archivo: maqcafe.c
| Propósito: Implementación de una máquina de café (TASKS XENOMAI)
| Documentación: -
|
| Revisiones:
| Fecha    Nombre          Revisión
| -------- --------------- ----------------------------------------
| 01-05-15 Dani y Diego    Created
| 03-05-15 Diego y Dani    Last release
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

/* MONEDERO STATES */
#define REPOSO 0
#define SIRVIENDO 1

/* MAQCAFE STATES */
#define IDLE 0
#define VASO 1
#define CUCHARA 2
#define AZUCAR 3
#define CAFE 4
#define LECHE 5
#define PITIDO 6

/* MAQCAFE TIMERS milliseconds */
#define VASO_TIMER 800 
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
pthread_mutex_t cerrojo_fin;
pthread_mutex_t cerrojo_endTest;

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
	int temp = 0;
	pthread_mutex_lock(&cerrojo_fin);
	temp = fin;
	pthread_mutex_unlock(&cerrojo_fin);
	return temp;
}

static int checkNoFin(fsm_t *this){
	int temp = 0;
	pthread_mutex_lock(&cerrojo_fin);
	temp = fin;
	pthread_mutex_unlock(&cerrojo_fin);
	if (temp == 0) {
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
	pthread_mutex_lock(&cerrojo_fin);
	fin = 1;
	pthread_mutex_unlock(&cerrojo_fin);

}

static void finOff(fsm_t *this) {
	pthread_mutex_lock(&cerrojo_fin);
	fin = 0;
	pthread_mutex_unlock(&cerrojo_fin);

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
static void *task_monedero(void *arg) {
	struct timespec next_activation;
	struct timespec *period = task_get_period(pthread_self());
	fsm_t *monedero = fsm_new(tt_monedero);
	monedero->current_state = REPOSO;
	clock_gettime(CLOCK_MONOTONIC, &next_activation);
	while(scanf("%d %d %d", &moneda, &botonCafe, &botonDevolver) == 3) {
		fsm_run(monedero);
		time_add(&next_activation, period, &next_activation);
		delay_until(&next_activation);
		pthread_mutex_lock(&cerrojo_fin);
		printf("\n>>> Mon: %d - Caf: %d - Dev: %d - Cue: %d - Fin: %d - Est1: %d ", moneda, botonCafe, botonDevolver, cuenta, fin, monedero->current_state);
                pthread_mutex_unlock(&cerrojo_fin);
	}
}


/* MAQUINA CAFE FSM TASK DEFINTION */
static void *task_maqcafe(void *arg) {
	struct timespec next_activation;
	struct timespec *period = task_get_period(pthread_self());
	fsm_t *maqcafe = fsm_new(tt_maqcafe);
	maqcafe->current_state = IDLE;
	clock_gettime(CLOCK_MONOTONIC, &next_activation);
	while(1) {
		fsm_run(maqcafe);
		time_add(&next_activation, period, &next_activation);
		delay_until(&next_activation);
		pthread_mutex_lock(&cerrojo_fin);
		printf("\n>>> Est2: %d - Fin: %d ", maqcafe->current_state, fin);
		pthread_mutex_unlock(&cerrojo_fin);
		pthread_mutex_lock(&cerrojo_endTest);
		if(endTest == 1) {
			break;
		}
		pthread_mutex_unlock(&cerrojo_endTest);
	}
}


int main(void){
	/* Variable initialization */
	moneda = 0;
	cuenta = 0;
	botonDevolver = 0;
	botonCafe = 0;
	fin = 1; /*Fin tiene que empezar en 1 para poner 0 cuando este sirviendo*/

	pthread_t pth_monedero = task_new("monedero", task_monedero,PER_MON,DLINE_MON,1,STACKSIZE);
	pthread_t pth_maqcafe = task_new("maqcafe", task_maqcafe,PER_MAQ,DLINE_MAQ,2,STACKSIZE);

	pthread_join(pth_monedero, NULL);
	/* Si se acaba el fichero de estimulos, acaba el proceso de
	 * monedero que es el que ejerce de interfaz con el
	 * usuario. De manera que se informa a la task de la maquina
	 * de cafe de que acabe su ejecucion en el bucle.*/
	pthread_mutex_lock(&cerrojo_endTest);
	endTest = 1;
	pthread_mutex_unlock(&cerrojo_endTest);
	pthread_join(pth_maqcafe, NULL);
	

	printf("**********  Fin de ejecucion  **********\n");
	return 0;
	
}
//EOF
