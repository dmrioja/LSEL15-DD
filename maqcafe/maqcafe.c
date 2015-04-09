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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

/* GLOBAL VARIABLES */
int moneda = 0;
int botonDevolver = 0;
int botonCafe = 0;
int fin = 0;
int cuenta = 0;
int timer = 0;
struct timespec tim;


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
		/* nanosleep(&tim, NULL);*/
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

/* long diff(struct timespec *start, struct timespec *end) { */
/*         struct timespec *temp; */
/*         if ((end.tv_nsec - start.tv_nsec) < 0) { */
/* 		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec; */
/* 	}  */
/* 	else { */
/* 		temp.tv_nsec = end.tv_nsec - start.tv_nsec; */
/*         } */
/*         return temp.tv_nsec; */
/* } */

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

	struct timespec *t1, *t2, *t3;
	long task1, task2;
	long maxTask1, maxTask2;
	task1 = task2 = maxTask1 = maxTask2 = 0;
	/* tim.tv_sec = 0; */
	/* tim.tv_nsec = 10000000;  */
	/* 10 ms */

	/* INIT VARIABLES */
	
	moneda = 0;
	cuenta = 0;
	botonDevolver = 0;
	botonCafe = 0;
	fin = 1; /*Fin tiene que empezar en 1 para poner 0 cuando esté sirviendo*/
	
	fsm_t *monedero = fsm_new(tt_monedero);
	fsm_t *maqcafe = fsm_new(tt_maqcafe);

	monedero->current_state = 0;
	
	while (scanf("%d %d %d", &moneda, &botonCafe, &botonDevolver) == 3) {
		/* INTERFAZ */
		printf("\n>>> Mon: %d ",moneda);
		printf("Caf: %d ", botonCafe);
		printf("Dev: %d ", botonDevolver);
		printf("Cue: %d ",cuenta);
		printf("Fin: %d",fin);		
		printf("Est1: %d ",monedero->current_state);
		printf("Est2: %d ",maqcafe->current_state);
		
		/* EJECUCION DE LAS FSM */
		/* clock_gettime(CLOCK_MONOTONIC, t1); */
		fsm_run(monedero);
		/* clock_gettime(CLOCK_MONOTONIC, t2); */
		fsm_run(maqcafe);
		/* clock_gettime(CLOCK_MONOTONIC, t3); */

		/* task1 = diff(t1,t2); */
		/* task2 = diff(t2,t3); */

		if(task1 > maxTask1){
			maxTask1 = task1;
		}

		if(task2 > maxTask2){
			maxTask2 = task2;
		}
		
	}
	printf("\n\nFin de ejecución");
	printf("\nMaxTime Monedero: %ld", maxTask1);
	printf("\nMaxTime MaqCafe: %ld", maxTask2);

	return 0;

}
//EOF
