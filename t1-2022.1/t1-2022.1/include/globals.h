#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "queue.h"
#include "table.h"
#include "buffet.h"

/**
 * @brief retorna o numero de buffets
 * 
 * @return int 
 */
extern int globals_get_number_of_buffets();

/**
 * @brief seta o numero de buffets em uma variavel global
 * 
 * @param number_of_buffets 
 */

extern void globals_set_number_of_buffets(int number_of_buffets);

/**
 * @brief Inicia uma fila (de modo global)
 * 
 * @param queue 
 */
extern void globals_set_queue(queue_t *queue);

/**
 * @brief Retorna uma fila (de modo global)
 * 
 * @return queue_t* 
 */
extern queue_t *globals_get_queue();

/**
 * @brief Insere o número de alunos (de modo global)
 * 
 */
extern void globals_set_students(int number);

/**
 * @brief Retorna o número de alunos (de modo global)
 * 
 * @return int 
 */

extern int globals_get_students();

/**
 * @brief Inicia um array de mesas (de modo global).
 * 
 * @param t 
 */
extern void globals_set_table(table_t *t);

/**
 * @brief Retorna um array de mesas (de modo global)
 * 
 * @return table_t* 
 */
extern table_t *globals_get_table();


/**
 * @brief Finaliza todas as variáveis globais.
 * 
 */
extern void globals_finalize();

/**
 * @brief Inicia um array de buffets (de modo global)
 * 
 */
extern void globals_set_buffets(buffet_t *buffets_ref);

/**
 * @brief Retorna um array de buffets (de modo global)
 * 
 * @return buffet_t* 
 */
extern buffet_t *globals_get_buffets();

/**
* @brief Insere o número de mesas (de modo global)
* 
* @param tables
*/

extern void globals_set_number_of_tables(int tables);

/**
 * @brief Retorna o número de mesas (de modo global)
 * 
 * @return int 
 */

extern int globals_get_number_of_tables();

/**
 * @brief Insere o número de bancos em cada mesa (de modo global)
 * 
 * @param seats
 */
extern void globals_set_seats_per_table(int seats);

/**
 * @brief Retorna o número de bancos em cada mesa (de modo global)
 * 
 * @return int 
 */

extern int globals_get_seats_per_table();

#endif
