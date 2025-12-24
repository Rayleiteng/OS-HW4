#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define RECORD_SIZE 100

void random_record(FILE *f) {
    uint32_t key = rand();  // random 4-byte key
    fwrite(&key, sizeof(uint32_t), 1, f);
    for (int i = 4; i < RECORD_SIZE; i++)
        fputc('A' + (rand() % 26), f);  // random letters
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s output_file num_records\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    int n = atoi(argv[2]);
    srand(time(NULL));
    for (int i = 0; i < n; i++)
        random_record(f);

    fclose(f);
    return 0;
}

