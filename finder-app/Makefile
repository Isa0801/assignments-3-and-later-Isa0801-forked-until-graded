

GCC=${CROSS_COMPILE}gcc


default: writer

writer: writer.c
	${GCC} writer.c -o writer

clean:
	rm -f writer *.o

print_gcc:
	${GCC} -print-sysroot -v
