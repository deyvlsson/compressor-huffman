#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>

typedef struct node {
    char caracteres[256];
    unsigned freq;
    struct node* esq;
    struct node* dir;
} node;

typedef struct elementoLista {
    node *dado;
    struct elementoLista *prox;
} elementoLista;

typedef struct buffer {
    unsigned char buffer;
    unsigned bufferSize;
} buffer;

node* createNode(char* texto, unsigned frequencia);
void inserirOrdenado(elementoLista **inicio, node *nodeArvore);
node* removerElemento(elementoLista **inicio);
void initBuffer(buffer *b);
void writeBuffer(buffer *b, int bit, FILE *arquivo);
void limparBuffer(buffer *b, FILE *arquivo);

#endif