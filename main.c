#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct Dynvec {
    void *data;   
    size_t elem_size;    
    size_t capacity; /* capacidade total do vetor  */
    size_t length; //espaço do vetor preenchido com data
} dynvec;

#define DYNVEC_INIT_CAPACITY 4

dynvec *dynvec_create(size_t elem_size){
    dynvec *v = (dynvec *)malloc(sizeof(dynvec));
    if (!v){
        errno = ENOMEM;
        #ifdef DEBUG_ON
            perror("Create. Erro ao alocar memória");
        #endif
        return NULL;
    }

    v->elem_size = elem_size; //Isto define quantos bytes cada elemento ocupará no vetor dinâmico
    v->capacity = DYNVEC_INIT_CAPACITY;
    v->length = 0;
    v->data = malloc(elem_size * v->capacity);

    if ((v->data) == NULL) {
        errno = ENOMEM;
        #ifdef DEBUG_ON
            perror("Create. Erro no array, ao alocar memória");
        #endif

        free(v);
        return NULL;
    }
    return v;
}

void dynvec_empty(dynvec *v) {
    if (v->data)
    {
        free(v);
    }
    
    v->capacity = DYNVEC_INIT_CAPACITY; 
    v->length = 0;
    v->data = malloc(v->elem_size * v->capacity);
    if (!v)
    {
        errno = ENOMEM;
        #ifdef DEBUG_ON
            perror("Erro ao alocar memória.")
        #endif
        
    }
    return;
}

size_t dynvec_capacity(dynvec *v){
    return v->capacity;
}

size_t dynvec_length(dynvec *v) {
    return v->length;
}

static void dynvec_resize(dynvec *v, size_t size) {

    /*realloc: se houver espaço a seguir ao bloco atual, ele apenas estica o bloco. Se não houver, 
    ele encontra um novo lugar na memória suficientemente grande, copia os dados antigos lá para dentro automaticamente, e apaga o bloco antigo.*/
    void *new_memory = realloc(v->data, v->elem_size * size); //O novo tamanho em bytes é calculado multiplicando o tamanho de cada elemento (v->elem_size) pela nova capacidade (size)
    if (!v)
    {
        errno = ENOMEM;
        #ifdef DEBUG_ON
            perror("Resize. Erro no array, ao realocar memória");
        #endif
        return;
    }
    v->data = new_memory;
    v->capacity = size; 
    
}
/* Nota: v->data é um ponteiro do tipo genérico void *. Como em C o compilador não sabe o tamanho dos elementos
 para onde um void * aponta, não é permitido fazer operações aritméticas diretamente num void *.
A solução em C é fazer o cast para char * porque um char tem, por definição, exatamente 1 byte de tamanho. Assim, se somares 1 a um char *, 
avançamos exatamente 1 byte de memória.
 */

void dynvec_push (dynvec *v, void *item) {

    if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("Push. Vetor dinâmico inválido.");
        #endif
        return;
    }
    if (v->length >= v->capacity) {
        dynvec_resize(v, v->capacity * 2);
    }
  
    //convertemos para (char *) pois em C não podemos fazer aritmética com void * diretamente.
    /*Isto acontece porque o compilador não sabe o tamanho dos elementos apontados por void 
    char tem tamanho fixo de 1 byte, então char * permite aritmética de ponteiros byte a byte.
    Assim: (char *)v->data + 2 --> estamos a avançar 2 bytes*/
    memmove((char *)v->data + v->length * v->elem_size, item, v->elem_size ); // memcpy: copia v->elem_size bytes de item para o índice [length] de data
    v->length++; 
    //((char *)v->data + v->length * v->elem_size)
     //memmove é mais seguro que memcpy. memcpy falha quando áreas da mamória se sobrepõem

}

void dynvec_pop (dynvec *v) {
    if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("Pop. Vetor dinâmico inválido.");
        #endif
        return;
    }
    //guardamos o item que queremos dar pop antes de diminuirmos o length para dps dar return
    void *item = (char *)v->data + (v->length-1) * v->elem_size;
    v->length--;

    //redimensionar o vetor, caso a length seja igual ou menor que 1/4 do tamanho de capacity
    if (v->length <= v->capacity * 0.25) {
        dynvec_resize(v, v->capacity * 0.5);
    }  
    return item;
}



/**
 * função que visa colocar um item dado no índice INDEX dado também
 */
void dynvec_set(dynvec *v, size_t index, void *item) {
    if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("set. Vetor dinâmico inválido.");
        #endif
        return;
    }
    if (index == v->length) {
        dynvec_push(v, item);
    } else if (index < 0 || index > v->length) {
        errno = ERANGE;
        #ifdef DEBUG_ON
            perror("set. Índice invalido");
        #endif
        return;
    }
    memmove((char *)v->data + index * v->elem_size, item, v->elem_size);
}







