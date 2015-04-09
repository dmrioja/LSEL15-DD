/*=================================================================
| Archivo: fsm.h
| Propósito: Descripción de una máquina de estados. Interfaz.
| Documentación: -
|
| Revisiones:
| Fecha    Nombre          Revisión
| -------- --------------- ----------------------------------------
| 02-03-15 Dani y Diego    Created
| 05-03-15 Diego           Modified
| 10-03-15 Diego           Modified
|
===================================================================*/

#ifndef FSM_H
#define FSM_H

typedef struct fsm_t fsm_t;

typedef int (*fsm_input_func_t) (fsm_t*);
typedef void (*fsm_output_func_t) (fsm_t*);

/* Transition table structure */
typedef struct fsm_trans_t {
	int last_state;
	fsm_input_func_t input;
	int next_state;
	fsm_output_func_t output;
} fsm_trans_t;

/* FSM definition struct */
struct fsm_t {
	int current_state;
	fsm_trans_t *tt;
};

/* FSM functions*/
fsm_t *fsm_new (fsm_trans_t *tt);
void fsm_init (fsm_t *this, fsm_trans_t *tt);
void fsm_run (fsm_t *this);

#endif

