#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "worker_gate.h"
#include "globals.h"
#include "config.h"


void worker_gate_look_queue()
{   
    //queue_t students_queue = *globals_get_queue();
    //int student_number = students_queue._first;
}

void worker_gate_remove_student()
{   
    //queue_t students_queue = *globals_get_queue();
    //student_t *removed_student; 
    //removed_student = queue_remove(&students_queue);
    //tira o estudante da fila para que ele seja designado para algum buffet

}

void worker_gate_look_buffet()
{
 //nao esta sendo usado
}

void *worker_gate_run(void *arg)
{
    int all_students_entered;
    int number_students;

    number_students = *((int *)arg);
    all_students_entered = number_students > 0 ? FALSE : TRUE;

    int number_of_buffets = globals_get_number_of_buffets();

    sem_t ratchet;// semafaro catraca para o worker gate so procurar posicoes livres nas filas do buffet quando elas existirem(evita espera ocupada)
    sem_init(&ratchet, 0, number_of_buffets*2);// inicializando ele com numero de buffets x 2 porque tem 2 filas 

    while (all_students_entered == FALSE)
    {
        number_students --;
        if (number_students == 0) {
            all_students_entered = TRUE;
            sem_destroy(&ratchet);
        }
        worker_gate_look_buffet();
        worker_gate_remove_student();
        msleep(5000); /* Pode retirar este sleep quando implementar a solução! */
    }

    pthread_exit(NULL);
}

void worker_gate_init(worker_gate_t *self)
{
    int number_students = globals_get_students();
    pthread_create(&self->thread, NULL, worker_gate_run, &number_students);
}

void worker_gate_finalize(worker_gate_t *self)
{
    pthread_join(self->thread, NULL);
    free(self);
}

void worker_gate_insert_queue_buffet(student_t *student)
{
    buffet_t *buffets = globals_get_buffets();
    int number_of_buffets = globals_get_number_of_buffets();
    sem_wait(&ratchet);// semafaro que evita espera ocupada
    for (int i = 0; i < number_of_buffets; i++){
        if (buffets[i].queue_left[0] == 0){
            student->_id_buffet = buffets[i]._id;
            student->left_or_right = 'L';
            buffet_queue_insert(buffets,student);
        }
        if (buffets[i].queue_right[0] == 0){
            student->_id_buffet = buffets[i]._id;
            student->left_or_right = 'R';
            buffet_queue_insert(buffets,student);
        }
    }
}