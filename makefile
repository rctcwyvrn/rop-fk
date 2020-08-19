CC=gcc
CFLAGS= "-g" "-fno-stack-protector" "-no-pie" "-lm"

rop: clean
	$(CC) -o rop-fk *.c $(CFLAGS)

clean:
	rm -f rop-fk
