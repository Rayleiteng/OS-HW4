#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#define RECORD_SIZE 100
#define KEY_SIZE 4

typedef struct
{
	uint32_t	key;
	char		data[RECORD_SIZE - KEY_SIZE];
}				Record;

int	main(int argc, char **argv)
{
	if (argc != 3){
		fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
		exit(1);
	}
	char *input_file = argv[1];
	char *output_file = argv[2];

	int fd_in = open(input_file, O_RDONLY);
	if (fd_in < 0){
		perror("Error opening input file");
		exit(1);
	}

	struct stat fd_in_stat;
	if (fstat(fd_in, &fd_in_stat) < 0){
        perror("Error getting file stats");
        exit(1);
	}

    size_t size = fd_in_stat.st_size;

    if(size % RECORD_SIZE != 0){
        perror("Error  inpout file should have a size as a multiple of 100");
        exit(1);
    }

    if (mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd_in, 0) == MAP_FAILED) {
        perror("Error mapping input file");
        exit(1);
    }
    printf("Input file mapped successfully! Total records: %ld\n", size / RECORD_SIZE);

    int fd_out = open(output_file, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0){
		perror("Error opening ouput file");
		exit(1);
	}

    if (ftruncate(fd_out, size) == -1) {
        perror("Error analyzing output file size");
        exit(1);
    }

    if (mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_out, 0) == MAP_FAILED) {
        perror("Error mapping input file");
        exit(1);
    }
    printf("Output file mapped and resized successfully!\n");

    // fsync(fd_out);
    close(fd_in);
    close(fd_out);

    return 0;
}