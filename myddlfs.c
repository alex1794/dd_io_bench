#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>


double elapsed(struct timespec start, struct timespec stop)
{
    return (double)stop.tv_sec - (double)start.tv_sec + ( (double)stop.tv_nsec - (double)start.tv_nsec )*1e-9;
}

void write_file(char *filename, unsigned int bs, unsigned int count)
{
	struct timespec start;
    struct timespec stop;
    double filesize = (double)bs*count*1e-9; // Gigabyte
	char *buf = malloc(sizeof(char) * bs);
    for(int i = 0; i < bs; ++i)
        buf[i] = rand() % 100;	

	int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if(!fd)
	{
		perror("open fd");
		exit(EXIT_FAILURE);
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for(int i = 0; i < count; ++i)
		write(fd, buf, bs);
	fsync(fd);
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);

	double wrtime = elapsed(start, stop);
    double wrbw = filesize / wrtime;
	printf("Write of %.2lf GB file : %lf seconds, %lf GB/s\n", filesize, wrtime, wrbw);

    free(buf);
	
	//clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	close(fd);
	//clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
	//printf("Close time of %s : %lf seconds\n", filename, elapsed(start, stop));
}

void read_file(char *filename, unsigned int bs)
{
	struct timespec start;
    struct timespec stop;
	char *buf = malloc(sizeof(char) * bs);

	int fd = open(filename, O_RDONLY);
	if(!fd)
	{
		perror("open fd");
		exit(EXIT_FAILURE);
	}

    off_t filesize = lseek(fd, 0, SEEK_END);
    printf("Filesize : %lu bytes\n", filesize);
    lseek(fd, 0, SEEK_SET);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for(off_t i = 0; i < filesize; i+=bs)
        read(fd, buf, bs);
	fsync(fd);
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
    //printf("%x\n", buf[0]);

	double rdtime = elapsed(start, stop);
    double rdbw = (double)filesize * 1e-9 / rdtime;
	printf("Read of %.2lf GB file : %lf seconds, %lf GB/s\n", (double)filesize*1e-9, rdtime, rdbw);

    free(buf);
	
	//clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	close(fd);
	//clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
	//printf("Close time of %s : %lf seconds\n", filename, elapsed(start, stop));
       
}    

int main(int argc, char **argv)
{
    srand(getpid());

    int opt;
	char filename[50];
	unsigned int bs = 512;
    unsigned int count = 1;
    char mode = 'w';

	while((opt = getopt(argc, argv, "m:b:c:")) != -1)
	{
		switch(opt) {
			case 'm' :
				if(sscanf(optarg, "%c", &mode) != 1)
				{
					fprintf(stderr, "%s: bad mode\n", optarg);
					return 1;
				}
				break;
			case 'b' :
				if(sscanf(optarg, "%u", &bs) != 1)
				{
					fprintf(stderr, "%s: bad block size\n", optarg);
					return 1;
				}
				break;
			case 'c':
				if(sscanf(optarg, "%u", &count) != 1)
				{
					fprintf(stderr, "%s: bad number of block", optarg);
					return 1;
				}
				break;
			default:
				fprintf(stderr, "Usage: %s [-m mode] [-b blocksize] [-c number of block] filename\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

    argc -= optind;
    argv += optind;
	
    if(argc > 0)
    {
        if(argc != 1)
		    return printf("Usage: ./mydd [-m mode] [-b blocksize] [-c number of block] filename\n"), 1;

        if(sscanf(argv[0], "%s", &filename) != 1)
		{
			fprintf(stderr, "%s: bad filename\n", argv[0]);
			return 1;
		}
    }
    else
	    return printf("Usage: ./mydd [-m mode] [-b blocksize] [-c number of block] filename\n"), 1;


	printf("Parameters : %c - %d - %d - %s\n\n", mode, bs, count, filename);

#ifndef BENCH
    if(mode == 'w')
        write_file(filename, bs, count);
    else if (mode == 'r')
        read_file(filename, bs);
    else
    {
        fprintf(stderr, "Bad mode : read mode (r) and write mode (w)\n");
        exit(EXIT_FAILURE);
    }
#else
    if(mode == 'w')
        for(int i = 0; i < 10; ++i)
            write_file(filename, bs, count);
    else if (mode == 'r')
        for(int i = 0; i < 10; ++i)
            read_file(filename, bs);
    else
    {
        fprintf(stderr, "Bad mode : read mode (r) and write mode (w)\n");
        exit(EXIT_FAILURE);
    }
#endif

	return 0;
}
