#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CODE_SIZE 256
#define BIT8 8

typedef struct node {
    unsigned char symb;
    unsigned char isSymb;
    unsigned int freq;
    unsigned char code[CODE_SIZE];
    int level;
    struct node *left, *right, *next;
} NODE;

typedef union bit2char {
    char symb;
    struct bit {
        unsigned b1 : 1;
        unsigned b2 : 1;
        unsigned b3 : 1;
        unsigned b4 : 1;
        unsigned b5 : 1;
        unsigned b6 : 1;
        unsigned b7 : 1;
        unsigned b8 : 1;
    } mbit;
} BIT2CHAR;

NODE *Add2List(NODE **head, NODE *newNode) {
    if (!*head || newNode->freq < (*head)->freq) {
        newNode->next = *head;
        *head = newNode;
        return *head;
    }
    NODE *current = *head;
    while (current->next && current->next->freq <= newNode->freq)
        current = current->next;
    newNode->next = current->next;
    current->next = newNode;
    return *head;
}

NODE *MakeNodeFromNode(const NODE *left, const NODE *right) {
    NODE *res = (NODE *)malloc(sizeof(NODE));
    res->freq = left->freq + right->freq;
    res->isSymb = 0;
    res->symb = 0;
    res->left = (NODE *)left;
    res->right = (NODE *)right;
    res->next = NULL;
    return res;
}

NODE *MakeTreeFromList(NODE *head) {
    while (head && head->next) {
        NODE *left = head;
        NODE *right = head->next;
        head = head->next->next;
        head = Add2List(&head, MakeNodeFromNode(left, right));
    }
    return head;
}

void GenerateHuffmanCodes(NODE *root, unsigned char *code, int depth, char huffmanTable[256][CODE_SIZE]) {
    if (!root) return;
    if (root->isSymb) {
        code[depth] = '\0';
        strcpy(huffmanTable[root->symb], (char *)code);
        return;
    }
    code[depth] = '0';
    GenerateHuffmanCodes(root->left, code, depth + 1, huffmanTable);
    code[depth] = '1';
    GenerateHuffmanCodes(root->right, code, depth + 1, huffmanTable);
}

void CompressFile(const char *inputFile, const char *outputFile) {
    unsigned int freq[256] = {0};
    FILE *fr = fopen(inputFile, "rb");
    if (!fr) return;

    fseek(fr, 0L, SEEK_END);
    long length = ftell(fr);
    fseek(fr, 0, SEEK_SET);
    
    for (int i = 0; i < length; ++i) {
        freq[(unsigned char)fgetc(fr)]++;
    }
    fclose(fr);
    
    NODE *head = NULL;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            NODE *newNode = (NODE *)malloc(sizeof(NODE));
            newNode->symb = (unsigned char)i;
            newNode->isSymb = 1;
            newNode->freq = freq[i];
            newNode->left = newNode->right = newNode->next = NULL;
            head = Add2List(&head, newNode);
        }
    }
    
    NODE *root = MakeTreeFromList(head);
    
    char huffmanTable[256][CODE_SIZE] = {0};
    unsigned char code[CODE_SIZE] = {0};
    GenerateHuffmanCodes(root, code, 0, huffmanTable);
    
    fr = fopen(inputFile, "rb");
    FILE *fw = fopen(outputFile, "wb");
    if (!fw) return;
    
    BIT2CHAR symb;
    char bitString[length * CODE_SIZE];
    bitString[0] = '\0';
    
    for (int i = 0; i < length; ++i) {
        strcat(bitString, huffmanTable[(unsigned char)fgetc(fr)]);
    }
    fclose(fr);
    
    int bitLen = strlen(bitString);
    int byteCount = bitLen / BIT8;
    int tail = bitLen % BIT8;
    fwrite(&tail, sizeof(int), 1, fw);
    fwrite(&byteCount, sizeof(int), 1, fw);
    
    for (int i = 0; i < byteCount; ++i) {
        symb.mbit.b1 = bitString[i * BIT8 + 0] - '0';
        symb.mbit.b2 = bitString[i * BIT8 + 1] - '0';
        symb.mbit.b3 = bitString[i * BIT8 + 2] - '0';
        symb.mbit.b4 = bitString[i * BIT8 + 3] - '0';
        symb.mbit.b5 = bitString[i * BIT8 + 4] - '0';
        symb.mbit.b6 = bitString[i * BIT8 + 5] - '0';
        symb.mbit.b7 = bitString[i * BIT8 + 6] - '0';
        symb.mbit.b8 = bitString[i * BIT8 + 7] - '0';
        fwrite(&symb.symb, sizeof(char), 1, fw);
    }
    fclose(fw);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }
    CompressFile(argv[1], argv[2]);
    printf("File compressed successfully!\n");
    return 0;
}