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
/* */
int moneda = 0;
int botonDevolver = 0;
int botonCafe = 0;
int fin = 0;
int cuenta = 0;
int timer = 0;


/* MONEDERO INPUT FUNCTIONS */
int checkCuenta(fsm_t *this) {
	if (moneda != 0) {
		cuenta += moneda;
		moneda = 0;
		return 1;
	} else {
		return 0;
	}
}

int checkBotonDevolver(fsm_t *this) {
	if (botonDevolver != 0) {
		return 1;
	} else {
		return 0;
	}
}

int checkBotonCafe(fsm_t *this) {
	if((botonCafe != 0) && (cuenta >= 50)) {
		cuenta -= 50;
		return 1;
	} 
	return 0;
}

int checkFin(fsm_t *this) {
	if (fin == 1) {
		return 1;
	}
	return 0;
}

int checkNoFin(fsm_t *this){
	if (fin == 0) {
		return 1;
	}	
	return 0;
}

/* MAQCAFE INPUT FUNCTIONS */


int checkTimer(fsm_t *this) {
	if((timer-1) <= 0) {
		return 1;
	}
	else {
		timer--;
		return 0;
	}
}

/* INPUT FUNCTIONS COLECTION */
int (*inputsMonedero[4])(fsm_t *this) = {checkCuenta, checkBotonDevolver, checkBotonCafe, checkFin};
int (*inputsMaqCafe[3])(fsm_t *this) = {checkFin, checkNoFin, checkTimer};



/* OUTPUT SIGNAL GENERATIONS */

void doDevolver(fsm_t *this) {
	printf(" ++ Cambio %d cént ++", cuenta);
	cuenta = 0;
}

void finOn(fsm_t *this) {
	fin = 1;
}

void finOff(fsm_t *this) {
	fin = 0;
}

void restartTimer(fsm_t *this){

/* En la funcion run primero se cambia de estado y luego se llama a las funciones output
 asi que hay que tener cuidado al definir esto
 por eso se pone estate -1*/

timer = timers[this->current_state-1];
}

void doNothing(fsm_t *this) {
	return;
}

/* OUTPUT FUNCTIONS COLECTION */
void (*outputsMonedero[3])() = {doDevolver, finOff, doNothing};
void (*outputsMaqCafe[3])() = {finOn, restartTimer, doNothing};


/* EXECUTION OF MAQUINA CAFE FSM */

int
main(void){
  /* INIT VARIABLES */
	
	moneda = 0;
	cuenta = 0;
	botonDevolver = 0;
	botonCafe = 0;
	fin = 1; /*Fin tiene que empezar en 1 para poner 0 cuando esté sirviendo*/
	
	fsm_trans_t tt_monedero[] = {
		{REPOSO, inputsMonedero[0], REPOSO , outputsMonedero[2]} , 
		{REPOSO,  inputsMonedero[1], REPOSO , outputsMonedero[0]} ,  
		{REPOSO,  inputsMonedero[2], SIRVIENDO , outputsMonedero[1] } ,
		{SIRVIENDO, inputsMonedero[3], REPOSO , outputsMonedero[0]} ,
		{SIRVIENDO, inputsMonedero[0], SIRVIENDO , outputsMonedero[2]} ,
		{-1, NULL, -1, NULL} ,
	};

	fsm_trans_t tt_maqcafe[] = {  
		{IDLE, inputsMaqCafe[0], IDLE , outputsMaqCafe[2]} ,  
		{IDLE, inputsMaqCafe[1], VASO , outputsMaqCafe[1]} ,  
		{VASO, inputsMaqCafe[2], CUCHARA , outputsMaqCafe[1]} , 
		{CUCHARA, inputsMaqCafe[2], AZUCAR , outputsMaqCafe[1]} , 
		{AZUCAR, inputsMaqCafe[2], CAFE , outputsMaqCafe[1]} ,  
		{CAFE, inputsMaqCafe[2], LECHE , outputsMaqCafe[1]} ,  
		{LECHE, inputsMaqCafe[2], PITIDO , outputsMaqCafe[1]} , 
		{PITIDO, inputsMaqCafe[2], IDLE , outputsMaqCafe[0]} ,
		{-1, NULL, -1, NULL} ,
	};	
	
	fsm_t *monedero = fsm_new(tt_monedero);
	fsm_t *maqcafe = fsm_new(tt_maqcafe);

	monedero->current_state = 0;
	int i = 0;
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

		fsm_run(monedero);
		fsm_run(maqcafe);
		
		
	}
	printf("\n\nFin de ejecución");
	return 0;
}
//EOF
