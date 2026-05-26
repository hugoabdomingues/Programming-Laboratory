#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>



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
 * função que visa colocar um elemento ITEM dado no índice INDEX dado
 * @param v Vetor dinâmico
 * @param index índice do vetor
 * @param item elemento a ser colocado no índice INDEX
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
 * Esta função devolve o elemento que está no índice passado em argumento (index)
 * @param v Vetor dinâmico
 * @param index índice do vetor
 * @return elemento do índice INDEX
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
 * Esta função devolve o último elemento do array
 * @param v vetor dinâmico
 * @return elemento do fim do array
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
 * insere o elemento value no indice index no campo data do dynvec apontado por v.
 * Todos os elementoes de index para a direita até length-1 são puxados de uma posição à direita,
 * para dar espaço ao novo elemento.
 * @param v array dinâmico
 * @param index índice onde o elemento VALUE deve ser inserido
 * @param value elemento a ser inserido no índice INDEX
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
        //Com os shifts todos feitos, falta apenas meter o elemento no índice INDEX que está agr vazio.
        void *indice_destino = (char *)v->data + index * v->elem_size;
        memmove(indice_destino, value, v->elem_size);
        v->length++; //finalmente aumentamos a length que tinhamos ignorado até agr
    }
}
/**
 * esta função remove o elemento da posição index do vetor data de v. Todos os elementoes de index para
 *  a direita até length -1 são puxados de uma posição à esquerda, para ocupar o espaço libertado.
 * @param v array dinâmico
 * @param index índice onde o elemento VALUE deve ser inserido
 * @param value elemento a ser inserido no índice INDEX
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
 * verifica se um determinado elemento ITEM é elemento do dynvec v
 * @return true se encontrar o elemento ITEM for elemento do dynvec e false se não for.
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
        void *elemento_atual = (char *)v->data + i * v->elem_size;
        // memcmp devolve 0 se os blocos de memória forem iguais
        if (memcmp(item, elemento_atual, v->elem_size) == 0) { // usamos memcmp para comparar os elementoes dos ponteiros
            return true;
        }
    }
    return false;
}

/**
 * devolve o primeiro índice onde se encontra o elemento inteiro passado em parâmetro (item) no dynvec
 * igualmente passado por referência (v).
 * @param v array dinâmico
 * @param item
 * @return index ou erro
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
        void *elemento_atual = (char *)v->data + i * v->elem_size;
        // memcmp devolve 0 se os blocos de memória forem iguais
        if (memcmp(item, elemento_atual, v->elem_size) == 0) { // usamos memcmp para comparar os elementoes dos ponteiros
            return i;
        }
    }
    return -1;
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



//Nó da lista ligada que contem o elemento e um ponteiro para o próximo nó. O campo
typedef struct s_node {
    void *content; //elemento do nó: aponta para um int alocado no heap
    struct s_node *next;    //ponteiro para o próximo nó (NULL se for o último)
} node;

//informação de cada lista, guardada no dynvec.
typedef struct {
    int   id;
    node *head;// ponteiro para o primeiro nó da lista
    node *current; //ponteiro para o nó atual, usado para print
    node *tail;// ponteiro para o último nó da lista
} lista;

/**
 * Procura no dynvec a lista identificada pelo número em "id_procurado".
 * Devolve um ponteiro direto para a sua entrada no dynvec,
 * ou NULL se não existir nenhuma lista com esse id.
 * @param v Vetor dinâmico que contém todas as listas
 * @param id_procurado Identificador (número) da lista a encontrar
 * @return Ponteiro para a lista encontrada, ou NULL se não existir
 */
lista *encontrar_lista(dynvec *v, int id_procurado) {

    for (size_t i = 0; i < dynvec_length(v); i++) {
        
        lista *lista_atual = (lista *)dynvec_get(v, i);

        if (lista_atual->id == id_procurado){
            return lista_atual;
        }
    }
    return NULL;
}

/**
 * Cria e devolve um novo nó com o inteiro "elemento" armazenado no heap.
 * O int é alocado separadamente porque o campo content é void*.
 * @param elemento número inteiro a guardar no novo nó da lista
 * @return Ponteiro para o novo nó criado
 */
node *criar_no(int elemento) {
    node *new_node = (node *)malloc(sizeof(node));
    if (!new_node) {
        errno = ENOMEM;
        #ifdef DEBUG_ON
            perror("criar_no. Erro ao alocar memória para o nó.");
        #endif
        return NULL;
    }
    int *elemento_heap = (int *)malloc(sizeof(int));
    if (!elemento_heap) {
        errno = ENOMEM;
        #ifdef DEBUG_ON
            perror("criar_no. Erro ao alocar memória para o elemento do nó.");
        #endif
        free(new_node);
        return NULL;
    }
    *elemento_heap = elemento;        //guarda o elemento no heap
    new_node->content = elemento_heap;   //content aponta para o int alocado
    new_node->next = NULL;         //ainda não tem nó seguinte
    return new_node;
}

/**
 * create <id>: cria uma nova lista vazia com o identificador id.
 * Não faz nada se já existir uma lista com esse id.
 * @param v  Vetor dinâmico com todas as listas
 * @param id Identificador da nova lista
 */
void create(dynvec *v, int id) {

    if (encontrar_lista(v, id)) return; //já existe, logo, ignoraramos

    lista nova_lista = {id, NULL, NULL, NULL};
    dynvec_push(v, &nova_lista);
}

/**
 * addtail <id> <elemento>: adiciona o inteiro "elemento" ao fim da lista "id".
 * @param v Vetor dinâmico com todas as listas
 * @param id número Identificador da lista
 * @param elemento elemento a adicionar ao fim
 */
void addtail(dynvec *v, int id, int elemento) {

    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual == NULL) return; //se não encontrar a lista, não fazemos nada

    node *new_node = criar_no(elemento);

    if (!new_node) return; //se houve erro ao criar o nó, não fazemos nada

    if (lista_atual->head == NULL) {

        //lista estava vazia: o novo nó é simultaneamente head, current e tail
        lista_atual->tail = new_node;
        lista_atual->current = new_node;
        lista_atual->head    = new_node;

    } else { // lista não estava vazia: o novo nó é o novo tail, e o antigo tail aponta para ele
        //encadeia o novo nó depois do tail atual
        lista_atual->tail->next = new_node;
        lista_atual->tail = new_node;
    }
}

/**
 * addhead <id> <elemento>: adiciona o inteiro "elemento" no início da lista "id".
 * current é atualizado para o novo head 
 * depois de addhead, um print imediato deve devolver o novo elemento inserido.
 * @param v     Vetor dinâmico com todas as listas
 * @param id    Identificador da lista
 * @param elemento elemento a adicionar no início
 */
void addhead(dynvec *v, int id, int elemento) {

    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual == NULL) return; //se não encontrar a lista, não fazemos nada
    
    node *new_node = criar_no(elemento);

    if (!new_node) return; //se houve erro ao criar o nó, não fazemos nada

    //se a lista estiver vazia, o novo nó é simultaneamente head, current e tail. 
    if (lista_atual->head == NULL) {

        lista_atual->head = new_node;
        lista_atual->current = new_node;
        lista_atual->tail = new_node;
    
        //Se não estiver vazia, o novo nó passa a ser o head e aponta para o antigo head.
    } else {
        new_node->next = lista_atual->head; //novo nó aponta para o antigo head
        lista_atual->head = new_node;
        lista_atual->current = new_node; //current passa a ser o novo head
    }
}

/**
 * add <id> <index> <elemento>: insere o elemento no index da lista id. 
 * Se a index for 0, é equivalente a addhead. Se for igual ao tamanho da lista, é equivalente a addtail.
 * --> Index 0 = início (igual a addhead). Index len = fim (igual a addtail).
 * Se a index for inválida (maior que o tamanho da lista), não faz nada.
 * @param v       Vetor dinâmico com todas as listas
 * @param id      Identificador da lista
 * @param index    Index onde inserir o elemento na lista (id) 
 * @param elemento elemento a inserir
 */
void add(dynvec *v, int id, int index, int elemento) {

    if (index < 0) return; //index negativo é inválido, logo, não fazemos nada
    
    //se index for 0, é addhead.
    if (index == 0) { 
        addhead(v, id, elemento); 
        return; 
    } 

    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual == NULL) return; //se não encontrar a lista, não fazemos nada -->return

    /**
     * percorrer index-1 passos para chegar ao nó anterior à posição de inserção para depois encadear o 
     * novo nó entre o nó anterior e o nó que estava nessa posição
     */
    node *no_anterior = lista_atual->head; //começa no head

    //percorre index-1 passos para chegar ao nó anterior à posição de inserção
    for (int i = 0; i < index - 1; i++) { 
        if (no_anterior == NULL) {
            return; // index fora dos limites
        }
        no_anterior = no_anterior->next;
    }

    if (no_anterior == NULL) return; //index fora dos limites da lista

    //cria o novo nó a inserir
    node *new_node = criar_no(elemento);


    if (new_node == NULL) return; //se houve erro ao criar o nó, não fazemos nada
    
    new_node->next = no_anterior->next; //novo nó aponta para o nó que estava na posição de inserção (pode ser NULL se estivermos a inserir no fim)
    no_anterior->next = new_node; //nó anterior passa a apontar para o novo nó, encadeando-o na lista
    
    if (new_node->next == NULL) lista_atual->tail = new_node; //inserimos no fim: atualizamos a tail
}

/**
 * deltail <id>: remove o último elemento da lista id.
 * Se current apontava para o tail --> removido, passa a NULL.
 * @param v  Vetor dinâmico com todas as listas
 * @param id Identificador da lista
 */
void deltail(dynvec *v, int id) {

    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual == NULL || lista_atual->head == NULL) return; //se não encontrar a lista ou se a lista estiver vazia, não fazemos nada

    node *no_a_remover = lista_atual->tail;
   
    if (lista_atual->head == lista_atual->tail) { 
        //único elemento: lista fica vazia
        if (lista_atual->current == no_a_remover)  { //current apontava para o tail que vai ser removido, logo, current passa a NULL
            lista_atual->current = NULL;
        }

        lista_atual->head = NULL;
        lista_atual->tail = NULL;

    //Se a lista tinha mais do que um elemento, percorremos até ao penúltimo para o tornar no novo tail, e depois libertamos o antigo tail.
    } else {
        //percorrer até ao penúltimo para o tornar no novo tail
        node *penultimo = lista_atual->head;

        while (penultimo->next != lista_atual->tail) {//enquanto o próximo do penúltimo não for o tail, continua a avançar
            penultimo = penultimo->next;
        }


        if (lista_atual->current == no_a_remover)  {
            lista_atual->current = NULL;
        } 

        penultimo->next = NULL;
        lista_atual->tail = penultimo;
    }
    free(no_a_remover->content);
    free(no_a_remover);
}

/**
 * delhead <id>: remove o primeiro elemento da lista id.
 * Se current apontava para o head removido, avança para o nó seguinte.
 * @param v  Vetor dinâmico com todas as listas
 * @param id Identificador da lista
 */
void delhead(dynvec *v, int id) {

    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual == NULL || lista_atual->head == NULL) return;

    node *no_a_remover = lista_atual->head;

    if (lista_atual->current == no_a_remover) { 
        lista_atual->current = no_a_remover->next; //current avança para o nó seguinte se estava a apontar para o head que vai ser removido
    }

    lista_atual->head = no_a_remover->next; //head passa a ser o nó seguinte ao head removido (pode ser NULL se a lista ficar vazia)

    if (lista_atual->head == NULL) { //lista ficou vazia: atualizamos tail e current para NULL
        lista_atual->tail = NULL;
        lista_atual->current = NULL;
    }
    free(no_a_remover->content);
    free(no_a_remover);
}

/**
 * del <id> <index>: remove o elemento no index «index».
 * Se a index for inválida, não faz nada.
 * @param v Vetor dinâmico com todas as listas
 * @param id Identificador da lista
 * @param index Index do elemento a remover
 */
void del(dynvec *v, int id, int index) {

    if (index < 0) return;
    
    if (index == 0) { //index 0 é o head, logo, é delhead
        delhead(v, id);
         return;
    } 

    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual == NULL || lista_atual->head == NULL) return;

    //percorrer index-1 passos para chegar ao nó anterior ao que vamos remover
    node *no_anterior = lista_atual->head;
   
    for (int i = 0; i < index - 1; i++) {
        if (no_anterior == NULL) {
            return; //index fora dos limites
        }
        no_anterior = no_anterior->next;
    }
    if (no_anterior->next == NULL) return; //index fora dos limites da lista
    
    node *no_a_remover = no_anterior->next;
    
    if (lista_atual->current == no_a_remover) {
        lista_atual->current = no_a_remover->next; //current avança se estava no nó removido
    }

    no_anterior->next = no_a_remover->next; //salta o nó removido na cadeia
    
    if (no_a_remover->next == NULL) {
        lista_atual->tail = no_anterior; //removeu o tail: o anterior passa a ser tail
    }
    free(no_a_remover->content);
    free(no_a_remover);
}

/**
 * print <id>: imprime o elemento do nó atual (current) e avança current para o seguinte.
 * Imprime NULL se current for NULL (lista vazia ou chegou ao fim da lista).
 * @param v  Vetor dinâmico com todas as listas
 * @param id Identificador da lista
 */
void print(dynvec *v, int id) {

    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual == NULL) return;

    if (lista_atual->current == NULL) {
        printf("NULL\n");
        return;
    }

    // *(int *)lista_atual->current->content:
    //content é void* --> convertemos para int* --> desreferencia para obter o int
    printf("%d\n", *(int *)lista_atual->current->content);

    lista_atual->current = lista_atual->current->next; //avança current para o próximo nó
}

/**
 * restart <id>: volta a colocar current no início da lista (head).
 * @param v  Vetor dinâmico com todas as listas
 * @param id Identificador da lista
 */
void restart(dynvec *v, int id) {
    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual != NULL) {
        lista_atual->current = lista_atual->head; //current volta a apontar para o head
    }
}

/**
 * find <id> <elemento>: percorre a lista "<id>" à procura do «elemento». Imprime Yes ou No.
 * @param v Vetor dinâmico com todas as listas
 * @param id Identificador da lista
 * @param elemento elemento inteiro a procurar
 */
void find(dynvec *v, int id, int elemento) {
    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual == NULL) { 
        printf("No\n"); 
        return; 
    }

    dynvec *visitados = dynvec_create(sizeof(node *)); //vetor dinâmico para guardar os nós já visitados e evitar ciclos infinitos

    node *no_atual = lista_atual->head; //começamos a percorrer a lista desde o head
    
    while (no_atual != NULL && dynvec_contains(visitados, &no_atual) == false) { //enquanto não chegarmos ao fim da lista e não visitarmos um nó já visitado (ciclo)
        
        if (*(int *)no_atual->content == elemento) { //encontrámos o elemento: imprimimos Yes, libertamos o vetor de visitados e retornamos
            
            dynvec_free(visitados);
            printf("Yes\n");
            return;
        }
        //marcamos o nó atual como visitado, para não entrarmos em ciclo infinito se houver um ciclo na lista
        dynvec_push(visitados, &no_atual);
        no_atual = no_atual->next;
    }
    dynvec_free(visitados);
    printf("No\n");
}

/**
 * len <id>: conta e imprime o número de elementos na lista.
 * @param v  Vetor dinâmico com todas as listas
 * @param id Identificador da lista
 */
void len(dynvec *v, int id) {

    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual == NULL || lista_atual->head == NULL) { 
        printf("0\n"); 
        return; 
    }

    dynvec *visitados = dynvec_create(sizeof(node *));

    node *no_atual = lista_atual->head;
    
    int tamanho = 0;
    while (no_atual != NULL && dynvec_contains(visitados, &no_atual) == false) { //enquanto não chegarmos ao fim da lista e não visitarmos um nó já visitado (ciclo)
       
        dynvec_push(visitados, &no_atual);
      
        no_atual = no_atual->next;
        tamanho++;
    }
    dynvec_free(visitados);
    printf("%d\n",tamanho);
}

/**
 * concat <id1> <id2>: liga o fim da lista id1 ao início da lista id2.
 * Após isto, os nós de id2 ficam partilhados entre as duas listas.
 * @param v   Vetor dinâmico com todas as listas
 * @param id1 Identificador da lista que recebe a concatenação
 * @param id2 Identificador da lista a concatenar (a sua meta-info não muda)
 */
void concat(dynvec *v, int id1, int id2) {

    //encontrar as duas listas no dynvec
    lista *lista1 = encontrar_lista(v, id1);
    lista *lista2 = encontrar_lista(v, id2);

    if (lista1 == NULL || lista2 == NULL || lista2->head == NULL) return;
    
    if (lista1->head == NULL) { 
        //lista1 estava vazia --> passa a apontar para os nós de lista2
        lista1->head = lista2->head; 
        lista1->current = lista2->head;
        lista1->tail = lista2->tail;
    } else {
        //liga o último nó de lista1 ao primeiro de lista2
        lista1->tail->next = lista2->head;
        lista1->tail = lista2->tail;
    }
}

/**
 * cycle <id>: deteta ciclos com o algoritmo de Floyd.
 * Dois ponteiros partem do head: ponteiro_lento avança 1 nó por iteração,
 * ponteiro_rapido avança 2. Se se encontrarem, há ciclo. Se ponteiro_rapido
 * chegar a NULL, não há ciclo.
 * Imprime "Yes, <n>" se houver ciclo (n = elemento no nó do encontro), ou "No".
 * @param v  Vetor dinâmico com todas as listas
 * @param id Identificador da lista
 */
void cycle(dynvec *v, int id) {

    lista *lista_atual = encontrar_lista(v, id);

    if (lista_atual == NULL) return;

    if (lista_atual->head == NULL) { 
        printf("No\n"); 
        return; 
    }

    node *ponteiro_lento  = lista_atual->head;
    node *ponteiro_rapido = lista_atual->head;

    while (1) {
        //se o ponteiro rápido chegar ao fim, não há ciclo.
        if (ponteiro_rapido->next == NULL || ponteiro_rapido->next->next == NULL) {
            printf("No\n");
            return;
        }

        ponteiro_lento = ponteiro_lento->next; //avança 1 casa
        ponteiro_rapido = ponteiro_rapido->next->next; //avança 2 casas

        if (ponteiro_lento == ponteiro_rapido) {
            //encontraram-se ---> há ciclo
            printf("Yes, %d\n", *(int *)ponteiro_lento->content);
            return;
        }
    }
}

/**
 * Liberta toda a memória alocada ao terminar o programa.
 * Resolve dois problemas:
 *   1) Ciclos: anula tail->next antes de percorrer, para não entrar em loop infinito.
 *   2) Nós partilhados (após concat): recolhe ponteiros únicos num dynvec auxiliar
 *      e liberta cada nó exatamente uma vez (evita double-free/crash).
 * @param v Vetor dinâmico com todas as listas (é libertado no final)
 */
void libertar_tudo(dynvec *v) {
    //1º passo: quebrar ciclos em todas as listas
    for (size_t i = 0; i < dynvec_length(v); i++) {

        lista *lista_atual = (lista *)dynvec_get(v, i);

        if (lista_atual->tail) lista_atual->tail->next = NULL;
    }
    //2º passo: recolher os endereços de todos os nós únicos
    dynvec *nos_vistos = dynvec_create(sizeof(node *));

    for (size_t i = 0; i < dynvec_length(v); i++) {

        lista *lista_atual = (lista *)dynvec_get(v, i);

        node *no_atual = lista_atual->head;
        while (no_atual != NULL) {
            if (!dynvec_contains(nos_vistos, &no_atual)) {
                dynvec_push(nos_vistos, &no_atual);
            }
            no_atual = no_atual->next;
        }
    }
    //3º passo: libertar cada nó exatamente uma vez
    for (size_t i = 0; i < dynvec_length(nos_vistos); i++) {
        //dentro do dynvec está guardado um node* --> (node **) para o desreferenciar
        node *no_a_libertar = *(node **)dynvec_get(nos_vistos, i);
        free(no_a_libertar->content);
        free(no_a_libertar);
    }
    dynvec_free(nos_vistos);
    dynvec_free(v);
}

int main() {
    dynvec *v = dynvec_create(sizeof(lista)); //dynvec com a informação das listas
    char comando[20];
    while (scanf("%19s", comando) == 1) {
        if (strcmp(comando, "create") == 0) {
            int id; scanf("%d", &id);
            create(v, id);
        } else if (strcmp(comando, "addtail") == 0) {
            int id, elemento; scanf("%d %d", &id, &elemento);
            addtail(v, id, elemento);
        } else if (strcmp(comando, "addhead") == 0) {
            int id, elemento; scanf("%d %d", &id, &elemento);
            addhead(v, id, elemento);
        } else if (strcmp(comando, "add") == 0) {
            int id, posicao, elemento; scanf("%d %d %d", &id, &posicao, &elemento);
            add(v, id, posicao, elemento);
        } else if (strcmp(comando, "deltail") == 0) {
            int id; scanf("%d", &id);
            deltail(v, id);
        } else if (strcmp(comando, "delhead") == 0) {
            int id; scanf("%d", &id);
            delhead(v, id);
        } else if (strcmp(comando, "del") == 0) {
            int id, posicao; scanf("%d %d", &id, &posicao);
            del(v, id, posicao);
        } else if (strcmp(comando, "print") == 0) {
            int id; scanf("%d", &id);
            print(v, id);
        } else if (strcmp(comando, "restart") == 0) {
            int id; scanf("%d", &id);
            restart(v, id);
        } else if (strcmp(comando, "find") == 0) {
            int id, elemento; scanf("%d %d", &id, &elemento);
            find(v, id, elemento);
        } else if (strcmp(comando, "len") == 0) {
            int id; scanf("%d", &id);
            len(v, id);
        } else if (strcmp(comando, "concat") == 0) {
            int id1, id2; scanf("%d %d", &id1, &id2);
            concat(v, id1, id2);
        } else if (strcmp(comando, "cycle") == 0) {
            int id; scanf("%d", &id);
            cycle(v, id);
        }
    }
    libertar_tudo(v);
    return 0;
}
