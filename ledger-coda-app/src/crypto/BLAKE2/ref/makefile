CC=gcc
CFLAGS=-O2 -I../testvectors -Wall -Wextra -std=c89 -pedantic -Wno-long-long
BLAKEBINS=blake2s 

all:		$(BLAKEBINS) check

blake2s:	blake2s-ref.c
		$(CC) blake2s-ref.c -o $@ $(CFLAGS) -DBLAKE2S_SELFTEST

check:          blake2s 
	        ./blake2s

kat:
		$(CC) $(CFLAGS) -o genkat-c genkat-c.c blake2s-ref.c 
		$(CC) $(CFLAGS) -o genkat-json genkat-json.c blake2s-ref.c
		./genkat-c > blake2-kat.h
		./genkat-json > blake2-kat.json

clean:
		rm -rf *.o genkat-c genkat-json blake2-kat.h blake2-kat.json $(BLAKEBINS)
