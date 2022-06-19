#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "worker_gate.h"
#include "globals.h"
#include "config.h"

pthread_mutex_t buffet_first_position_mutex; // mutex para impedir que estudantes diferentes queiram ser inseridos na mesma primeira posicao de um buffet
pthread_mutex_t remove_student_mutex;
sem_t ratchet; // semafaro catraca para o worker gate so procurar posicoes livres nas filas do buffet quando elas existirem(evita espera ocupada)

int worker_gate_look_queue()
{
    queue_t *students_queue = globals_get_queue();
    if (students_queue->_length == 0)
    {
        printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        return 1;
    }

    return 0;
}

void worker_gate_remove_student()
{
    queue_t *students_queue = globals_get_queue();
    msleep(500);
    if (students_queue->_length != 0)
    {
        printf("tamanho da fila %d\n", students_queue->_length);
        printf("primeiro estudante removido\n");
        pthread_mutex_lock(&remove_student_mutex);
        queue_remove(students_queue);
        pthread_mutex_unlock(&remove_student_mutex);
    }
}

void worker_gate_look_buffet()
{
    // nao esta sendo usado
}

void *worker_gate_run(void *arg)
{
    int all_students_entered;
    int number_students;

    number_students = *((int *)arg);
    all_students_entered = number_students > 0 ? FALSE : TRUE;

    pthread_mutex_init(&buffet_first_position_mutex, NULL);
    pthread_mutex_init(&remove_student_mutex, NULL);

    int number_of_buffets = globals_get_number_of_buffets();
    // inicializar o mutex do worker gate init queue

    sem_init(&ratchet, 0, number_of_buffets * 2); // inicializando ele com numero de buffets x 2 porque tem 2 filas
    int sval2;
    sem_getvalue(&ratchet, &sval2);
    // printf("Semaphore value: %d\n", sval2);

    while (all_students_entered == FALSE)
    {

        worker_gate_look_buffet();
        worker_gate_remove_student();
        int students_on_queue = worker_gate_look_queue();
        int number_of_students = globals_get_students();
        if (students_on_queue != 0)
        {
            break;
        }
        // msleep(5000); /* Pode retirar este sleep quando implementar a solução! */
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
    buffet_t *buffets = globals_get_buffets();
    int number_of_buffets = globals_get_number_of_buffets();
    int sval2;
    sem_getvalue(&ratchet, &sval2);
    printf("Semaphore value: %d\n", sval2);
    sem_wait(&ratchet); // semafaro que evita espera ocupada
    pthread_mutex_lock(&buffet_first_position_mutex);
    for (int i = 0; i < number_of_buffets; i++)
    {
        int number_of_students = globals_get_students();
        if (buffets[i].queue_left[0] == 0)
        {
            student->_id_buffet = buffets[i]._id;
            student->left_or_right = 'L';
            buffet_queue_insert(buffets, student);
            pthread_mutex_unlock(&buffet_first_position_mutex);
            printf("ugauga %d\n", number_of_students);
            break;
        }
        if (buffets[i].queue_right[0] == 0)
        {
            student->_id_buffet = buffets[i]._id;
            student->left_or_right = 'R';
            buffet_queue_insert(buffets, student);
            pthread_mutex_unlock(&buffet_first_position_mutex);
            printf("ugauga %d\n", number_of_students);
            break;
        }
    }
}