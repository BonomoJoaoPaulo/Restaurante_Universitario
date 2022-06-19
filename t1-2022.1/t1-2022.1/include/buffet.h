#ifndef __buffet_H__
#define __buffet_H__

#include <pthread.h>
#include "queue.h"

typedef struct buffet
{
    int _id;
    int _meal[5];

    int queue_left[5];
    int queue_right[5];

    /* Mutexes para cada uma das 5 posicoes (bacias) na fila do buffet. */
    pthread_mutex_t mutex_queue_left[5];
    pthread_mutex_t mutex_queue_right[5];

    /* Criacao de um mutex para cada bacia (para somente um estudante acessar determinada bacia por vez). */
    pthread_mutex_t mutex_serving[5];

    /* Thread do buffet   */
    pthread_t thread;
} buffet_t;

/**
 * @brief Thread do buffet.
 *
 * @return void*
 */
extern void *buffet_run();

/**
 * @brief Inicia o buffet
 *
 */
extern void buffet_init(buffet_t *self, int number_of_buffets);

/**
 * @brief Encerra o buffet
 *
 */
extern void buffet_finalize(buffet_t *self, int number_of_buffets);

/**
 * @brief Vai para a próxima posição da fila do buffet
 *
 * @param self
 * @param student
 */
extern void buffet_next_step(buffet_t *self, student_t *student);

/**
 * @brief Retorna TRUE quando inseriu um estudante com sucesso no fim da fila do buffet.
 *        Retorna FALSE, caso contrário.
 *
 */
extern int buffet_queue_insert(buffet_t *self, student_t *student);

/**
 * @brief Referências para funções privadas ao arquivo.
 *
 * @param self
 */
void _log_buffet(buffet_t *self);

#endif