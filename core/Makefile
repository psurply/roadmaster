CC	= $(CROSS_COMPILE)gcc
CFLAGS	= -std=c99 -Wall -Werror -I../include/ \
	  -DDEBUG -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE=500 -D_BSD_SOURCE\
	  -D$(ROADMASTER_TARGET) -lrt
CPPFLAGS = -MMD

EXEC	= roadmaster-core

SRC	= main.c \
	  core.c \
	  gpio.c
SRC	:= $(addprefix src/, $(SRC))
OBJ	= $(SRC:.c=.o)
DEPS	= $(OBJ:.o=.d)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

upload: $(EXEC)
	./upload.sh

clean:
	$(RM) $(OBJ)

.PHONY: clean

-include $(DEPS)
