#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

typedef struct Dynvec {
    int *data;       
    size_t capacity; /* capacidade total do vetor  */
    size_t length; //espaço do vetor preenchido com data
} dynvec;

#define DYNVEC_INIT_CAPACITY 4

dynvec *dynvec_create(void){
    dynvec *v = (dynvec *)malloc(sizeof(dynvec));
    if (!v){
        errno = ENOMEM;
        #ifdef DEBUG_ON
            perror("Create. Erro ao alocar memória");
        #endif
        return NULL;
    }

    v->capacity = DYNVEC_INIT_CAPACITY;
    v->length = 0;
    v->data = (int *)malloc(sizeof(int) * v->capacity);

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
    v->data = (int *)malloc(sizeof(int) * v->capacity);
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

static void dynvec_resize(dynvec *v, int size) {
    if (v->capacity == v->length)
    {
        v->capacity = v->capacity * 2;
    } else if (v->length <= v->capacity * 0.25);
    {
        v->capacity = v->capacity / 2;
    }
    
}


