

GCC=${CROSS_COMPILE}gcc


default: aesdsocket

all: aesdsocket

aesdsocket: aesdsocket.c
	${GCC} aesdsocket.c -o aesdsocket

clean:
	rm -f aesdsocket *.o

print_gcc:
	${GCC} -print-sysroot -v
