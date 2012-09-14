EXES = $(basename $(wildcard *.c))
CC = cc
CFLAGS = -O3 -march=native  `pkg-config --cflags --libs gtk+-3.0`

all: 
	$(MAKE) $(EXES)

%: %.c
	$(CC) $(CFLAGS) $@.c -o $@

clean:
	rm -f $(EXES) *.o
