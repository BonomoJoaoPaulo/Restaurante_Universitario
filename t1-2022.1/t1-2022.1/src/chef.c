#include <stdlib.h>

#include "chef.h"
#include "config.h"
#include "globals.h"

void *chef_run()
{
    /* Insira sua lógica aqui */
    queue_t *students_queue = globals_get_queue();

    while (students_queue->_length != 0)
    {
        chef_check_food();
    }
    
    pthread_exit(NULL);
}


void chef_put_food(int i, int j)
{   
    buffet_t *buffets = globals_get_buffets();
    for (int i = 0; i < 40; i++){
        //sem_post(buffets[i]._meal[j])
    }
    
}
void chef_check_food()
{
    buffet_t *buffets = globals_get_buffets();
    int number_of_buffets = globals_get_number_of_buffets();
    for (int line = 0; line < number_of_buffets; ++line)
        for (int col = 0; col < 5; ++col){
            //proteger com mutex
            if(get_sem_valeu(buffets[line]._meal[col]) == 0) {
                chef_put_food(line, col);
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