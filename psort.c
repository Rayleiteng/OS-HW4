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

    Record *input_map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd_in, 0);
    if (input_map == MAP_FAILED) {
        perror("Error mapping input file");
        exit(1);
    }
printf("[Step 2 Success] Input file mapped! Total records: %ld\n", size / RECORD_SIZE);

    // ==========================================
    // 4. 处理输出文件 (Output File)
    // ==========================================
    
    // 打开/创建输出文件
    // O_RDWR: 读写模式 (mmap 需要读写权限)
    // O_CREAT: 如果文件不存在就创建
    // O_TRUNC: 如果文件存在就清空它
    // 0666: 文件权限
    int fd_out = open(output_file, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0) {
        perror("Error opening output file");
        exit(1);
    }

    // ★ 关键细节：扩展文件大小
    // 新创建的文件大小是 0。如果你直接 map 0 字节的文件然后往里写数据，程序会崩 (SIGBUS 错误)。
    // 我们必须先用 ftruncate 把文件“撑大”到和输入文件一样大。
    if (ftruncate(fd_out, size) == -1) {
        perror("Error analyzing output file size");
        exit(1);
    }

    // ★ mmap 输出文件
    // PROT_READ | PROT_WRITE: 我们需要写入排序后的数据
    // MAP_SHARED: ★这是必须的★。SHARED 意味着我们的修改会写回硬盘文件，而不是只留在内存里。
    Record *output_map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_out, 0);
    if (output_map == MAP_FAILED) {
        perror("Error mapping output file");
        exit(1);
    }
    
    printf("[Step 2 Success] Output file mapped and resized!\n");

    // ==========================================
    // 5. 收尾工作 (Cleanup)
    // ==========================================
    
    // 解除映射 (虽然程序结束会自动解除，但好习惯是手动做)
    munmap(input_map, size);
    munmap(output_map, size);
    
    // 关闭文件描述符
    close(fd_in);
    close(fd_out);

    return 0;
}