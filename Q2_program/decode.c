#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLORS 10000
#define MAX_CODE_LENGTH 256

#define ENCODED_PATH "image_encode_step2(RL).txt"
#define DECODED_Huffman_PATH "image_decode_step1(Huff).txt"
#define OUTPUT_PATH "image_decoded.ppm"

typedef struct {
    char huffman_code[MAX_CODE_LENGTH];
    int r, g, b;
} color;

color colors[MAX_COLORS];
int color_count = 0;

//get the color code for huffman code
color* find_color(char* code) {
    for(int i = 0; i < color_count; ++i) {
        if(strcmp(colors[i].huffman_code, code) == 0) {
            return &colors[i];
        }
    }
    return NULL;
}

//get the file size using the path
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
    //open the encoded image
    FILE* file = fopen(path, "r");
    int width, height;
    //read the header for image size
    fscanf(file, "%d %d\n", &width, &height);

    char line[256];
    //read the header for (decode)color-index 
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strcmp(line, "-\n") == 0) {
            break;
        }

        color c;
        sscanf(line, "%d %d %d %s", &c.r, &c.g, &c.b, c.huffman_code);
        colors[color_count++] = c;
    }

    //write the ppm P3 header(size and the P3)
    FILE* output = fopen(OUTPUT_PATH, "w");
    fprintf(output, "P3\n%d %d\n255\n", width, height);

    char code[MAX_CODE_LENGTH + 1] = {0};
    int code_length = 0;
    char ch;

    //write the pixel information
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
    //open the fully encoded file
    FILE *file = fopen(path, "r");
    FILE *outfile = fopen(DECODED_Huffman_PATH, "w");

    if (file == NULL || outfile == NULL) {
        printf("Error opening file!\n");
        return;
    }

    int count;
    char line[256], value[MAX_CODE_LENGTH];

    // Write header to output file
    while (fgets(line, sizeof(line), file)) {
        if (strcmp(line, "-\n") == 0) {
            fprintf(outfile, "%s", line);
            break;
        }
        fprintf(outfile, "%s", line);
    }

    // Decompress rest of file
    while (fscanf(file, "%s %d", value, &count) != EOF) {
        for (int i = 0; i < count; i++) {
            fprintf(outfile, "%s\n", value);
        }
    }

    fclose(file);
    fclose(outfile);
}

int main() {
    //First decode using the RL method
    decode_image_RL(ENCODED_PATH);
    //then decode using the Huffman method
    decode_image_Huffman(DECODED_Huffman_PATH);
    
    long size = getFileSize(OUTPUT_PATH);
    printf("The decoded file(using Huffman&RL coding) is: %ld bytes\n", size);
    return 0;
}
