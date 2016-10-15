CC	= $(CROSS_COMPILE)gcc
CFLAGS	= -std=c99 -Wall -Werror -I../include/ \
	  -DDEBUG -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE=500 \
	  -lrt

EXEC	= roadmaster-cli

SRC	= main.c
SRC	:= $(addprefix src/, $(SRC))
OBJ	= $(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

upload: $(EXEC)
	./upload.sh

clean:
	$(RM) $(OBJ)

.PHONY: clean
