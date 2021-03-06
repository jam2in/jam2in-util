CC = gcc
CFLAGS = -Wall -Werror -g
LFLAGS =

SRC = gethostname.c \
      checkhashperf.c \
      checkhashring.c \
      genassochash.c \
      gennodelist.c \
      genarcuslocalconf.c

OBJ = $(SRC:.c=.o)

EXE = gethostname \
      checkhashperf \
      checkhashring \
      genassochash \
      gennodelist \
      genarcuslocalconf

all: $(EXE)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.c
	$(CC) $(CFLAGS) -MM -o $@ $<

clean:
	rm -f $(OBJ) $(EXE) $(SRC:.c=.d)
