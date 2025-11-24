#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

void gerarDicionario (node *raiz, char *codigo, char tabela[256][256]);

int main() {
    unsigned frequencias[256] = {0};
    int c;
    FILE *entrada = fopen("animacao.tex", "r");
    
    if (!entrada) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }
    
    while ((c = fgetc(entrada)) != EOF) {
        frequencias[c]++;
    }
    
    rewind(entrada);
    elementoLista *lista = NULL;

    for (int i = 0; i < 256; i++) {
        if (frequencias[i] > 0) {
            char str[2] = { (char)i, '\0' };
            node *novoNo = createNode(str, frequencias[i]);
            inserirOrdenado(&lista, novoNo);
        }
    }

    while (lista != NULL && lista->prox != NULL) {
        node *n1 = removerElemento(&lista);
        node *n2 = removerElemento(&lista);
        char nomePai[256] = "";
        strcpy(nomePai, n1->caracteres);
        strcpy(nomePai, n2->caracteres);
        node *pai = createNode(nomePai, n1->freq + n2->freq);
        pai->esq = n1;
        pai->dir = n2;
        inserirOrdenado(&lista, pai);
    }

    node *raiz = removerElemento(&lista);

    printf("Freq: %u)\n", raiz->freq);

    char tabela[256][256];
    for (int i = 0; i < 256; i++) {
        strcpy(tabela[i], "");
    }
       gerarDicionario(raiz, "", tabela);
       FILE *saida = fopen("animacao.huff", "wb");
       fwrite(frequencias, sizeof(unsigned), 256, saida);
       buffer b;
       initBuffer(&b);

       while ((c = fgetc(entrada)) != EOF) {
            char *cod = tabela[c];
            for (int i = 0; cod[i] != '\0'; i++) {
                if (cod[i] == '1') {
                    writeBuffer(&b, 1, saida);
                } else {
                    writeBuffer(&b, 0, saida);
                }
            }
       }

       limparBuffer(&b, saida);
       fclose(entrada);
       fclose(saida);
       return 0;
}

void gerarDicionario (node *raiz, char *codigo, char tabela[256][256]) {
    if (raiz->esq == NULL && raiz->dir == NULL) {
        unsigned char c =(unsigned char)raiz->caracteres[0];
        strcpy(tabela[c], codigo);
    } else {
        char esquerda[256], direita[256];
        strcpy(esquerda, codigo);
        strcat(esquerda, "0");
        strcpy(direita, codigo);
        strcat(direita, "1");

        if(raiz->esq) {
            gerarDicionario(raiz->esq, esquerda, tabela);
        }

        if(raiz->dir) {
            gerarDicionario(raiz->dir, direita, tabela);
        }
    }
}