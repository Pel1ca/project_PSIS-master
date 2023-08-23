# _*_ MakeFile _*_


CC = gcc
CFLAGS = -Wall

_OBJ = serial.o socket.o fc_funcs.o mv_func.o
ODIR = obj
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $<

all: coms

coms : $(OBJ)
	ar -cvq ../obj/coms.a $^

clean: 
	rm $(ODIR)/*.o
