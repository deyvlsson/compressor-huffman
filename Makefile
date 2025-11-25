CC = gcc
CFLAGS = -Wall -Wextra -O2

all: compressor descompressor

compressor: compressor.c
	$(CC) $(CFLAGS) compressor.c -o compressor

descompressor: descompressor.c
	$(CC) $(CFLAGS) descompressor.c -o descompressor

clean:
	rm -f compressor descompressor *.huff *_restaurado.tex