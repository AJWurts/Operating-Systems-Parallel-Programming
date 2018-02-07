CC = cc
CFLAGS = -Wall -g
LIBS = -pthread -lm
DEPS = bathroom.h
OBJ = bathroom.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
        

main: $(OBJ)
	cc -o $@ $^ $(CFLAGS) $(LIBS)

all: main
