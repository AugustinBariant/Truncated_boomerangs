from os import error
from pickle import SHORT_BINSTRING
import sys
#Call python3 BeautifulBoomerang.py solname r1 r2 (keyvalue.file)
filename = "Deoxys_TK3_6R.sol"
r1 = 4
r2 = 4
filename2 = ""
if(__name__=="__main__"):
    if len(sys.argv) >=2:
        filename = sys.argv[1]
    if len(sys.argv) >=3:
        r1 = int(sys.argv[2])
    if len(sys.argv) >=4:
        r2 = int(sys.argv[3])
    # Filename2 is the name of the file containing the values of the tweakey differences
    if(len(sys.argv) >=5):
        filename2 = sys.argv[4]

dic = {}
#Initialise dic
for i in range((r1+1)*16):
    dic["ux"+str(i)] = 0
    dic["uxt"+str(i)] = 0
    dic["uy"+str(i)] = 0
    dic["uyt"+str(i)] = 0
    dic["uzt"+str(i)] = 0
    dic["utk"+str(i)] = 0
    dic["uye"+str(i)] = 0
    dic["uze"+str(i)] = 0
    dic["ute"+str(i)] = 0
    dic["uxe"+str(i)] = 0
    dic["utkv"+str(i)] = "00"

for i in range((r2+1)*16):
    dic["lx"+str(i)] = 0
    dic["lxt"+str(i)] = 0
    dic["ly"+str(i)] = 0
    dic["lyt"+str(i)] = 0
    dic["lzt"+str(i)] = 0
    dic["ltk"+str(i)] = 0
    dic["lye"+str(i)] = 0
    dic["lze"+str(i)] = 0
    dic["lte"+str(i)] = 0
    dic["lxe"+str(i)] = 0
    dic["ltkv"+str(i)] = "00"

for i in range((r2+1)*16,(r2+2)*16):
    dic["ltkv"+str(i)] = "00"
    dic["ltk"+str(i)] = 0


for i in range(48):
    dic["deltautk"+str(i)] = "00"
    dic["deltaltk"+str(i)] = "00"


for i in range(32):
    dic["uzswitched"+str(i)] = 0
    dic["uxswitched"+str(i)] = 0
    dic["uyswitched"+str(i)] = 0

    dic["lzswitched"+str(i)] = 0
    dic["lxswitched"+str(i)] = 0
    dic["lyswitched"+str(i)] = 0


def is_float(val):
    try:
        num = float(val)
    except ValueError:
        return False
    return True

equal = False
# dic["x1"]= value of x1 in the solution
with open(filename,"r") as f:
    for l in f:
        val = l.split(" ")
        val = [i for i in val if i!=" "]
        if(len(val)==2 and is_float(val[1])):              
            dic[val[0]] = int(float(val[1]))
            #In case of imprecisions of symphony
            if(float(val[1])>int(float(val[1]))+0.95):
                dic[val[0]] = int(float(val[1])) + 1
if(filename2 != ""):
    with open(filename2,"r") as f:
        for l in f:
            val = l.split(" ")
            val = [i for i in val if i!=" "]
            if(len(val)==2):
                dic[val[0]] = val[1][:2]

#Plaintext
print("""\\documentclass{standalone}

% From https://tex.stackexchange.com/questions/15010/vecx-but-with-arrow-from-right-to-left
\\makeatletter
\\DeclareRobustCommand{\\cev}[1]{%
  {\\mathpalette\\do@cev{#1}}%
}
\\newcommand{\\do@cev}[2]{%
  \\vbox{\\offinterlineskip
    \\sbox\\z@{$\m@th#1 x$}%
    \\ialign{##\\cr
      \\hidewidth\\reflectbox{$\\m@th#1\\vec{}\\mkern4mu$}\\hidewidth\\cr
      \\noalign{\\kern-\\ht\\z@}
      $\\m@th#1#2$\\cr
    }%
  }%
}
\\makeatother
\\usepackage{MnSymbol}
\\usepackage{amsmath}

\\usepackage{tikz}
%%% Public TikZ libraries
\\usetikzlibrary{patterns}
\\usetikzlibrary{decorations.pathreplacing}

%%% Custom TikZ addons
\\usetikzlibrary{crypto.symbols}
\\tikzset{shadows=no}  

\\begin{document}""")
print("""\\begin{tikzpicture}[scale=0.2]

\centering

  \everymath{\scriptstyle}

  \\tikzset{edge/.style=-latex new, arrow head=6pt, thick};
""")
print("\draw (5.5,5.5) -- (5.5,1.6);")
print("\\begin{scope}[xshift=3.5cm, yshift=5.5cm]")
print("""
 \draw (0,0) rectangle (4,4);
      \draw (0,1) -- +(4,0);
      \draw (0,2) -- +(4,0);
      \draw (0,3) -- +(4,0);
      \draw (1,0) -- +(0,4);
      \draw (2,0) -- +(0,4);
      \draw (3,0) -- +(0,4);
      \path (2,4.5) node {\scriptsize $P$};""")
for i in range(4):
    for j in range(4):
        if(dic["uxt"+str(4*j+i)]==1):
            print("\\fill[color=gray] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
            if(dic["uxe"+str(4*j+i)]==1):
                print("\\node at ("+str(j+0.5)+","+str(3-i + 0.5)+") {$\\ast$};")
                equal = True
        elif(dic["ux"+str(4*j+i)]==1):
            print("\\fill[color=blue] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
            print("\\node[color=white] at ("+str(j+0.5)+","+str(3-i+0.5)+") {\\tiny \\texttt{XX}};")
print("\end{scope}")












#Rounds
for r in range(r1+1):
    print("\\begin{scope}[yshift=-"+str(r*7)+"cm]")


    if(r!=r1): print("\draw[edge] (1,0) -- node[left] {\\tiny $TKS$} ++(0,-3);")
    print("""\draw (5.5,2) circle (0.3);
    

    \\begin{scope}[xshift=-1cm]
	  
      \draw (0,0) rectangle (4,4);
      \draw (0,1) -- +(4,0);
      \draw (0,2) -- +(4,0);
      \draw (0,3) -- +(4,0);
      \draw (1,0) -- +(0,4);
      \draw (2,0) -- +(0,4);
      \draw (3,0) -- +(0,4);
      \path (-2,2) node {$tk_{"""+str(r)+"""}$};
      
      \draw[edge] (4,2) -- node[below] {\\tiny $ATK_{"""+str(r)+"""}$} +(5,0);""")
    for i in range(4):
        for j in range(4):
            if(dic["utk"+str(16*r+4*j+i)]==1):
                print("\\fill[color=blue] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                print("\\node[color=white] at ("+str(j+0.5)+","+str(3-i+0.5)+") {\\tiny \\texttt{"+dic["utkv"+str(16*r+4*j+i)]+"}};")

    print("\end{scope}")

    
    print("""\\begin{scope}[xshift=8cm]
      \draw (0,0) rectangle (4,4);
      \draw (0,1) -- +(4,0);
      \draw (0,2) -- +(4,0);
      \draw (0,3) -- +(4,0);
      \draw (1,0) -- +(0,4);
      \draw (2,0) -- +(0,4);
      \draw (3,0) -- +(0,4);
      \path (2,4.5) node {\scriptsize $x_{"""+str(r)+"""}$};
      
      \draw[edge] (4,2) -- node[above] {\\tiny $SB$} +(4,0);""")

    bpattern = [[False for i in range(4)] for j in range(4)]
    if(r >= r1-1):
        for i in range(4):
            for j in range(4):
                if(dic["uyswitched"+str(16*(r-r1+1)+4*j+i)]==1):
                    print("\\draw[pattern=north west lines] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    bpattern[i][j]=True
    
    for i in range(4):
        for j in range(4):
            if not bpattern[i][j]:
                if(dic["uyt"+str(16*r+4*j+i)]==1):
                    print("\\fill[color=gray] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    if(dic["uye"+str(16*r+4*j+i)]==1):
                        print("\\node at ("+str(j+0.5)+","+str(3-i + 0.5)+") {$\\ast$};")
                        equal = True
                elif(dic["uy"+str(16*r+4*j+i)]==1):
                    print("\\fill[color=blue] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    print("\\node[color=white] at ("+str(j+0.5)+","+str(3-i+0.5)+") {\\tiny \\texttt{XX}};")
    print("\end{scope}")


    print("""\\begin{scope}[xshift=16cm]
      
      \draw (0,0) rectangle (4,4);
      \draw (0,1) -- +(4,0);
      \draw (0,2) -- +(4,0);
      \draw (0,3) -- +(4,0);
      \draw (1,0) -- +(0,4);
      \draw (2,0) -- +(0,4);
      \draw (3,0) -- +(0,4);
      \path (2,4.5) node {\scriptsize $y_{"""+str(r)+"""}$};
      
      """)
    if(r!=r1): print("\draw[edge] (4,2) -- node[above] {\\tiny $SR,MC$} +(4,0);")
    bpattern = [[False for i in range(4)] for j in range(4)]
    if(r >= r1-1):
        for i in range(4):
            for j in range(4):
                if(dic["uzswitched"+str(16*(r-r1+1)+4*j+i)]==1):
                    print("\\draw[pattern=north west lines] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    bpattern[i][j]=True

    
    for i in range(4):
        for j in range(4):
            if not bpattern[i][j]:
                if(dic["uzt"+str(16*r+4*j+i)]==1):
                    print("\\fill[color=gray] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    if(dic["uze"+str(16*r+4*j+i)]==1):
                        print("\\node at ("+str(j+0.5)+","+str(3-i + 0.5)+") {$\\ast$};")
                        equal = True
                elif(dic["uy"+str(16*r+4*j+i)]==1):
                    print("\\fill[color=blue] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    print("\\node[color=white] at ("+str(j+0.5)+","+str(3-i+0.5)+") {\\tiny \\texttt{XX}};")
    
    print("\end{scope}")

    if(r!=r1):  
        print("""\\begin{scope}[xshift=24cm]
        \draw (0,0) rectangle (4,4);
        \draw (0,1) -- +(4,0);
        \draw (0,2) -- +(4,0);
        \draw (0,3) -- +(4,0);
        \draw (1,0) -- +(0,4);
        \draw (2,0) -- +(0,4);
        \draw (3,0) -- +(0,4);
        \path (2,4.5) node {\scriptsize $w_{"""+str(r)+"""}$};""")
      
    
        print("\draw (4,2) -- ++(2,0) -- ++(0,-3.5) -- ++(-24.5,0) -- ++(0,-3.9);")
        bpattern = [[False for i in range(4)] for j in range(4)]
        
        if(r == r1-1):
            for i in range(4):
                for j in range(4):
                    if(dic["uxswitched"+str(16+4*j+i)]==1):
                        print("\\draw[pattern=north west lines] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                        bpattern[i][j]=True

        
        for i in range(4):
            for j in range(4):
                if not bpattern[i][j]:
                    if(dic["uxt"+str(16*(r+1)+4*j+i)]==1):
                        print("\\fill[color=gray] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                        if(dic["uxe"+str(16*(r+1)+4*j+i)]==1):
                            print("\\node at ("+str(j+0.5)+","+str(3-i + 0.5)+") {$\\ast$};")
                            equal = True
                    elif(dic["ux"+str(16*(r+1)+4*j+i)]==1):
                        print("\\fill[color=blue] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                        print("\\node[color=white] at ("+str(j+0.5)+","+str(3-i+0.5)+") {\\tiny \\texttt{XX}};")
    
        print("\end{scope}")


    print("\end{scope}")






#lower trail
#Different layout : print("\\begin{scope}[yshift=-"+str((r1-1)*7)+"cm,xshift=+35cm]")
print("\\begin{scope}[yshift=-"+str((r1+1)*7)+"cm]")

for r in range(r2+1):
    print("\\begin{scope}[yshift=-"+str(r*7)+"cm]")

    if r!=0:
        print("\draw[edge] (1,0) -- node[left] {\\tiny $TKS$} ++(0,-3);")
        
        print("""\draw (5.5,2) circle (0.3);""")
        print("""
        \\begin{scope}[xshift=-1cm]
        
        \draw (0,0) rectangle (4,4);
        \draw (0,1) -- +(4,0);
        \draw (0,2) -- +(4,0);
        \draw (0,3) -- +(4,0);
        \draw (1,0) -- +(0,4);
        \draw (2,0) -- +(0,4);
        \draw (3,0) -- +(0,4);
        \path (-2,2) node {$tk_{"""+str(r+r1-1)+"""}$};
        
        \draw[edge] (4,2) -- node[below] {\\tiny $ATK_{"""+str(r+r1-1)+"""}$} +(5,0);""")
        for i in range(4):
            for j in range(4):
                if(dic["ltk"+str(16*r+4*j+i)]==1):
                    print("\\fill[color=blue] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    print("\\node[color=white] at ("+str(j+0.5)+","+str(3-i+0.5)+") {\\tiny \\texttt{"+str(dic["ltkv"+str(16*r+4*j+i)])+"}};")

        print("\\end{scope}")

    
    print("""\\begin{scope}[xshift=8cm]
      \draw (0,0) rectangle (4,4);
      \draw (0,1) -- +(4,0);
      \draw (0,2) -- +(4,0);
      \draw (0,3) -- +(4,0);
      \draw (1,0) -- +(0,4);
      \draw (2,0) -- +(0,4);
      \draw (3,0) -- +(0,4);
      \path (2,4.5) node {\scriptsize $x_{"""+str(r+r1-1)+"""}$};
      
      \draw[edge] (4,2) -- node[above] {\\tiny $SB$} +(4,0);""")
    bpattern = [[False for i in range(4)] for j in range(4)]
    
    if(r <= 1):
        for i in range(4):
            for j in range(4):
                if(dic["lyswitched"+str(16*r+4*j+i)]==1):
                    print("\\draw[pattern=north west lines] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    bpattern[i][j]=True
    
    for i in range(4):
        for j in range(4):
            if not bpattern[i][j]:
                if(dic["lyt"+str(16*r+4*j+i)]==1):
                    print("\\fill[color=gray] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    if(dic["lye"+str(16*r+4*j+i)]==1):
                        print("\\node at ("+str(j+0.5)+","+str(3-i + 0.5)+") {$\\ast$};")
                        equal = True
                elif(dic["ly"+str(16*r+4*j+i)]==1):
                    print("\\fill[color=blue] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    print("\\node[color=white] at ("+str(j+0.5)+","+str(3-i+0.5)+") {\\tiny \\texttt{XX}};")
    print("\end{scope}")


    print("""\\begin{scope}[xshift=16cm]
      
      \draw (0,0) rectangle (4,4);
      \draw (0,1) -- +(4,0);
      \draw (0,2) -- +(4,0);
      \draw (0,3) -- +(4,0);
      \draw (1,0) -- +(0,4);
      \draw (2,0) -- +(0,4);
      \draw (3,0) -- +(0,4);
      \path (2,4.5) node {\scriptsize $y_{"""+str(r+r1-1)+"""}$};
      
      """)
    if(r!=r2): print("\draw[edge] (4,2) -- node[above] {\\tiny $SR,MC$} +(4,0);")

    
    bpattern = [[False for i in range(4)] for j in range(4)]
    if(r <= 1):
        for i in range(4):
            for j in range(4):
                if(dic["lzswitched"+str(16*r+4*j+i)]==1):
                    print("\\draw[pattern=north west lines] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    bpattern[i][j]=True

    for i in range(4):
        for j in range(4):
            if not bpattern[i][j]:
                if(dic["lzt"+str(16*r+4*j+i)]==1):
                    print("\\fill[color=gray] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    if(dic["lze"+str(16*r+4*j+i)]==1):
                        print("\\node at ("+str(j+0.5)+","+str(3-i + 0.5)+") {$\\ast$};")
                        equal = True
                elif(dic["ly"+str(16*r+4*j+i)]==1):
                    print("\\fill[color=blue] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    print("\\node[color=white] at ("+str(j+0.5)+","+str(3-i+0.5)+") {\\tiny \\texttt{XX}};")
    print("\end{scope}")

    if(r!=r2):  
        print("""\\begin{scope}[xshift=24cm]
        \draw (0,0) rectangle (4,4);
        \draw (0,1) -- +(4,0);
        \draw (0,2) -- +(4,0);
        \draw (0,3) -- +(4,0);
        \draw (1,0) -- +(0,4);
        \draw (2,0) -- +(0,4);
        \draw (3,0) -- +(0,4);
        \path (2,4.5) node {\scriptsize $w_{"""+str(r+r1-1)+"""}$};""")
      
    
        print("\draw (4,2) -- ++(2,0) -- ++(0,-3.5) -- ++(-24.5,0) -- ++(0,-3.9);")

        
        bpattern = [[False for i in range(4)] for j in range(4)]
        if(r == 0):
            for i in range(4):
                for j in range(4):
                    if(dic["lxswitched"+str(16 + 4*j+i)]==1):
                        print("\\draw[pattern=north west lines] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                        bpattern[i][j]=True
        
        for i in range(4):
            for j in range(4):
                if not bpattern[i][j]:
                    if(dic["lxt"+str(16*(r+1)+4*j+i)]==1):
                        print("\\fill[color=gray] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                        if(dic["lxe"+str(16*(r+1)+4*j+i)]==1):
                            print("\\node at ("+str(j+0.5)+","+str(3-i + 0.5)+") {$\\ast$};")
                            equal = True
                    elif(dic["lx"+str(16*(r+1)+4*j+i)]==1):
                        print("\\fill[color=blue] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                        print("\\node[color=white] at ("+str(j+0.5)+","+str(3-i+0.5)+") {\\tiny \\texttt{XX}};")
    
        print("\end{scope}")
    else:
        print("\draw (20,2) -- ++(2,0) -- ++(0,-3.5) -- ++(-16.5,0) -- ++(0,-3.9);")
        print("\\begin{scope}[yshift=-7cm]")

        print("""\draw (5.5,2) circle (0.3);
        

        \\begin{scope}[xshift=-1cm]
        
        \draw (0,0) rectangle (4,4);
        \draw (0,1) -- +(4,0);
        \draw (0,2) -- +(4,0);
        \draw (0,3) -- +(4,0);
        \draw (1,0) -- +(0,4);
        \draw (2,0) -- +(0,4);
        \draw (3,0) -- +(0,4);
        \path (-2,2) node {$tk_{"""+str(r2+r1)+"""}$};
        
        \draw[edge] (4,2) -- node[below] {\\tiny $ATK_{"""+str(r2+r1)+"""}$} +(5,0);""")

        for i in range(4):
            for j in range(4):
                if(dic["ltk"+str(16*(r+1)+4*j+i)]==1 or str(dic["ltkv"+str(16*(r+1)+4*j+i)])!= "00"):
                    print("\\fill[color=blue] ("+str(j)+","+str(3-i)+") rectangle +(1,1);")
                    print("\\node[color=white] at ("+str(j+0.5)+","+str(3-i+0.5)+") {\\tiny \\texttt{"+str(dic["ltkv"+str(16*(r+1)+4*j+i)])+"}};")

        print("\\end{scope}")

        
        print("""\\begin{scope}[xshift=8cm]
        \draw (0,0) rectangle (4,4);
        \draw (0,1) -- +(4,0);
        \draw (0,2) -- +(4,0);
        \draw (0,3) -- +(4,0);
        \draw (1,0) -- +(0,4);
        \draw (2,0) -- +(0,4);
        \draw (3,0) -- +(0,4);
        \path (2,4.5) node {\\scriptsize $C$};""")
        print("\end{scope}")
        print("\end{scope}")

    print("\end{scope}")
print("\end{scope}")

# Légende
print("\\begin{scope}[yshift=-"+str(0)+"cm,xshift=35cm]")
print("\\draw (0,0) rectangle +(20,-" + (str(10) if equal else "8") + ");")
print("\\fill[color=gray] (0.5,-1.5) rectangle +(1,1);")
print("\\draw (0.5,-1.5) rectangle +(1,1);")
print("\\path (1.5,-1) node[anchor=west] {\scriptsize Truncated bytes};")
print("\\fill[color=blue] (0.5,-3.5) rectangle +(1,1);")
print("\\draw (0.5,-3.5) rectangle +(1,1);")
print("\\path (1.5,-3) node[anchor=west] {\scriptsize Active bytes};")
print("\\node[color=white] at (1,-3) {\\tiny \\texttt{01}};")
print("\\fill[pattern=north west lines] (0.5,-5.5) rectangle +(1,1);")
print("\\draw (0.5,-5.5) rectangle +(1,1);")
print("\\path (1.5,-5) node[anchor=west] {\scriptsize Switched bytes};")
print("\\fill[color=gray] (0.5,-7.5) rectangle +(1,1);")
print("\\filldraw[color=blue] (0.5,-7.5) -- (1.5,-7.5) -- (0.5,-6.5)--cycle;")
print("\\draw (0.5,-7.5) rectangle +(1,1);")
print("\\draw (1.5,-7.5) -- +(-1,1);")
print("\\node[color=white] at (1,-7) {\\tiny \\texttt{01}};")
print("\\path (1.5,-7) node[anchor=west] {\scriptsize State changing bytes};")
if(equal):
    print("\\draw (0.5,-9.5) rectangle +(1,1);")
    print("\\path (1.5,-9) node[anchor=west] {\scriptsize Truncated equal bytes};")
    print("\\fill[color=gray] (0.5,-9.5) rectangle +(1,1);")
    print("\\node at (1,-9) {$\\ast$};")
print("\\end{scope}") 
#

def returnKey(i,ul):
    l = []
    baseindex = "delta"+ul+"tk"
    for k in range(16):
        l.append(dic[baseindex+str(i*16 +k)])
    return l

print("\\begin{scope}[yshift=-"+str(15)+"cm,xshift=32cm]")
for i in range(3):
    l = returnKey(i,"u")
    print("\\path (0,"+str(-7*i)+") node[anchor=west] {")
    print("$\\Delta TK^"+str(i+1)+" =")
    print("\\left[\\begin{smallmatrix}")
    print("  \\mathtt{"+ l[0] +"} & \\mathtt{"+ l[4] +"} & \\mathtt{"+ l[8] +"} & \\mathtt{"+ l[12] +"}\\\\")
    print("  \\mathtt{"+ l[1] +"} & \\mathtt{"+ l[5] +"} & \\mathtt{"+ l[9] +"} & \\mathtt{"+ l[13] +"}\\\\")
    print("  \\mathtt{"+ l[2] +"} & \\mathtt{"+ l[6] +"} & \\mathtt{"+ l[10] +"} & \\mathtt{"+ l[14] +"}\\\\")
    print("  \\mathtt{"+ l[3] +"} & \\mathtt{"+ l[7] +"} & \\mathtt{"+ l[11] +"} & \\mathtt{"+ l[15] +"}")
    print("\\end{smallmatrix}\\right]$};")

for i in range(3):
    l = returnKey(i,"l")
    print("\\path (16,"+str(-7*i)+") node[anchor=west] {")
    print("$\\nabla TK^"+str(i+1)+" =")
    print("\\left[\\begin{smallmatrix}")
    print("  \\mathtt{"+ l[0] +"} & \\mathtt{"+ l[4] +"} & \\mathtt{"+ l[8] +"} & \\mathtt{"+ l[12] +"}\\\\")
    print("  \\mathtt{"+ l[1] +"} & \\mathtt{"+ l[5] +"} & \\mathtt{"+ l[9] +"} & \\mathtt{"+ l[13] +"}\\\\")
    print("  \\mathtt{"+ l[2] +"} & \\mathtt{"+ l[6] +"} & \\mathtt{"+ l[10] +"} & \\mathtt{"+ l[14] +"}\\\\")
    print("  \\mathtt{"+ l[3] +"} & \\mathtt{"+ l[7] +"} & \\mathtt{"+ l[11] +"} & \\mathtt{"+ l[15] +"}")
    print("\\end{smallmatrix}\\right]$};")

print("\\end{scope}")

i=0
def printLineifInDic(Name, key):
    global i
    if key in dic.keys():
        print("\\path (0,"+str(i)+") node[anchor=west] {{\scriptsize "+ Name + ": }$"+str(dic[key]) + "$};")
        i-=2


print("""
\\begin{scope}[yshift=-45cm,xshift=35cm]
\\path (0,0) node[anchor=west,text width=3cm] {\\scriptsize  \\begin{align*}
  \\cev{q} &= 2^{-46} &
  \\vec{p} &= 2^{-28} \\\\
  r &= 2^{-12} &
  \\vec{q} &= 2^{-6} \\\\
  |\mathcal{D}_{out}^{up}| &= 1 &
  p_b &= 2^{-108} \\\\
  p_{\\$} &= 2^{-88} &
  \\kappa &= 40 \\\\
  l &= 2^{-32} &
  \\tilde{\\sigma} &= 2^{52} \\\\
  \\mu &= 1 &  S &= 2^{54}\\\\
  D = T &= 2^{55} & M &= 2^{55}
\\end{align*}};
\\end{scope}""")

print("\\path (current bounding box.north east) +(5,5);")
print("\\path (current bounding box.south west) +(-5,-5);")
print("""\\end{tikzpicture}""")
print("\\end{document}")



'''for i in range(32):
    key1 = "ly"+str(i)
    key = "uy"+str((r1-1)*16 + i)
    if key in dic.keys():
        print(str(dic[key]) + " and " + str(dic[key1]))
    else:
        print("0")'''


