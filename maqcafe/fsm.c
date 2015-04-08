/*=================================================================
| Archivo: fsm.c
| Propósito: Implementación de una FSM
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

#include <stdlib.h>
#include <stdio.h>
#include "fsm.h"


/* Constructor function */
fsm_t*
fsm_new (fsm_trans_t *tt) {
	fsm_t *this = (fsm_t*) malloc (sizeof (fsm_t));
	fsm_init (this, tt);
	return this;
}

/* Initialization function */
void
fsm_init (fsm_t *this, fsm_trans_t *tt) {
	this->tt = tt;
}

/* FSM execution function */
void
fsm_run (fsm_t *this) {
  fsm_trans_t *t;
	t = this->tt;
	int estado = this->current_state;
	for (t = this->tt; t->last_state >= 0; ++t) {
		if ((this->current_state == t->last_state) && t->input(this)) {
			this->current_state = t->next_state;
			if (t->last_state != t->next_state) {
				printf(" ** Cambio de estado **");
			}
			if (t->output)
				t->output(this);
			break;
		}
	}
}
