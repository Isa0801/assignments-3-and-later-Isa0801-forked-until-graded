
CC=${CROSS_COMPILE}gcc

CFLAGS ?= -lpthread -g

default: aesdsocket

all: aesdsocket

aesdsocket: aesdsocket.c
	${CC} aesdsocket.c -o aesdsocket ${CFLAGS}

clean:
	rm -f aesdsocket *.o

print_gcc:
	${CC} -print-sysroot -v
