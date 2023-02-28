CFLAGS= -Wall -Wextra -O3 -g -fopenmp -std=c99

BIN = Deoxys_boomerang Kiasu_boomerang
LPFILES := $(wildcard output/*.lp)
SOLFILES := $(patsubst %.lp,%.sol,${LPFILES})
TEXFILES := $(patsubst %.lp,%.tex,${LPFILES})
PDFFILES := $(patsubst %.lp,%.pdf,${LPFILES})

build: ${BIN}

gurobi: ${SOLFILES}

figures: ${PDFFILES}

%.sol: %.lp
	gurobi_cl ResultFile=$@ Threads=8 $<

%.tex: %.sol
	./make_figure.py $< `echo $<| sed -e 's/.*_\(.\)R_\(.\)R.sol/\1\n\2/'` > $@

%.pdf: %.tex
	(cd output; pdflatex ${<:output/%=%})

clean:
	rm -f ${BIN}

.PHONY: build run_gurobi