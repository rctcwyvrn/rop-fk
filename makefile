CC=gcc
CFLAGS="-fno-stack-protector" "-no-pie" "-lm"

rop: clean
	$(CC) -g -o rop-fk *.c $(CFLAGS)

clean:
	rm -f rop-fk
