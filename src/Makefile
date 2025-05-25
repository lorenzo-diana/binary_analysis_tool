CC=gcc
CFLAGS=-std=c23 -g -Wall -Werror
LINKED_LIBS=-lbfd
OBJ=linked_list.o loader.o
DEMO_TARGET=demo1 demo2
DEMO_FOLDER=./demo/

all: $(DEMO_TARGET)

*.o: *.c
	$(CC) $(CFLAGS) -o $@ -c $< $(LINKED_LIBS)

$(DEMO_TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(DEMO_FOLDER)$@.c $^ $(LINKED_LIBS)

clean:
	rm $(OBJ) $(DEMO_TARGET)
