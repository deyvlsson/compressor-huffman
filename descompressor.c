#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 256

// --- Estruturas (Idênticas ao Compressor para consistência) ---

typedef struct Node {
    unsigned char character;
    unsigned int frequency;
    struct Node *left, *right;
} Node;

typedef struct {
    Node **nodes;
    int size;
    int capacity;
} MinHeap;

Node* createNode(unsigned char character, unsigned int frequency) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->character = character;
    node->frequency = frequency;
    node->left = node->right = NULL;
    return node;
}

MinHeap* createMinHeap(int capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->nodes = (Node**)malloc(minHeap->capacity * sizeof(Node*));
    return minHeap;
}

void swapNodes(Node** a, Node** b) {
    Node* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->nodes[left]->frequency < minHeap->nodes[smallest]->frequency)
        smallest = left;

    if (right < minHeap->size && minHeap->nodes[right]->frequency < minHeap->nodes[smallest]->frequency)
        smallest = right;

    if (smallest != idx) {
        swapNodes(&minHeap->nodes[smallest], &minHeap->nodes[idx]);
        minHeapify(minHeap, smallest);
    }
}

Node* extractMin(MinHeap* minHeap) {
    Node* temp = minHeap->nodes[0];
    minHeap->nodes[0] = minHeap->nodes[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(MinHeap* minHeap, Node* minHeapNode) {
    ++minHeap->size;
    int i = minHeap->size - 1;
    while (i && minHeapNode->frequency < minHeap->nodes[(i - 1) / 2]->frequency) {
        minHeap->nodes[i] = minHeap->nodes[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->nodes[i] = minHeapNode;
}

Node* buildHuffmanTree(unsigned int frequencies[]) {
    MinHeap* minHeap = createMinHeap(MAX_SYMBOLS);
    for (int i = 0; i < MAX_SYMBOLS; ++i) {
        if (frequencies[i] > 0)
            insertMinHeap(minHeap, createNode((unsigned char)i, frequencies[i]));
    }
    
    if (minHeap->size == 0) return NULL;

    while (minHeap->size != 1) {
        Node* left = extractMin(minHeap);
        Node* right = extractMin(minHeap);
        Node* top = createNode('$', left->frequency + right->frequency);
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);
    }
    Node* root = extractMin(minHeap);
    free(minHeap->nodes);
    free(minHeap);
    return root;
}

// --- Função Principal ---

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_comprimido.huff>\n", argv[0]);
        return 1;
    }

    char *inputFileName = argv[1];
    
    // Tenta gerar nome de saida restaurado
    // Ex: animacao.huff -> animacao_restaurado.tex
    char outputFileName[256];
    char *dot = strrchr(inputFileName, '.');
    if (dot && strcmp(dot, ".huff") == 0) {
        int baseLen = dot - inputFileName;
        strncpy(outputFileName, inputFileName, baseLen);
        outputFileName[baseLen] = '\0';
        strcat(outputFileName, "_restaurado.tex"); // Garante que nao sobrescreve o original
    } else {
        sprintf(outputFileName, "%s.restaurado", inputFileName);
    }

    FILE *inFile = fopen(inputFileName, "rb");
    if (!inFile) {
        printf("Erro ao abrir arquivo comprimido: %s\n", inputFileName);
        return 1;
    }

    // 1. Ler Cabeçalho (Tamanho Original + Frequências)
    long originalSize;
    unsigned int frequencies[MAX_SYMBOLS];
    
    if (fread(&originalSize, sizeof(long), 1, inFile) != 1 ||
        fread(frequencies, sizeof(unsigned int), MAX_SYMBOLS, inFile) != MAX_SYMBOLS) {
        printf("Erro: Arquivo corrompido ou formato invalido.\n");
        fclose(inFile);
        return 1;
    }

    // 2. Reconstruir a Árvore de Huffman
    Node* root = buildHuffmanTree(frequencies);
    if (!root) {
        printf("Erro ao reconstruir arvore (arquivo vazio?).\n");
        fclose(inFile);
        return 1;
    }

    // 3. Decodificar Dados
    FILE *outFile = fopen(outputFileName, "wb");
    if (!outFile) {
        printf("Erro ao criar arquivo de saida: %s\n", outputFileName);
        fclose(inFile);
        return 1;
    }

    Node* current = root;
    unsigned char buffer;
    long bytesWritten = 0;

    // Le byte a byte e percorre a arvore bit a bit
    while (fread(&buffer, 1, 1, inFile) && bytesWritten < originalSize) {
        for (int i = 7; i >= 0; i--) {
            int bit = (buffer >> i) & 1;
            
            if (bit == 0)
                current = current->left;
            else
                current = current->right;

            // Encontrou uma folha (caractere)
            if (!current->left && !current->right) {
                fwrite(&(current->character), 1, 1, outFile);
                bytesWritten++;
                current = root; // Reseta para o topo da arvore
                
                if (bytesWritten == originalSize) break;
            }
        }
    }

    printf("\n=== Resultado da Descompressao ===\n");
    printf("Arquivo gerado: %s\n", outputFileName);
    printf("Tamanho:        %ld bytes\n", bytesWritten);
    printf("Status:         Sucesso.\n");
    printf("==================================\n");

    fclose(inFile);
    fclose(outFile);
    return 0;
}