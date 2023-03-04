CFLAGS= -Wall -Wextra -O3 -g -fopenmp -std=c99

BIN = Truncated_boomerang_MILP
LPFILES := $(wildcard output/*.lp)
SOLFILES := $(patsubst %.lp,%.sol,${LPFILES})
TEXFILES := $(patsubst %.lp,%.tex,${LPFILES})
PDFFILES := $(patsubst %.lp,%.pdf,${LPFILES})

build: ${BIN}

gurobi: ${SOLFILES}

pdf: ${PDFFILES}

test: build
	./Truncated_boomerang_MILP -d0 -p 3 3 
	$(MAKE) pdf

%.sol: %.lp
	gurobi_cl ResultFile=$@ $<

%.tex: %.sol
	./make_figure.py $< > $@

%.pdf: %.tex
	(cd output; pdflatex ${<:output/%=%})

clean:
	rm -f ${BIN}

.PHONY: build gurobi pdf test
