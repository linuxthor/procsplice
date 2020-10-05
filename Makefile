CC=gcc
CFLAGS=

procsplice: procsplice.o 
	$(CC) -o procsplice procsplice.o 

