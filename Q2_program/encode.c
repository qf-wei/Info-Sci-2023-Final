#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLORS 100000
#define COLOR_LENGTH 12  //RGB,(3+1)*3
#define MAX_TREE_HT 100
#define MAX_CODE_LENGTH 256

#define FILE_PATH "further_test.ppm"
#define ENCODED_PATH "image_encode_step1(Huff).txt"
#define ENCODED_PATH_R "image_encode_step2(RL).txt"

typedef struct {
    char color[COLOR_LENGTH]; 
    int freq;
} Color;

typedef struct Node {
    char color[COLOR_LENGTH];
    unsigned freq;
    struct Node *left, *right;
} Node;

typedef struct MinHeap {
    unsigned size;
    unsigned capacity;
    Node **array;
} MinHeap;

typedef struct {
    char color[COLOR_LENGTH];
    char huffmanCode[MAX_CODE_LENGTH];
} ColorCode;

Color colors[MAX_COLORS];
int totalColors = 0;

//get the file size using the path
long getFileSize(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("File not found.\n");
        return -1;
    }

    fseek(file, 0, SEEK_END); // seek to end of file
    long size = ftell(file); // get current file pointer

    fclose(file);
    return size;
}

int findColorIndex(char* color) {
    for (int i = 0; i < totalColors; i++) {
        if (strcmp(colors[i].color, color) == 0) {
            return i;
        }
    }
    return -1;
}

void addColor(char* color) {
    int index = findColorIndex(color);
    if (index != -1) {
        colors[index].freq++;
    } else {
        strcpy(colors[totalColors].color, color);
        colors[totalColors].freq = 1;
        totalColors++;
    }
}


Node* newNode(char* color, unsigned freq) {
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->left = temp->right = NULL;
    temp->freq = freq;
    strcpy(temp->color, color);
    return temp;
}

MinHeap* createMinHeap(unsigned capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (Node**)malloc(minHeap->capacity * sizeof(Node*));
    return minHeap;
}


//swaps two nodes
void swapNode(Node** a, Node** b) {
    Node* t = *a;
    *a = *b;
    *b = t;
}

//ensure the Minimum Heap property is not violated
void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

int isSizeOne(MinHeap* minHeap) {
    return (minHeap->size == 1);
}

//remove the smallest node from the MinHeap
Node* extractMin(MinHeap* minHeap) {
    Node* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

//insert node into the MinHeap.
void insertMinHeap(MinHeap* minHeap, Node* node) {
    ++minHeap->size;
    int i = minHeap->size - 1;
    while (i && node->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = node;
}

//construct the MinHeap from given array of Nodes
void buildMinHeap(MinHeap* minHeap) {
    int n = minHeap->size - 1;
    int i;
    for (i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
}


int isLeaf(Node* root) {
    return !(root->left) && !(root->right);
}

//create MinHeap with given color frequencie
MinHeap* createAndBuildMinHeap(char color[][12], int freq[], int size) {
    MinHeap* minHeap = createMinHeap(size);
    for (int i = 0; i < size; ++i)
        minHeap->array[i] = newNode(color[i], freq[i]);
    minHeap->size = size;
    buildMinHeap(minHeap);
    return minHeap;
}

//build the Huffman Tree
Node* buildHuffmanTree(char color[][12], int freq[], int size) {
    Node *left, *right, *top;
    MinHeap* minHeap = createAndBuildMinHeap(color, freq, size);
    while (!isSizeOne(minHeap)) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);
        top = newNode("dummy", left->freq + right->freq);
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);
    }
    return extractMin(minHeap);
}

//creates Huffman code
void getHuffmanCodes(Node* root, int arr[], int top, ColorCode colorCodes[], int *codeCount) {
    if (root->left) {
        arr[top] = 0;
        getHuffmanCodes(root->left, arr, top + 1, colorCodes, codeCount);
    }

    if (root->right) {
        arr[top] = 1;
        getHuffmanCodes(root->right, arr, top + 1, colorCodes, codeCount);
    }

    if (isLeaf(root)) {
        char huffmanCode[MAX_CODE_LENGTH] = "";
        for (int i = 0; i < top; ++i) {
            char bit[2];
            sprintf(bit, "%d", arr[i]);
            strcat(huffmanCode, bit);
        }
        strcpy(colorCodes[*codeCount].color, root->color);
        strcpy(colorCodes[*codeCount].huffmanCode, huffmanCode);
        (*codeCount)++;
    }
}

void HuffmanEncode(char color[][12], int freq[], int size) {
    Node* root = buildHuffmanTree(color, freq, size);
    int arr[MAX_TREE_HT], top = 0;

    ColorCode colorCodes[size];
    int codeCount = 0;
    getHuffmanCodes(root, arr, top, colorCodes, &codeCount);

    FILE *fp = fopen(FILE_PATH, "r");
    FILE *out = fopen(ENCODED_PATH, "w");


    char line[128];
    int width, height;
    fgets(line, sizeof(line), fp); // Skip the magic number
    fgets(line, sizeof(line), fp); // Skip the comment
    fgets(line, sizeof(line), fp); // Skip the dimensions
    sscanf(line, "%d %d", &width, &height); // Parse width and height
    fgets(line, sizeof(line), fp); // Skip the max color value

    // Write image size info
    fprintf(out, "%d %d\n", width, height);
    
    // Write Huffman codes
    for (int i = 0; i < codeCount; i++) {
        fprintf(out, "%s %s\n", colorCodes[i].color, colorCodes[i].huffmanCode);
    }

    // Write a delimiter line
    fprintf(out, "-\n");

    int r, g, b;
    int j=0;
    while (fscanf(fp, "%d %d %d", &r, &g, &b) == 3) {
        char currentColor[12];
        sprintf(currentColor, "%d %d %d", r, g, b);
        for (int i = 0; i < codeCount; i++) {
            if (strcmp(currentColor, colorCodes[i].color) == 0) {
                fprintf(out, "%s\n", colorCodes[i].huffmanCode);
                break;
            }
        }
    }

    fclose(fp);
    fclose(out);
}

void rlEncode() {
    FILE *fp = fopen(ENCODED_PATH, "r");
    FILE *outfile = fopen(ENCODED_PATH_R, "w");

    if (fp == NULL || outfile == NULL) {
        printf("Error opening file!\n");
        return;
    }

    int count;
    char line[MAX_CODE_LENGTH], current[MAX_CODE_LENGTH], next[MAX_CODE_LENGTH];

    // Write header to output file
    while (fgets(line, sizeof(line), fp)) {
        if (strcmp(line, "-\n") == 0) {
            fprintf(outfile, "%s", line);
            break;
        }
        fprintf(outfile, "%s", line);
    }

    // Initialize count and current value
    fscanf(fp, "%s", current);
    count = 1;

    while (fscanf(fp, "%s", next) != EOF) {
        if (strcmp(next,current)==0) {
            count++;
        } else {
            fprintf(outfile, "%s %d\n", current, count);
            strcpy(current, next);
            count = 1;
        }
    }

    // Write the last group
    fprintf(outfile, "%s %d\n", current, count);

    fclose(fp);
    fclose(outfile);
}


int main() {

    //1. read the input file
    FILE *fp = fopen(FILE_PATH, "r"); //Open the Q2.ppm
    if (fp == NULL) {
        printf("Error opening file\n");
        return -1;
    }

    long size_ori = getFileSize(FILE_PATH); //The size of original Q2.ppm
    printf("The original ppm file is: %ld bytes\n", size_ori);

    char line[128];
    fgets(line, sizeof(line), fp); // Skip the magic number
    fgets(line, sizeof(line), fp); // Skip the dimensions
    fgets(line, sizeof(line), fp); // Skip the comments
    fgets(line, sizeof(line), fp); // Skip the max color value

    //2. get every different colors
    int r, g, b;  //get 
    char colorStr[COLOR_LENGTH];
    while (fscanf(fp, "%d %d %d", &r, &g, &b) == 3) {
        sprintf(colorStr, "%d %d %d", r, g, b);
        addColor(colorStr);
    }

    fclose(fp);

    printf("Total colors: %d\n", totalColors);

    //3. count each color's freq
    char colorsArr[totalColors][12];
    int freqArr[totalColors];
    for (int i = 0; i < totalColors; i++) {
        strcpy(colorsArr[i], colors[i].color);
        freqArr[i] = colors[i].freq;
    }

    //4. make the HuffmanTree using the color and freq
    //5. open the input file again and skip the header
    //6. read each RGB pixel in the input file and get the Huffman code. Then write it into a new file.
    HuffmanEncode(colorsArr, freqArr, totalColors);

    //7. output the Huffman encoded file size
    long size_enc = getFileSize(ENCODED_PATH);
    printf("The encoded file(using Huffman) is: %ld bytes\n", size_enc);
    float rate = (float)size_ori / size_enc;
    printf("(Compression rate: %.4f)\n", rate);

    //8. run the rl encode using the Huffman-encoded file
    rlEncode();

    //9. output the Huffman/RL encoded file size
    long size_enc1 = getFileSize(ENCODED_PATH_R);
    printf("The encoded file(using Huffman+RL) is: %ld bytes\n", size_enc1);
    rate = (float)size_ori / size_enc1;
    printf("(Compression rate: %.4f)\n", rate);

    return 0;
}
