#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLORS 10000
#define MAX_CODE_LENGTH 5

#define ENCODED_PATH "image_encoded_R.txt"
#define DECODED_Huffman_PATH "image_decoded_H.txt"
#define OUTPUT_PATH "Q2_decoded.ppm"

typedef struct {
    char huffman_code[MAX_CODE_LENGTH + 1];
    int r, g, b;
} color;

color colors[MAX_COLORS];
int color_count = 0;

color* find_color(char* code) {
    for(int i = 0; i < color_count; ++i) {
        if(strcmp(colors[i].huffman_code, code) == 0) {
            return &colors[i];
        }
    }
    return NULL;
}

long getFileSize(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("File not found or unable to open the file\n");
        return -1;
    }

    fseek(file, 0, SEEK_END); // seek to end of file
    long size = ftell(file); // get current file pointer

    fclose(file);
    return size;
}

void decode_image_Huffman(char *path) {
    FILE* file = fopen(path, "r");
    int width, height;
    fscanf(file, "%d %d\n", &width, &height);

    char line[256];

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strcmp(line, "-\n") == 0) {
            break;
        }

        color c;
        sscanf(line, "%d %d %d %s", &c.r, &c.g, &c.b, c.huffman_code);
        colors[color_count++] = c;
    }

    FILE* output = fopen(OUTPUT_PATH, "w");
    fprintf(output, "P3\n%d %d\n255\n", width, height);

    char code[MAX_CODE_LENGTH + 1] = {0};
    int code_length = 0;
    char ch;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == ' ' || ch == '\n') {
            continue;
        }

        code[code_length++] = ch;
        code[code_length] = '\0';

        color* c = find_color(code);

        if (c) {
            fprintf(output, "%d %d %d\n", c->r, c->g, c->b);
            code_length = 0;
        }
    }
    fclose(file);
    fclose(output);
}

void decode_image_RL(char *path) {
    FILE *file = fopen(path, "r");
    FILE *outfile = fopen(DECODED_Huffman_PATH, "w");

    if (file == NULL || outfile == NULL) {
        printf("Error opening file!\n");
        return;
    }

    int value, count;
    char line[256];

    // Write header to output file
    while (fgets(line, sizeof(line), file)) {
        if (strcmp(line, "-\n") == 0) {
            fprintf(outfile, "%s", line);
            break;
        }
        fprintf(outfile, "%s", line);
    }

    // Decompress rest of file
    while (fscanf(file, "%d %d", &value, &count) != EOF) {
        for (int i = 0; i < count; i++) {
            fprintf(outfile, "%d\n", value);
        }
    }

    fclose(file);
    fclose(outfile);
}

int main() {

    decode_image_RL(ENCODED_PATH);
    decode_image_Huffman(DECODED_Huffman_PATH);
    
    long size = getFileSize(OUTPUT_PATH);
    printf("The encoded file(using Huffman&RL coding) is: %ld bytes\n", size);
    return 0;
}
