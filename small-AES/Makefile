CFLAGS= -Wall -Wextra -O3 -g -fopenmp -std=c99
CXXFLAGS= -Wall -Wextra -O3 -g -fopenmp -std=gnu++11

all: tables4.h boomerang5 boomerang6

tables4.h:
	sage aes-tables.sage 4 > $@

boomerang6: tables4.h boomerang6.o parallel_sort.o
	$(CXX) $(CXXFLAGS) -o $@ boomerang6.o parallel_sort.o $(LDLIBS)

clean:
	rm -f boomerang5 boomerang6 *.o

.phony: all
