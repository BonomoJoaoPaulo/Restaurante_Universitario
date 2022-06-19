#include <stdlib.h>
#include "buffet.h"
#include "config.h"
#include "worker_gate.h"
#include "chef.h"
#include "globals.h"

/* Esse semaforo que funciona como a catraca do RU. */
sem_t ratchet;
/*Variavel que conta quantos estudantes ja sairam do buffet. */
int count;

void *buffet_run(void *arg)
{
    /* Variavel que mostra se todos os students ja entraram no RU ou se ainda ha alguem de fora. */
    int all_students_entered = FALSE;
    buffet_t *self = (buffet_t *)arg;

    /*  O buffet funciona enquanto houver alunos na fila externa. */
    while (all_students_entered == FALSE)
    {
        /* Se o numero de students for 0, 'breaka' o funcionamento do buffet. */
        if (globals_get_students() == 0)
        {
            break;
        }
        _log_buffet(self);
        msleep(100);
    }
    /* Como sao 5 mutex (1 para cada posicao no buffet), eh necessario fazer o destroy dentro de um for. */
    for (int i = 0; i < 5; i++)
    {
        pthread_mutex_destroy(&self->mutex_queue_left[i]);
        pthread_mutex_destroy(&self->mutex_queue_right[i]);
    }
    pthread_exit(NULL);
}

void buffet_init(buffet_t *self, int number_of_buffets)
{
    /* 'Seta' o numero de buffets como o parametro recebido. */
    globals_set_number_of_buffets(number_of_buffets);

    /* i representa o id do buffet e j representa a posicao na fila do buffet (bacia). */
    int i = 0, j = 0;

    for (i = 0; i < number_of_buffets; i++)
    {
        /* A fila possui um ID. */
        self[i]._id = i;

        /* Inicia com 40 unidades de comida em cada bacia. */
        for (j = 0; j < 5; j++)
        {
            self[i]._meal[j] = 40;

            /* Inicia um mutex para cada posicao na fila do buffet (cada bacia). */
            pthread_mutex_init(&self[i].mutex_queue_left[j], NULL);
            pthread_mutex_init(&self[i].mutex_queue_right[j], NULL);
        }

        for (j = 0; j < 5; j++)
        {
            /* A fila esquerda do buffet possui cinco posicoes. */
            self[i].queue_left[j] = 0;
            /* A fila direita do buffet possui cinco posicoes. */
            self[i].queue_right[j] = 0;
        }

        pthread_create(&self[i].thread, NULL, buffet_run, &self[i]);
    }
}

int buffet_queue_insert(buffet_t *self, student_t *student)
{
    /* Se o estudante vai para a fila esquerda... */
    if (student->left_or_right == 'L')
    {
        /* Verifica se a primeira posicao (bacia) do buffet esta vaga. */
        if (!self[student->_id_buffet].queue_left[0])
        {
            /* Lock no mutex que bloqueia a primeira bacia. */
            pthread_mutex_lock(&self[student->_id_buffet].mutex_queue_left[0]);
            self[student->_id_buffet].queue_left[0] = student->_id;
            student->_buffet_position = 0;
            return 1;
        }
    }
    /* Se o estudante vai para a fila direita... */
    else
    {
        /* Verifica se a primeira posicao (bacia) do buffet esta vaga. */
        if (!self[student->_id_buffet].queue_right[0])
        {
            /* Lock no mutex que bloqueia a primeira bacia. */
            pthread_mutex_lock(&self[student->_id_buffet].mutex_queue_right[0]);
            self[student->_id_buffet].queue_right[0] = student->_id;
            student->_buffet_position = 0;
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
        /* Se esta na fila esquerda... */
        if (student->left_or_right == 'L')
        {
            /* Variavel que recebe a posicao atual do estudante. */
            int position = student->_buffet_position;
            /* Lock no mutex da proxima posicao (bacia) da fila do buffet. */
            pthread_mutex_lock(&self[student->_id_buffet].mutex_queue_left[position + 1]);
            /* Caminha para a posição seguinte da fila do buffet. */
            self[student->_id_buffet].queue_left[position] = 0;
            self[student->_id_buffet].queue_left[position + 1] = student->_id;

            /* Se ele estiver na primeira posicao da fila do buffet... */
            if (student->_buffet_position == 0)
            {
                /* Sempre que a primeira posicao da fila de qualquer buffet fica vaga damos um post(incremento) no semafaro da catraca. */
                sem_post(&ratchet);
            }

            student->_buffet_position++;

            /* Unlock no mutex da posição (bacia) ATUAL do estudante. */
            pthread_mutex_unlock(&self[student->_id_buffet].mutex_queue_left[position]);
        }
        /* Se esta na fila da direita...*/
        /* Mesma logica da fila da esquerda. */
        else
        {
            int position = student->_buffet_position;

            pthread_mutex_lock(&self[student->_id_buffet].mutex_queue_right[position + 1]);

            self[student->_id_buffet].queue_right[position] = 0;
            self[student->_id_buffet].queue_right[position + 1] = student->_id;

            if (student->_buffet_position == 0)
            {
                sem_post(&ratchet);
            }

            student->_buffet_position++;

            pthread_mutex_unlock(&self[student->_id_buffet].mutex_queue_right[position]);
        }
    }
    /* Caso o estudante esteja na última bacia... */
    else
    {
        if (student->left_or_right == 'L')
        {
            /* Unlock no mutex da bacia que o estudante esta (a ultima). */
            self[student->_id_buffet].queue_left[4] = 0;
            pthread_mutex_unlock(&self[student->_id_buffet].mutex_queue_left[4]);
        }
        else
        {
            /* Unlock no mutex da bacia que o estudante esta (a ultima). */
            self[student->_id_buffet].queue_right[4] = 0;
            pthread_mutex_unlock(&self[student->_id_buffet].mutex_queue_right[4]);
        }

        /* Define a posicao do estudante no buffet como -1 (fora). */
        student->_buffet_position = -1;
        printf("O student %d saiu do buffet. \n", student->_id);
        count++;
        printf("Students ja servidos: %d\n", count);
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

    printf("\n\n\u250F\u2501 Queue left: [ %d %d %d %d %d ]\n", ids_left[0], ids_left[1], ids_left[2], ids_left[3], ids_left[4]);
    fflush(stdout);
    printf("\u2523\u2501 BUFFET %d = [RICE: %d/40 BEANS:%d/40 PLUS:%d/40 PROTEIN:%d/40 SALAD:%d/40]\n",
           self->_id, self->_meal[0], self->_meal[1], self->_meal[2], self->_meal[3], self->_meal[4]);
    fflush(stdout);
    printf("\u2517\u2501 Queue right: [ %d %d %d %d %d ]\n", ids_right[0], ids_right[1], ids_right[2], ids_right[3], ids_right[4]);
    fflush(stdout);
}