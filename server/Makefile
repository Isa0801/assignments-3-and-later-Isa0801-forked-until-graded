

CC=${CROSS_COMPILE}gcc


default: aesdsocket

all: aesdsocket

aesdsocket: aesdsocket.c
	${CC} aesdsocket.c -o aesdsocket

clean:
	rm -f aesdsocket *.o

print_gcc:
	${CC} -print-sysroot -v
