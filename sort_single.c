#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define RECORD_SIZE 128
#define KEY_SIZE 4

typedef struct
{
	uint32_t	key;
	char		data[RECORD_SIZE - KEY_SIZE];
}				Record;

int my_compare(const void *a, const void *b){
    Record *r1 = (Record *)a;
    Record *r2 = (Record *)b;
    if(r1->key < r2->key)
        return -1;
    if(r1->key > r2->key)
        return 1;
    return 0;
}

int	main(int argc, char **argv)
{
	if (argc != 3){
		fprintf(stderr, "Usage: %s input_file output_file.\n", argv[0]);
		exit(1);
	}
	char *input_file = argv[1];
	char *output_file = argv[2];

	int fd_in = open(input_file, O_RDONLY);
	if (fd_in < 0){
		perror("Error opening input file.");
		exit(1);
	}

	struct stat fd_in_stat;
	if (fstat(fd_in, &fd_in_stat) < 0){
        perror("Error getting file stats.");
        exit(1);
	}

    size_t file_size = fd_in_stat.st_size;

    if(file_size % RECORD_SIZE != 0){
        perror("Error  inpout file should have a size as a multiple of 100.");
        exit(1);
    }

    size_t num_records = file_size / RECORD_SIZE;

    Record *input_map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd_in, 0);
    if (input_map == MAP_FAILED) {
        perror("Error mapping input file.");
        exit(1);
    }
    printf("Input file mapped successfully! Total records: %ld\n", file_size / RECORD_SIZE);

    int fd_out = open(output_file, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0){
		perror("Error opening ouput file.");
		exit(1);
	}

    if (ftruncate(fd_out, file_size) == -1) {
        perror("Error analyzing output file size.");
        exit(1);
    }

    Record *output_map = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_out, 0);

    if ( output_map == MAP_FAILED) {
        perror("Error mapping input file");
        exit(1);
    }
    printf("Output file mapped and resized successfully!\n");

	Record *buffer = malloc(file_size);
	if (!buffer)
	{
		perror("malloc"); 
		exit(1);
	}

	memcpy(buffer, input_map, file_size); //copy to buffer for sorting

	qsort(buffer, num_records, RECORD_SIZE, my_compare);

	memcpy(output_map, buffer, file_size);

	printf("Sorting complete. Syncing to disk...\n");

	fsync(fd_out);

	munmap(input_map, file_size);
	munmap(output_map, file_size);
	close(fd_in);
	close(fd_out);
	free(buffer);

	return (0);
}