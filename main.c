#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

typedef struct Dynvec {
    int *data;   
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
    
    
}


