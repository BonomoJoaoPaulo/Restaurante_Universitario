#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "worker_gate.h"
#include "globals.h"
#include "config.h"

/* Mutex para impedir que estudantes diferentes queiram ser inseridos na mesma primeira posicao de um buffet. */
pthread_mutex_t buffet_first_position_mutex;

/* Mutex para nao remover mais de um estudante por vez da fila de fora. */
pthread_mutex_t remove_student_mutex;

/* Semafaro catraca para o worker gate so procurar posicoes livres nas filas do buffet quando elas existirem (evita espera ocupada). */
sem_t ratchet;

/* Usamos essa funcao para verificar se a fila de fora esta vazia ou nao. */
int worker_gate_look_queue()
{
    queue_t *students_queue = globals_get_queue();
    if (students_queue->_length == 0)
    {
        /* Printf para mostrar que nao ha mais ninguem na fila de fora esperando. */
        printf("| ----------------------------- | FILA DE FORA VAZIA | ----------------------------- |\n");
        return 1;
    }

    return 0;
}

/* Funcao para remover o primeiro estudante na fila de fora. */
void worker_gate_remove_student()
{
    queue_t *students_queue = globals_get_queue();
    msleep(500);
    /* Execucao somente caso a fila de fora nao esteja vazia. */
    if (students_queue->_length != 0)
    {
        pthread_mutex_lock(&remove_student_mutex);
        queue_remove(students_queue);
        /* Printf para auxilar na visualizacao do funcionamento da funcao. */
        printf("Tamanho atual da fila de fora: %d\n", students_queue->_length);

        pthread_mutex_unlock(&remove_student_mutex);
    }
}

void worker_gate_look_buffet()
{
    /* Nao usamos essa funcao em nossa implementacao. */
}

void *worker_gate_run(void *arg)
{
    /* Variaveis que recebem, respectivamente, se todos os estudantes ja entraram e o numero total de estudantes na fila de fora. */
    int all_students_entered;
    int number_students;

    number_students = *((int *)arg);
    all_students_entered = number_students > 0 ? FALSE : TRUE;

    pthread_mutex_init(&buffet_first_position_mutex, NULL);
    pthread_mutex_init(&remove_student_mutex, NULL);

    /* Variavel que recebe o numero total de buffets. */
    int number_of_buffets = globals_get_number_of_buffets();

    /* inicializacao do semaforo ratchet com o dobro do numero de buffets porque temos 2 filas (esquerda e direita). */
    sem_init(&ratchet, 0, number_of_buffets * 2);
    // int sval2;
    // sem_getvalue(&ratchet, &sval2);

    /* Enquanto todos os estudantes nao tiverem entrado... */
    while (all_students_entered == FALSE)
    {
        // worker_gate_look_buffet();
        /* Chamada da funcao que remove o primeiro estudante da fila de fora. */
        worker_gate_remove_student();

        /* Chamada da funcao worker_gate_look_queue em uma variavel para usarmos no if seguinte. */
        int students_on_queue = worker_gate_look_queue();
        // int number_of_students = globals_get_students();
        /* Se nao houver mais ninguem do lado de fora, para de remover na fila de fora. */
        if (students_on_queue != 0)
        {
            break;
        }
    }

    pthread_mutex_destroy(&buffet_first_position_mutex);
    pthread_mutex_destroy(&remove_student_mutex);
    pthread_exit(NULL);
}

void worker_gate_init(worker_gate_t *self)
{
    int number_students = globals_get_students();
    pthread_create(&self->thread, NULL, worker_gate_run, &number_students);
}

void worker_gate_finalize(worker_gate_t *self)
{
    sem_destroy(&ratchet);
    pthread_join(self->thread, NULL);
    free(self);
}

void worker_gate_insert_queue_buffet(student_t *student)
{
    // printf("chamou a funcao");
    // queue_t *students_queue = globals_get_queue();
    // queue_insert(students_queue, student);
    /* Pegamos todos os buffets e o numero total de buffets. */
    buffet_t *buffets = globals_get_buffets();
    int number_of_buffets = globals_get_number_of_buffets();
    // int sval2;
    // sem_getvalue(&ratchet, &sval2);
    // printf("Semaphore value: %d\n", sval2);
    /*  Wait no semafaro que evita espera ocupada. */
    sem_wait(&ratchet);

    /* Lock no mutex da primeira posicao do buffet. */
    pthread_mutex_lock(&buffet_first_position_mutex);

    /* Execucao dentro de for para cobrir todos os buffets. */
    for (int i = 0; i < number_of_buffets; i++)
    {
        int number_of_students = globals_get_students();
        /* Verifica se a primeira posicao da fila da esquerda esta vaga. */
        if (buffets[i].queue_left[0] == 0)
        {
            student->_id_buffet = buffets[i]._id;
            student->left_or_right = 'L';
            /* Insercao do estudante no buffet. */
            buffet_queue_insert(buffets, student);
            /* Unlock no mutex da primeira posicao do buffet. */
            pthread_mutex_unlock(&buffet_first_position_mutex);
            // printf("ugauga %d\n", number_of_students);
            break;
        }
        /* Verifica se a primeira posicao da fila da direita esta vaga. */
        if (buffets[i].queue_right[0] == 0)
        {
            student->_id_buffet = buffets[i]._id;
            student->left_or_right = 'R';
            /* Insercao do estudante no buffet. */
            buffet_queue_insert(buffets, student);
            /* Unlock no mutex da primeira posicao do buffet. */
            pthread_mutex_unlock(&buffet_first_position_mutex);
            // printf("ugauga %d\n", number_of_students);
            break;
        }
    }
}