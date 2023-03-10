# MILP model For truncated boomerang attacks


There are several steps to generate parameters for a boomerang attack:
1. Generate the MILP model with `Truncated_boomerang_MILP`
2. Solve the model (with Gurobi)
3. (optional) Generate a figure to visualize the trail using `make_figure.py`
4. Instanciate the differences.  The requires some manual work, starting from `Instantiate_diff.sage`. An instantiation example is given inside the script.
5. (optional) Generate a figure with instanciated values using `make_figure.py`

## Compilation

The provided `Makefile` should automate part of the work:
- running `make build` or `make` will compile the programs
- running `make test` will run the search for a 6-round bommerang attack from plaintext against AES and generate a PDF figure. Warning: gurobi takes around 5 minutes to solve the model with 8 threads.
- for a given set of parameters, you can run step 1 manually (for instance `Deoxys_boomerang 5 6 0 3` for an attack against 11-round Deoxys in the RTK3 model), and run `make pdf` to run steps 2 and 3.

## MILP model generation

To generate the MILP model run the executable:
```sh
./Truncated_boomerang_MILP [options] [U L] 
        -d, --deoxys=i    Deoxys with model RTK<i>
        -k, --kiasu       Kiasu
        -p, --plaintext   Boomerang starting from plaintext
        -c, --ciphertext  Boomerang starting from ciphertext
        -t, --time        Minimize the time complexity instead of the data complexity (experimental)
        -h, --help        Give a help message
        U L               Boomerang with <U> upper rounds and <L> lower rounds
```

The `.lp` file is generated in the `output/` folder. In total, the boomerang trail will reach ⟨U⟩+⟨L⟩ rounds (the upper and lower trails have each 1 extra round in the middle, which overlaps with the other's main trail).


## MILP solving

Then, run the gurobi optimizer (or any other MILP solver of your choice) on the MILP `.lp` file. For gurobi, the command looks like this:
```sh
gurobi_cl ResultFile=output/file.sol Threads=8 output/file.lp
``` 

Alternatively, running `make gurobi` will run Gurobi over all existing `.lp` files in the `output` directory.


## Figure generation

After the generation of the solution file (`.sol` file), use the python script `make_figure.py` to generate the LaTeX figure (without tweakey differences instantiation for now). 

```sh
python3 make_figure.py output/file.sol > output/Figure.tex
``` 

Then, compile the latex file:

```sh
cd output
pdflatex Figure.tex
```
Non standard packages needed for the compilation of the latex file are in the `output` folder.

The figure `Figure.pdf` is generated.

## Tweakey instantiation
The sage source code `Instantiate_diff.sage` provides tools to generate the optimal tweakey differences of a particular trail, together with an example of optimal tweakey generation for the 12-round attack on Deoxys-BC in the RTK3 model of the paper (available in the full version on eprint). The following command will write the optimal tweakey differences in the file `output/Deoxys_TK3_boomerang_fromCiphertext_5R_7R_tweakeyvalues.txt`:

```sh
sage Instantiate_diff.sage
```

The code can be modified to generate optimal tweakey differences for other trails. The figure generation script can take the path to the optimal tweakey difference file `outputkey.txt` as an extra input:

```sh
python3 make_figure output/file.sol outputkey.txt > output/Figure.tex
```

This instantiates the key values on the figure `Figure.pdf` after compiling with `pdflatex`. The rest needs to be done by hand.
