#include <stdio.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



typedef struct Dynvec {
    void *data;   
    size_t elem_size;    
    size_t capacity; /* capacidade total do vetor  */
    size_t length; //espaço do vetor preenchido com data
} dynvec;

// Optamos por aproveitar o mecanismo das macros para
// concretizar esta semelhança.
#define stack_create dynvec_create
#define stack_free dynvec_free
#define stack_push dynvec_push
#define stack_pop dynvec_pop
#define stack_top dynvec_top
// A excepção é o predicado stack_is_empty,
// Apenas... basta defini-lo como uma igualdade directa
#define stack_is_empty(vec) (dynvec_length(vec) == 0)


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
    if (v->data){
        free(v->data);
    }
    
    v->capacity = DYNVEC_INIT_CAPACITY; 
    v->length = 0;
    v->data = malloc(v->elem_size * v->capacity);
    if (!v->data) {
        errno = ENOMEM;
        #ifdef DEBUG_ON
            perror("Erro ao alocar memória.");
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
    if (!new_memory) {
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
        errno = EINVAL; //erro de argumento inválido.
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
    if (v->length == 0) {
        return;
    }

    v->length--;

    //redimensionar o vetor, caso a length seja igual ou menor que 1/4 do tamanho de capacity
    if (v->length <= v->capacity * 0.25 && v->length > DYNVEC_INIT_CAPACITY) {
        dynvec_resize(v, v->capacity * 0.5);
    }  
}



/**
 * função que visa colocar um valor ITEM dado no índice INDEX dado
 * @param v Vetor dinâmico
 * @param index índice do vetor 
 * @param item valor a ser colocado no índice INDEX 
 * @return nada
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
        return;
    } else if (index > v->length) {
        errno = ERANGE; //erro de Index out of bounds
        #ifdef DEBUG_ON
            perror("set. Index out of bounds");
        #endif
        return;
    }
    memmove((char *)v->data + index * v->elem_size, item, v->elem_size);
}

/**
 * Esta função devolve o valor que está no índice passado em argumento (index)
 * @param v Vetor dinâmico
 * @param index índice do vetor 
 * @return valor do índice INDEX
 */
void* dynvec_get(dynvec *v, size_t index) {

    if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("get. Vetor dinâmico inválido.");
        #endif
        return NULL;
    }
    if (index >= v->length) {
        errno = ERANGE;
        #ifdef DEBUG_ON
            perror("get. Index out of bounds.");
        #endif
        return NULL; 
    }
    return (char *)v->data + index * v->elem_size;
}

/**
 * Esta função devolve o último valor do array
 * @param v vetor dinâmico
 * @return valor do fim do array
 */
void* dynvec_top(dynvec *v) {
    if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("top. Vetor dinâmico inválido.");
        #endif
        return NULL;
    }
    if (v->length == 0) {
        return NULL;
    }
    return (char *)v->data + (v->length - 1) * v->elem_size;
}
/**
 * insere o valor value no indice index no campo data do dynvec apontado por v. 
 * Todos os valores de index para a direita até length-1 são puxados de uma posição à direita, 
 * para dar espaço ao novo valor. 
 * @param v array dinâmico
 * @param index índice onde o valor VALUE deve ser inserido
 * @param value Valor a ser inserido no índice INDEX
 */
void dynvec_insert(dynvec *v, size_t index, void *value) {
    if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("Insert. Vetor dinâmico inválido.");
        #endif
        return;
    }
    //se for índice inválido
    if (index > v->length){
        errno = ERANGE;
        #ifdef DEBUG_ON
            perror("insert. Index out of bounds.");
        #endif
        return;
    }
    if (index == v->length){ //se o index for igual a length
        dynvec_push(v, value);
    } else { //se o índice solicitado tiver no meio do array --> right shift
        //right shift:
        if (v->length >= v->capacity) {
            dynvec_resize(v, v->capacity * 2);
        }
        for (int i = (int)v->length; i > index; i--) {
            void *src = (char *)v->data + (i-1) * v->elem_size;
            void *dest = (char *)v->data + i * v->elem_size;
            memmove(dest,src,v->elem_size);
        }
        //Com os shifts todos feitos, falta apenas meter o valor no índice INDEX que está agr vazio.
        void *indice_destino = (char *)v->data + index * v->elem_size; 
        memmove(indice_destino, value, v->elem_size); 
        v->length++; //finalmente aumentamos a length que tinhamos ignorado até agr
    }
}
/**
 * esta função remove o valor da posição index do vetor data de v. Todos os valores de index para
 *  a direita até length -1 são puxados de uma posição à esquerda, para ocupar o espaço libertado. 
 * @param v array dinâmico
 * @param index índice onde o valor VALUE deve ser inserido
 * @param value Valor a ser inserido no índice INDEX 
 * */
void dynvec_delete(dynvec *v, size_t index) {
    if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("delete. Vetor dinâmico inválido.");
        #endif
        return;
    }
    if (index >= v->length) {
        errno = ERANGE;
        #ifdef DEBUG_ON
            perror("delete. Index out of bounds.");
        #endif
        return;
    } else if (index == v->length-1) {
        dynvec_pop(v);
    } else {
        for (int i = index; i < (int)v->length-1; i++) {
            void *src = (char *)v->data + (i+1) * v->elem_size; //um índice à direita do INDEX
            void *dest = (char *)v->data + i * v->elem_size; //índice INDEX a ser sobreescrito
            memmove(dest,src,v->elem_size);
        }
        v->length--;
        if (v->length <= v->capacity * 0.25){
            dynvec_resize(v, v->capacity * 0.5);
        }
    }
}

/**
 * verifica se um determinado valor ITEM é elemento do dynvec v
 * @return true se encontrar o valor ITEM for elemento do dynvec e false se não for.
 */
bool dynvec_contains(dynvec* v, void *item) {
    if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("contains. Vetor dinâmico inválido.");
        #endif
        return false;
    }
    for (int i = 0; i < (int)v->length; i++) {
        void *valor_atual = (char *)v->data + i * v->elem_size;
        // memcmp devolve 0 se os blocos de memória forem iguais
        if (memcmp(item, valor_atual, v->elem_size) == 0) { // usamos memcmp para comparar os valores dos ponteiros
            return true;
        }
    }
    return false;  
}

/**
 * devolve o primeiro índice onde se encontra o valor inteiro passado em parâmetro (item) no dynvec 
 * igualmente passado por referência (v).
 * @param v array dinâmico
 * @param item 
 */
int dynvec_index(dynvec* v, void* item) {
    if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("contains. Vetor dinâmico inválido.");
        #endif
        return -1;
    }
    for (int i = 0; i < (int)v->length; i++) {
        void *valor_atual = (char *)v->data + i * v->elem_size;
        // memcmp devolve 0 se os blocos de memória forem iguais
        if (memcmp(item, valor_atual, v->elem_size) == 0) { // usamos memcmp para comparar os valores dos ponteiros
            return i;
        }
    }
    return -1;  
}
/**
 * Esta função imprime na saída standard (stdout) o conteúdo do dynvec passado em parâmetro.
 */
void dynvec_print (dynvec *v) {
        if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("print. Vetor dinâmico inválido.");
        #endif
        return;
    }

    // Convertemos os dados genéricos (void*) para um array de inteiros (int*)
    int *valores_do_array = (int *)v->data;

    for (int i = 0; i < (int)v->length; i++) {
        printf("%d ", valores_do_array[i]);
    }
    printf("\n");
}
/**
 * Esta função liberta todo o espaço do dynvec v passado por referência, campo a campo, terminando com v ele próprio.
 */
void dynvec_free (dynvec *v) {
    if (!v) {
        errno = EINVAL;
        #ifdef DEBUG_ON
            perror("free. Vetor dinâmico inválido.");
        #endif
        return;
    }
    if (v->data) {
        free(v->data);
    }
    free(v);    
}



int main() {

   return 0;
}