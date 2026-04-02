#include <stdio.h>
#include <stdlib.h>


// no da lista duyplamente encadeada
typedef struct Node
{
    int key; // chave do elemento 
    int value; // Valor do elemento 
    struct NODE* prev; // ponteiro para o no anterior
    struct NODE* next; // ponteiro para o proximo no 
} Node;

typedef struct LRUCache {
    int capacity; // capacidade maxima do cache
    int size;  // tamanho atual do cache
    Node* head; // ponteiro para o inicido do mais recente
    Node* tail; // ponteiro para o final do menos recente
} LRUCache;


// funcao que cria no 


Node* createNode(int key,int value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->key = key;
    newNode->value = value;
    newNode->prev = NULL; //  nao existe anterior do no anteiror logo ele tem que ser inicializado como nulo
    newNode->next = NULL; //  nao existe proximo do no anteiror logo ele tem que ser inicializado como nulo
    return newNode;
}

// inicializa o cache lru
LRUCache* createCache(int capacity) {
    LRUCache* cache = (LRUCache*)malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    return cache;
}

// mover um no para o inicio da lista (ele se torna mais eficiente )

void moveToHead(LRUCache* cache,Node* node) {
        if(node == cache->head) return;

        if(node->prev) node->prev->next = node->next;

        if(node->next) node->next->prev = node->prev;

        if(node == cache->tail) cache-> tail = node->prev;



        node->next = cache->head;

        node->prev = NULL;

        if(cache->head) cache->head->prev = node;
        cache->head=node;
    }
