GCC = gcc
CFLAGS = -Wall -g
LIBS = -pthread -lm
DEPS = bathroom.h
OBJ = bathroom.o main.o

%.o: %.c $(DEPS)
	$(GCC) -c -o $@ $< $(CFLAGS)
        

bathroomSim: $(OBJ)
	$(GCC) -o $@ $^ $(CFLAGS) $(LIBS)

all: bathroomSim

clean:
	rm -f $(OBJ) bathroomSim

run: 
	./bathroomSim 100 5 1000 2000
