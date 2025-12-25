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

typedef struct{
	uint32_t	key;
	char		data[RECORD_SIZE - KEY_SIZE];
} Record;

typedef struct {
    void *base;
    size_t nmemb;
    int thread_id;
} ThreadArgs;

int my_compare(const void *a, const void *b){
    Record *r1 = (Record *)a;
    Record *r2 = (Record *)b;
    if(r1->key < r2->key)
        return -1;
    if(r1->key > r2->key)
        return 1;
    return 0;
}

void *sort_for_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    
    qsort(args->base, args->nmemb, RECORD_SIZE, my_compare);
    
    return NULL;
}

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

    size_t file_size = fd_in_stat.st_size;

    if(file_size % RECORD_SIZE != 0){
        perror("Error  inpout file should have a size as a multiple of 100");
        exit(1);
    }

    size_t num_records = file_size / RECORD_SIZE;

    Record *input_map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd_in, 0);
    if (input_map == MAP_FAILED) {
        perror("Error mapping input file");
        exit(1);
    }
    printf("Input file mapped successfully! Total records: %ld\n", file_size / RECORD_SIZE);

    int fd_out = open(output_file, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0){
		perror("Error opening ouput file");
		exit(1);
	}

    if (ftruncate(fd_out, file_size) == -1) {
        perror("Error analyzing output file size");
        exit(1);
    }

    Record *output_map = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_out, 0);

    if ( output_map == MAP_FAILED) {
        perror("Error mapping input file");
        exit(1);
    }
    printf("Output file mapped and resized successfully!\n");



    Record *buffer = malloc(file_size);
    memcpy(buffer, input_map, file_size);

    int num_threads = get_nprocs();

    size_t records_per_thread = num_records / num_threads;
    size_t remainder = num_records % num_threads;
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadArgs *args = malloc(num_threads * sizeof(ThreadArgs));

    size_t current_offset = 0;

    for (int i = 0; i < num_threads; i++) {
        size_t count = records_per_thread + (i < remainder ? 1 : 0);
        
        args[i].thread_id = i;
        args[i].base = buffer + current_offset; 
        args[i].nmemb = count;
        
        current_offset += count; 

        if(pthread_create(&threads[i], NULL, sort_for_thread, &args[i])){
            fprintf(stderr, "Error creating thread %d\n", i);
            exit(1);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("[Step 3 Success] All chunks sorted!\n");

    memcpy(output_map, buffer, file_size);

    munmap(input_map, file_size);
    munmap(output_map, file_size);
    close(fd_in);
    close(fd_out);
    free(args);
    free(threads);
    

    return 0;
}