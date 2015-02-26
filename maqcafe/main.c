/* Método MAIN */
#include <stdio.h>
//#include <fsm.h>

/* ESTADOS MONEDERO */
#define REPOSO 0
#define SIRVIENDO 1

/* ESTADOS MAQCAFE */
#define IDLE 0
#define VASO 1
#define CUCHARA 2
#define AZUCAR 3
#define CAFE 4
#define LECHE 5
#define PITIDO 6

/* VARIABLES GLOBALES*/
int moneda;
int botonDevolver;
int botonCafe;
int fin;
int cuenta;

/* FUNCIONES DE ENTRADA MONEDERO*/

int checkMoneda() {
	if (moneda != 0){
		cuenta+=moneda;
		moneda = 0;
		return 1;
}
return 0;
}

int checkBotonDevolver(){
	if (botonDevolver != 0){
		return 1;
}
return 0;
}

int checkBotonCafe(){
	if(botonCafe != 0 && cuenta >= 50){
		cuenta-=50;
		return 1;
}
return 0;
}

int (*entMonedero[3])();

/* FUNCIONES DE SALIDA */

int doDevolver(){
	printf("\nDevolución: %d céntimos", cuenta);
	cuenta = 0;
	return 0;
}

int (*salMonedero[1])();
	

/* TABLA DE TRANSICCIÓN MAQCAFE */
/*int tt_maqcafe[][] = {  
 {IDLE, ,IDLE , } ,  
 {IDLE, ,VASO , } ,  
 {VASO, ,CUCHARA , } , 
 {CUCHARA, ,AZUCAR , } , 
 {AZUCAR, ,CAFE , } ,  
 {CAFE, ,LECHE , } ,  
 {LECHE, ,PITIDO , } , 
 {PITIDO, ,IDLE , }
};
*/

main(void){
/* INICIALIZACIÓN DE LA MÁQUINA */

moneda = 0;
cuenta = 0;
botonDevolver=0;;
botonCafe=0;
fin=0;

entMonedero[0]=checkMoneda;
entMonedero[1]=checkBotonDevolver;
entMonedero[2]=checkBotonCafe;

salMonedero[0] = doDevolver;
/* TABLA DE TRANSICIONES DEL MONEDERO */
int tt_monedero[4][4] = {
  {REPOSO, (*entMonedero[0])(),REPOSO , 0} , 
{REPOSO,  (*entMonedero[1])(),REPOSO , (*salMonedero[0])()} ,  
{REPOSO,  (*entMonedero[2])(),SIRVIENDO ,0 } ,
{SIRVIENDO, fin ,REPOSO , (*salMonedero[0])()} 
 };

int *p_tt_monedero = &tt_monedero;

//int *p_tt_maqcafe = &tt_maqcafe;



//fsm monedero = fsm_new(tt_monedero);
//int *p_monedero = &monedero;

//fsm maqcafe = fsm_new(tt_maqcafe);
//int *p_maqcafe = &maqcafe;

while(1){
//fsm_run(p_monedero);
//fsm_run(p_maqcafe);

//delay until next action

}
}
