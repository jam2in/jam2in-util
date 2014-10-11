CC = gcc
CFLAGS = -Wall -Werror -g
LFLAGS =

SRC = gethostname.c
OBJ = $(SRC:.c=.o)

all: gethostname

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.c
	$(CC) $(CFLAGS) -MM -o $@ $<

clean:
	rm -f $(OBJ) gethostname $(SRC:.c=.d)
