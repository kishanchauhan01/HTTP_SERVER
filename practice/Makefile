CC = gcc
CFLAGS = -Wall -g

SRC = $(wildcard *.c)
OUT = $(SRC:.c=.out)

all: $(OUT)

%.out: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.out
