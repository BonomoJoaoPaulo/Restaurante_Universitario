#include <stdlib.h>
#include "buffet.h"
#include "config.h"
#include "worker_gate.h"
#include "chef.h"
#include "globals.h"

sem_t ratchet;
int cont= 0;

void *buffet_run(void *arg)
{   
    int all_students_entered = FALSE;
    buffet_t *self = (buffet_t*) arg;

    
    /*  O buffet funciona enquanto houver alunos na fila externa. */
    while (all_students_entered == FALSE)
    {
        /* Cada buffet possui: Arroz, Feijão, Acompanhamento, Proteína e Salada */
        /* Máximo de porções por bacia (40 unidades). */
        _log_buffet(self);

        msleep(500); /* Pode retirar este sleep quando implementar a solução! */
    }

    pthread_exit(NULL);
}

void buffet_init(buffet_t *self, int number_of_buffets)
{
    globals_set_number_of_buffets(number_of_buffets);
    int i = 0, j = 0;
    for (i = 0; i < number_of_buffets; i++)
    {
        /*A fila possui um ID*/
        self[i]._id = i;

        /* Inicia com 40 unidades de comida em cada bacia */
        for(j = 0; j < 5; j++)
        {
            self[i]._meal[j] = 40;

        /* Inicia um mutex para cada posição na fila do buffet (cada bacia) */
            pthread_mutex_init(&self[i].mutex_queue_left[j], NULL);
            pthread_mutex_init(&self[i].mutex_queue_right[j], NULL);
        }
            

        for(j= 0; j< 5; j++){
             /* A fila esquerda do buffet possui cinco posições. */
            self[i].queue_left[j] = 0;
            /* A fila direita do buffet possui cinco posições. */
            self[i].queue_right[j] = 0;
        }

        pthread_create(&self[i].thread, NULL, buffet_run, &self[i]);
    }
}


int buffet_queue_insert(buffet_t *self, student_t *student)
{
    /* Se o estudante vai para a fila esquerda */
    if (student->left_or_right == 'L') 
    {
        /* Verifica se a primeira posição (bacia) do buffet está vaga */
        if (!self[student->_id_buffet].queue_left[0])
        {   
            /* Lock no mutex que bloqueia a primeira bacia */
            pthread_mutex_lock(&self[student->_id_buffet].mutex_queue_left[0]);
            self[student->_id_buffet].queue_left[0] = student->_id;
            student->_buffet_position = 0;
            //printf("studentL %d", student->_id);
            return 1;
        }
    }
    /* Se o estudante vai para a fila direita */
    else
    { 
        /* Verifica se a primeira posição (bacia) do buffet está vaga */
        if (!self[student->_id_buffet].queue_right[0])
        {
            /* Lock no mutex que bloqueia a primeira bacia */
            pthread_mutex_lock(&self[student->_id_buffet].mutex_queue_right[0]);
            self[student->_id_buffet].queue_right[0] = student->_id;
            student->_buffet_position = 0;
            //printf("studentR %d", student->_id);
            return 1;
        }
    }
    return 0;
}


void buffet_next_step(buffet_t *self, student_t *student)
{
    /* Se estudante ainda precisa se servir de mais alguma coisa... */
    if (student->_buffet_position < 4)
    { 
        if (student->left_or_right == 'L') /* Está na fila esquerda? */
        {   /* Caminha para a posição seguinte da fila do buffet.*/
            int position = student->_buffet_position;
            /* Lock no mutex da próxima posição (bacia) da fila do buffet*/
            pthread_mutex_lock(&self->mutex_queue_left[position + 1]);
            self->queue_left[position] = 0;
            self->queue_left[position + 1] = student->_id;
            if (student->_buffet_position == 0){
                /* Sempre que a primeira posição da fila de qualquer buffet fica vaga damos um post(incremento) no semafaro da catraca */
                //printf("student %d", student->_id);
                sem_post(&ratchet);
            }
            student->_buffet_position ++;

            /* Unlock no mutex da posição (bacia) ATUAL do estudante */
            pthread_mutex_unlock(&self->mutex_queue_left[position]);
            //printf("student %d deu unlock L : %d \n", student->_id, position);

        } else /* Está na fila direita? */
        {   /* Caminha para a posição seguinte da fila do buffet.*/
            int position = student->_buffet_position;
            /* Lock no mutex da próxima posição (bacia) da fila do buffet*/
            pthread_mutex_lock(&self->mutex_queue_right[position + 1]);
            self->queue_left[position] = 0;
            self->queue_left[position + 1] = student->_id;
            if (student->_buffet_position == 0){
                /* Sempre que a primeira posição da fila de qualquer buffet fica vaga damos um post(incremento) no semafaro da catraca */
                sem_post(&ratchet);
            }
            student->_buffet_position ++;
            /* Unlock no mutex da posição (bacia) ATUAL do estudante */
            pthread_mutex_unlock(&self->mutex_queue_right[position]);
            //printf("student %d deu unlock R : %d \n", student->_id, position);
        }
    } else /* Caso o estudante esteja na última bacia */
    {
        if (student->left_or_right == 'L') {
            /* Unlock no mutex da bacia que o estudante está (a última) */
            self->queue_left[4] = 0;
            pthread_mutex_unlock(&self->mutex_queue_left[4]);
        } else {
            /* Unlock no mutex da bacia que o estudante está (a última) */
            self->queue_right[4] = 0;
            pthread_mutex_unlock(&self->mutex_queue_right[4]);
        }

        /* Define a posição do estudante no buffet como -1 (fora) */
        student->_buffet_position = -1;
        printf("student %d saiu: %d \n", student->_id, student->_buffet_position);
        cont++;
        printf("%d", cont);
    }
}

/* --------------------------------------------------------- */
/* ATENÇÃO: Não será necessário modificar as funções abaixo! */
/* --------------------------------------------------------- */

void buffet_finalize(buffet_t *self, int number_of_buffets)
{
    /* Espera as threads se encerrarem...*/
    for (int i = 0; i < number_of_buffets; i++)
    {
        pthread_join(self[i].thread, NULL);
    }
    
    /*Libera a memória.*/
    free(self);
}


void _log_buffet(buffet_t *self)
{
    /* Prints do buffet */
    int *ids_left = self->queue_left; 
    int *ids_right = self->queue_right; 

    printf("\n\n\u250F\u2501 Queue left: [ %d %d %d %d %d ]\n", ids_left[0],ids_left[1],ids_left[2],ids_left[3],ids_left[4]);
    fflush(stdout);
    printf("\u2523\u2501 BUFFET %d = [RICE: %d/40 BEANS:%d/40 PLUS:%d/40 PROTEIN:%d/40 SALAD:%d/40]\n",
           self->_id, self->_meal[0], self->_meal[1], self->_meal[2], self->_meal[3], self->_meal[4]);
    fflush(stdout);
    printf("\u2517\u2501 Queue right: [ %d %d %d %d %d ]\n", ids_right[0],ids_right[1],ids_right[2],ids_right[3],ids_right[4]);
    fflush(stdout);
}