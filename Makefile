all: mydd

mydd: myddlfs.c
	gcc -Wall -Wextra -O2 -march=native -D_GNU_SOURCE $^ -o $@ -lpthread

clean:
	rm mydd
