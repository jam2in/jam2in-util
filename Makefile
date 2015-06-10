CC = gcc
CFLAGS = -Wall -Werror -g
LFLAGS =

SRC = gethostname.c checkhashperf.c
OBJ = $(SRC:.c=.o)

all: gethostname checkhashperf

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.c
	$(CC) $(CFLAGS) -MM -o $@ $<

clean:
	rm -f $(OBJ) gethostname checkhashperf $(SRC:.c=.d)
