/*=================================================================
| Archivo: maqcafe.c
| Propósito: Implementación de una máquina de café
| Documentación: -
|
| Revisiones:
| Fecha    Nombre          Revisión
| -------- --------------- ----------------------------------------
| 26-02-15 Dani y Diego    Created
| 03-03-15 Diego           Add lectura fichero
| 05-03-15 Diego           Modified functions
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

/* Definir aquí flags para el modo DEBUG */ 

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

/* MAQCAFE TIMERS */
#define VASO_TIMER 3
#define CUCHARA_TIMER 4
#define AZUCAR_TIMER 5
#define CAFE_TIMER 6
#define LECHE_TIMER 7
#define PITIDO_TIMER 8

int timers[6] = {VASO_TIMER, CUCHARA_TIMER, AZUCAR_TIMER,
		 CAFE_TIMER, LECHE_TIMER, PITIDO_TIMER};

/* CONSTANTS */
#define Nphases 1

/* GLOBAL VARIABLES */
int moneda = 0;
int botonDevolver = 0;
int botonCafe = 0;
int fin = 0;
int cuenta = 0;
int timer = 0;


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
	if (fin == 1) {
		return 1;
	}
	return 0;
}

static int checkNoFin(fsm_t *this){
	if (fin == 0) {
		return 1;
	}	
	return 0;
}

/* MAQCAFE INPUT FUNCTIONS */

static int checkTimer(fsm_t *this) {
	if((timer-1) <= 0) {
		return 1;
	}
	else {
		timer--;
		return 0;
	}
}

/* OUTPUT SIGNAL GENERATIONS */

static void doDevolver(fsm_t *this) {
	printf(" ++ Cambio %d cént ++", cuenta);
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
	
	timer = timers[this->current_state-1];
}

static void doNothing(fsm_t *this) {
	return;
}


/* TIME CALC FUNCTIONS */

void diff(struct timespec *start, struct timespec *end, struct timespec *sub) {
	if ((end->tv_nsec - start->tv_nsec) < 0) {
		sub->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
	}
	else {
		sub->tv_nsec = end->tv_nsec - start->tv_nsec;
        }
}

/* Delay until next_activation */
void delay_until (struct timespec *next_activation){
	struct timespec now, delay;
	clock_gettime(CLOCK_MONOTONIC,&now);
	delay.tv_nsec = next_activation->tv_nsec - now.tv_nsec;
	/* diff(&now, next_activation, &delay); */
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

int main(void){

	/* TIME VARIABLES */
	struct timespec period = { 0, 100 * 1000000 };
	struct timespec next_activation = {0,0};
	struct timespec t1,t2,t3,task1,task2 = {0,0};
	long maxTask1 = 0;
	long maxTask2 = 0;
	
	/* INIT VARIABLES */
	
	moneda = 0;
	cuenta = 0;
	botonDevolver = 0;
	botonCafe = 0;
	fin = 1; /*Fin tiene que empezar en 1 para poner 0 cuando este sirviendo*/
	
	fsm_t *monedero = fsm_new(tt_monedero);
	fsm_t *maqcafe = fsm_new(tt_maqcafe);

	monedero->current_state = 0;
	maqcafe->current_state = 0;

	clock_gettime(CLOCK_MONOTONIC,&next_activation);
	next_activation.tv_nsec += period.tv_nsec;
	int phase = 0;

	while (scanf("%d %d %d", &moneda, &botonCafe, &botonDevolver) == 3) {

		/* INTERFAZ */
		printf("\n>>> Mon: %d ",moneda);
		printf("Caf: %d ", botonCafe);
		printf("Dev: %d ", botonDevolver);
		printf("Cue: %d ",cuenta);
		printf("Fin: %d",fin);
		printf("Est1: %d ",monedero->current_state);
		printf("Est2: %d ",maqcafe->current_state);
		printf("antes");
		
		
		/* EJECUTIVO CICLICO DE LAS FSM */
		delay_until(&next_activation);
		next_activation.tv_nsec += period.tv_nsec;
		switch(phase) {
		case 0:
			
			clock_gettime(CLOCK_MONOTONIC, &t1);
			fsm_run(monedero);
			clock_gettime(CLOCK_MONOTONIC, &t2);
			fsm_run(maqcafe);
			clock_gettime(CLOCK_MONOTONIC, &t3);
			
			diff(&t1,&t2,&task1);
			diff(&t2,&t3,&task2);
			
			if(task1.tv_nsec > maxTask1){
				maxTask1 = task1.tv_nsec;
			}
			
			if(task2.tv_nsec > maxTask2){
				maxTask2 = task2.tv_nsec;
			}
		}
		phase = (phase + 1) % Nphases;
	}
	
	printf("\n\nFin de ejecución");
	printf("\nMaxTime Monedero: %ld", maxTask1);
	printf("\nMaxTime MaqCafe: %ld", maxTask2);
	
	return 0;
	
}
//EOF
