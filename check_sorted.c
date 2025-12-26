#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdint.h>

#define RECORD_SIZE 128

typedef struct {
    uint32_t key;
    char data[RECORD_SIZE - 4];
} Record;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_to_check>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }

    struct stat st;
    fstat(fd, &st);
    size_t file_size = st.st_size;
    
    if (file_size % RECORD_SIZE != 0) {
        fprintf(stderr, "Error: File size is not a multiple of record size.\n");
        close(fd);
        return 1;
    }

    size_t num_records = file_size / RECORD_SIZE;

    Record *map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    for (size_t i = 0; i < num_records - 1; i++) {
        if (map[i].key > map[i+1].key) {
            fprintf(stderr, "Error: File is NOT sorted at index %ld. Key %u > Key %u\n", 
                    i, map[i].key, map[i+1].key);
            munmap(map, file_size);
            close(fd);
            return 1;
        }
    }

    printf("Success: File is sorted correctly. Total records: %ld\n", num_records);

    munmap(map, file_size);
    close(fd);
    return 0;
}