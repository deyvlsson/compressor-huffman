#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 256

// --- Estruturas e Funções de Huffman ---

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

typedef struct {
    char *code;
    int length;
} HuffmanCode;

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

    if (minHeap->size == 0) return NULL; // Arquivo vazio

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

void generateCodes(Node* root, char* code, int top, HuffmanCode* huffmanCodes) {
    if (root->left) {
        code[top] = '0';
        generateCodes(root->left, code, top + 1, huffmanCodes);
    }
    if (root->right) {
        code[top] = '1';
        generateCodes(root->right, code, top + 1, huffmanCodes);
    }
    if (!root->left && !root->right) {
        code[top] = '\0';
        huffmanCodes[root->character].code = (char*)malloc((top + 1) * sizeof(char));
        strcpy(huffmanCodes[root->character].code, code);
        huffmanCodes[root->character].length = top;
    }
}

void writeBit(FILE *out, int bit, unsigned char *buffer, int *bitCount) {
    if (bit)
        *buffer |= (1 << (7 - *bitCount));
    
    (*bitCount)++;
    
    if (*bitCount == 8) {
        fwrite(buffer, 1, 1, out);
        *buffer = 0;
        *bitCount = 0;
    }
}

// --- Função Principal ---

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_entrada.tex>\n", argv[0]);
        return 1;
    }

    char *inputFileName = argv[1];
    char outputFileName[256];
    
    // Cria o nome do arquivo de saida (ex: animacao.tex -> animacao.huff)
    // Uma abordagem simples: remover a extensao .tex se existir e adicionar .huff
    char *dot = strrchr(inputFileName, '.');
    if (dot && strcmp(dot, ".tex") == 0) {
        int baseLen = dot - inputFileName;
        strncpy(outputFileName, inputFileName, baseLen);
        outputFileName[baseLen] = '\0';
        strcat(outputFileName, ".huff");
    } else {
        sprintf(outputFileName, "%s.huff", inputFileName);
    }

    FILE *inFile = fopen(inputFileName, "rb");
    if (!inFile) {
        printf("Erro ao abrir arquivo de entrada: %s\n", inputFileName);
        return 1;
    }

    // 1. Estatísticas Iniciais e Contagem
    unsigned int frequencies[MAX_SYMBOLS] = {0};
    unsigned char buffer;
    long originalSize = 0;
    
    while (fread(&buffer, 1, 1, inFile)) {
        frequencies[buffer]++;
        originalSize++;
    }
    rewind(inFile);

    if (originalSize == 0) {
        printf("Arquivo vazio. Nada a comprimir.\n");
        fclose(inFile);
        return 0;
    }

    // 2. Construção da Árvore e Códigos
    Node* root = buildHuffmanTree(frequencies);
    HuffmanCode huffmanCodes[MAX_SYMBOLS];
    char tempCode[MAX_SYMBOLS];
    for(int i=0; i<MAX_SYMBOLS; i++) huffmanCodes[i].code = NULL;
    
    generateCodes(root, tempCode, 0, huffmanCodes);

    // 3. Gravação do Arquivo Comprimido
    FILE *outFile = fopen(outputFileName, "wb");
    if (!outFile) {
        printf("Erro ao criar arquivo de saida: %s\n", outputFileName);
        fclose(inFile);
        return 1;
    }

    // Cabeçalho: Tamanho Original (long) + Tabela de Frequências (256 * int)
    fwrite(&originalSize, sizeof(long), 1, outFile);
    fwrite(frequencies, sizeof(unsigned int), MAX_SYMBOLS, outFile);

    // Corpo: Bits
    unsigned char bitBuffer = 0;
    int bitCount = 0;
    
    while (fread(&buffer, 1, 1, inFile)) {
        char *code = huffmanCodes[buffer].code;
        if(code) {
            for (int i = 0; code[i] != '\0'; i++) {
                writeBit(outFile, code[i] - '0', &bitBuffer, &bitCount);
            }
        }
    }
    // Escrever bits remanescentes
    if (bitCount > 0) {
        fwrite(&bitBuffer, 1, 1, outFile);
    }

    // 4. Relatório Final
    long compressedSize = ftell(outFile);
    float taxa = (1.0 - ((float)compressedSize / originalSize)) * 100.0;

    printf("\n=== Resultado da Compressao ===\n");
    printf("Entrada:  %s (%ld bytes)\n", inputFileName, originalSize);
    printf("Saida:    %s (%ld bytes)\n", outputFileName, compressedSize);
    printf("Taxa de Compressao: %.2f%%\n", taxa);
    printf("===============================\n");

    fclose(inFile);
    fclose(outFile);
    return 0;
}