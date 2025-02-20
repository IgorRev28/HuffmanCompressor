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

typedef struct {
    unsigned int code;
    unsigned char length;
} HuffmanCode;

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

void GenerateHuffmanCodes(NODE* root, unsigned int code, unsigned char length, HuffmanCode huffmanTable[256]) {
    if (!root) return;
    if (root->isSymb) {
        huffmanTable[root->symb].code = code;
        huffmanTable[root->symb].length = length;
        return;
    }
    GenerateHuffmanCodes(root->left, code << 1, length + 1, huffmanTable);
    GenerateHuffmanCodes(root->right, (code << 1) | 1, length + 1, huffmanTable);
}

void FreeTree(NODE* root) {
    if (!root) return;
    FreeTree(root->left);
    FreeTree(root->right);
    free(root);
}
void CompressFile(const char* inputFile, const char* outputFile) {
    unsigned int freq[256] = { 0 };
    FILE* fr = fopen(inputFile, "rb");
    if (!fr) {
        printf("Error opening input file: %s\n", inputFile);
        return;
    }

    fseek(fr, 0L, SEEK_END);
    long length = ftell(fr);
    fseek(fr, 0, SEEK_SET);

    for (long i = 0; i < length; ++i) {
        freq[(unsigned char)fgetc(fr)]++;
    }
    fclose(fr);
    
    NODE* head = NULL;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            NODE* newNode = (NODE*)calloc(1, sizeof(NODE));
            newNode->symb = (unsigned char)i;
            newNode->isSymb = 1;
            newNode->freq = freq[i];
            head = Add2List(head, newNode);
        }
    }
    
    NODE* root = MakeTreeFromList(head);
    if (!root) {
        printf("oshibka\n");
        return;
    }

    
    HuffmanCode huffmanTable[256] = { 0 };
    GenerateHuffmanCodes(root, 0, 0, huffmanTable);

    fr = fopen(inputFile, "rb");
    FILE* fw = fopen(outputFile, "wb");
    if (!fw) {
        printf("Error opening output file: %s\n", outputFile);
        fclose(fr);
        return;
    }
    unsigned char bitAccumulator = 0;
    int bitCount = 0;

    for (long i = 0; i < length; ++i) {
        unsigned char c = (unsigned char)fgetc(fr);
        unsigned int code = huffmanTable[c].code;
        unsigned char length = huffmanTable[c].length;

        for (int j = length - 1; j >= 0; j--) {
            bitAccumulator = (bitAccumulator << 1) | ((code >> j) & 1);
            bitCount++;

            if (bitCount == BIT8) {
                fputc(bitAccumulator, fw);
                bitAccumulator = 0;
                bitCount = 0;
            }
        }
    }

    if (bitCount > 0) {
        bitAccumulator <<= (BIT8 - bitCount);
        fputc(bitAccumulator, fw);
    }

    fclose(fr);
    fclose(fw);
    FreeTree(root);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }
    CompressFile(argv[1], argv[2]);
    printf("File compressed successfully!\n");
    return 0;
}
