#include <stdlib.h>

#include "chef.h"
#include "config.h"
#include "globals.h"

void *chef_run()
{
    queue_t *students_queue = globals_get_queue();
    buffet_t *buffets = globals_get_buffets();

/* Enquanto tiver estudantes na fila, o chef vai checkar as bacias de comida */
    while (students_queue->_length != 0)
    {
        chef_check_food(&buffets);
    }
    
    pthread_exit(NULL);
}


void chef_put_food(buffet_t *buffets, int i, int j)
/* Inicia a bacia com 40 porcoes */
{
    buffets[i]._meal[j] = 40;
}

void chef_check_food(buffet_t *buffets)
/* Se a bacia estiver zerada de porcoes, a funcao chef_put_food eh chamada */
/* line eh o buffet e col eh a bacia daquele buffet que estah sendo verificada */
{
    for (int line = 0; line < config.buffets; ++line)
        for (int col = 0; col < 5; ++col){
            if(buffets[line]._meal[col] == 0) {
                chef_put_food(&buffets, line, col);
            }
        }
}

/* --------------------------------------------------------- */
/* ATENÇÃO: Não será necessário modificar as funções abaixo! */
/* --------------------------------------------------------- */

void chef_init(chef_t *self)
{
    pthread_create(&self->thread, NULL, chef_run, NULL);
}

void chef_finalize(chef_t *self)
{
    pthread_join(self->thread, NULL);
    free(self);
}
