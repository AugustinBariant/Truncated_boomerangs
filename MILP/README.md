# MILP model For truncated boomerang attacks


## MILP model generation
To generate the MILP Files, first create a folder LPFiles/ in your current directory, then compile the c source Deoxys_boomerang_truncated_all.c (or Kiasu_boomerang_truncated.c) into an executable (for instance, a.out):

```sh
gcc Deoxys_boomerang_truncated_all.c
```

then run the executable (omit the fourth parameter i for Kiasu):

```sh
./a.out U L fromPt i opt
```

This command will create the MILP file for the search of a boomerang trail with U upper rounds, L lower rounds, starting from the Plaintext iff fromPT!=0, in the RTKi model, and with equal states optimisation disabled iff opt=0. The .lp file is generated in the LPFiles/ folder. In total, the boomerang trail will reach U + L rounds (the upper and lower trails have each 1 extra round in the middle, which overlaps with the other's main trail).


## MILP solving
Then, run the gurobi optimizer (or any other MILP solver of your choice) on the MILP file. For gurobi, the command looks like this:

```sh
gurobi_cl ResultFile=/result/file.sol Threads=8 LPFile.lp
``` 


## Figure generation
After the generation of the solution file (.sol file), use the python script *Figure_generation/BeautifulBoomerang.py* to generate the latex figure (without tweakey differences instantiation for now). 

```sh
python3 BeautifulBoomerang.py /result/file.sol U L > Figure.tex
``` 

Where U and L are the same arguments than those used for the generation of the MILP files. 

Then, compile the latex file:

```sh
pdflatex Figure.tex
```

Every non standard package needed for the compilation of the latex file is in the folder.

The figure *Figure.pdf* is generated.

## Tweakey instantiation
The sage source code *Tweakey_differences_instantiation/trail.sage* can be modified to generate the optimal tweakey differences of a particular trail. For this, find the constraints of the tweakey in the trail, write them down as shown in the code, modify the main function and execute the command:

```sh
sage trail.sage ../outputkey
```

This will write concrete tweakey differences in the file *outputkey*.
The figure generation script can take the path to this file as an extra input:

```sh
python3 BeautifulBoomerang.py /result/file.sol U L ../outputkey > Figure.tex
```


This instantiates the key values on the figure *Figure.pdf* after compiling with `pdflatex`. The rest needs to be done by hand.


