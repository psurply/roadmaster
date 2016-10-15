##
## Makefile for Roadmaster
##
## Made by Pierre Surply
## <pierre.surply@gmail.com>
##
## Started on  Sat Dec 28 17:54:22 2013 Pierre Surply
## Last update Mon Dec 30 18:07:52 2013 Pierre Surply
##

CC	= arm-linux-gcc
CFLAGS	= -std=c99

EXEC	= test

SRC	= main.c
OBJ	= $(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

upload: $(EXEC)
	./upload.sh

clean:
	$(RM) $(OBJ)

.PHONY: clean
