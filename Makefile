CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2
LDFLAGS = -lm

all: integral sort ipc

integral: integral.c
	$(CC) $(CFLAGS) -o integral integral.c $(LDFLAGS)

sort: sort.c
	$(CC) $(CFLAGS) -o sort sort.c

ipc: ipc.c
	$(CC) $(CFLAGS) -o ipc ipc.c

clean:
	rm -f integral sort ipc
