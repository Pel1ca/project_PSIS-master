# _*_ MakeFile _*_


CC = gcc
CFLAGS = -Wall -lpthread -lSDL2 -lSDL2_image -g
dirs = coms

_OBJ = UI_library.o board_func.o coms.a game_rules.o graphics.o client_funcs.o
ODIR = obj
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $<


$(ODIR)/%.a: $(foreach dir, $(dirs), $(wildcard $(dir)/*.o))
	+$(MAKE) -C $(basename $(notdir $@))

all: pacman server

pacman : pacman.c $(OBJ) pac_lib.h 
	$(CC) -o $@ $^  $(CFLAGS)

server : server.c $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

n_server : n_server.c $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)


clean: 
	rm $(ODIR)/*
