CFLAGS= -Wall -Wextra -march=native -O3 -g -std=c99 -fopenmp
LDFLAGS= -fopenmp
LDLIBS= -lm

BIN= deoxys_tk2 deoxys_tk1_8 deoxys_tk3_10 deoxys_tk3_11 deoxys_tk3_12 deoxys_tk3_13 deoxys_tk2_8 deoxys_tk2_9

all: $(BIN)

clean:
	rm $(BIN)
	rm *.o

$(BIN): %: %.o aes-rng.o

.PHONY: all clean
