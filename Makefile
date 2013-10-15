
CC = gcc

srcs := $(wildcard *.c)
objs := $(srcs:.c=.o)

all: jpegc

jpegc: $(objs)
	$(CC) -o $@ $(objs) -lm `pkg-config --cflags --libs gtk+-2.0`

$(objs): %.o: %.c
	$(CC) -c $< -o $@ `pkg-config --cflags --libs gtk+-2.0`


.PHONY: clean
clean:
	@rm jpegc $(objs)
