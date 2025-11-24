#include <stdlib.h>
#include <string.h>
#include "huffman.h"

node* createNode(char* texto, unsigned frequencia) {
    node* novo = (node*)malloc(sizeof(node)); 
    strcpy(novo->caracteres, texto);
    novo->freq = frequencia;
    novo->esq = NULL;
    novo->dir = NULL;
    return novo;
}

void inserirOrdenado(elementoLista **inicio, node *nodeArvore) {
    elementoLista *novoElemento = (elementoLista*)malloc(sizeof(elementoLista));
    novoElemento->dado = nodeArvore;
    novoElemento->prox = NULL;

    if (*inicio == NULL || nodeArvore->freq < (*inicio)->dado->freq) {
        novoElemento->prox = *inicio;
        *inicio = novoElemento;    
    } else {
        elementoLista *atual = *inicio;
        while (atual->prox != NULL && atual->prox->dado->freq <= nodeArvore->freq) {
            atual = atual->prox;
        }
        novoElemento->prox = atual->prox;
        atual->prox = novoElemento;
    }
}

node* removerElemento(elementoLista **inicio) {
    if (*inicio == NULL) return NULL;
    elementoLista *removido = *inicio;
    node *nodeArvore = removido->dado;
    *inicio = removido->prox;
    free(removido);
    return nodeArvore;
}

void initBuffer(buffer *b) {
    b->buffer = 0;
    b->bufferSize = 0;
}

void writeBuffer(buffer *b, int bit, FILE *arquivo) {
    if (b->bufferSize == 8) {
        fwrite(&(b->buffer), sizeof(unsigned char), 1, arquivo);
        b->buffer = 0;
        b->bufferSize = 0;
    }
    b->buffer = (b->buffer << 1);
    if (bit == 1) b->buffer = b->buffer | 1;
    b->bufferSize++;
    if (b->bufferSize == 8) {
        fwrite(&(b->buffer), sizeof(unsigned char), 1, arquivo);
        b->buffer = 0;
        b->bufferSize = 0;
    }
}

void limparBuffer(buffer *b, FILE *arquivo) {
    if (b->bufferSize > 0) {
        b->bufferSize = b ->buffer << (8 - b->bufferSize);
        fwrite(&(b->buffer), sizeof(unsigned char), 1, arquivo);
        b->buffer = 0;
        b->bufferSize = 0;
    }
}