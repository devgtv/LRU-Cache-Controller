#include <stdio.h>
#include <stdlib.h>


// no da lista duyplamente encadeada
typedef struct Node
{
    int key; // chave do elemento 
    int value; // Valor do elemento 
    struct NODE* prev; // ponteiro para o no anterior
    struct NODE* next; // ponteiro para o proximo no 
}Node;

typedef struct LRUCache {
    int capacity; // capacidade maxima do cache
    int size;  // tamanho atual do cache
    Node* head; // ponteiro para o inicido do mais recente
    Node* tail; // ponteiro para o final do menos recente
} LRUCache;

