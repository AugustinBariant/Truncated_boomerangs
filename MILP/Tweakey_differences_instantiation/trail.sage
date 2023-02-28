from sys import argv
#AES parameters


F.<a> = GF(2^8, modulus=x^8+x^4+x^3+x+1) # AES field

V = VectorSpace(F,4)
MC = Matrix(F,4,4,[F.fetch_int(i) for i in [2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2]]) # MixColumn
MC_inv = MC.inverse()

MC_inv_blockmatrix = [block_matrix(1,4,[el.matrix() for el in MC_inv[i]])  for i in range(4)]


M = block_matrix(4,4,[[F.fetch_int(i).matrix() for i in [2,3,1,1]],[F.fetch_int(i).matrix() for i in [1,2,3,1]],[F.fetch_int(i).matrix() for i in [1,1,2,3]],[F.fetch_int(i).matrix() for i in [3,1,1,2]]])

MS = MatrixSpace(GF(2),8,8)

Zero = MS(0)
One = MS(1)

from prometheus_client import Counter
from sage.crypto.sboxes import AES as SBox
DDT = SBox.difference_distribution_table()
BCT = SBox.boomerang_connectivity_table()


F2x.<z> = GF(2)[] # Polynomials over GF(2)
LFSR2_matrix = companion_matrix(z^8+z^2+1,format='right').antitranspose()

LFSR3_matrix = LFSR2_matrix.inverse()


# Exemple debile sur F(2)^8
#  - (x,y) tels que 2*x+LFSR(y) = 0
#  - Maximiser les transitions x->y et 2*x->2*y
'''
M = block_matrix([[a.matrix(), LFSR2_matrix]])
V = M.right_kernel()
maxp  = 0
maxat = None
for v in V:
    if v.is_zero():
        continue
    z = F(v[0:8])
    y = F(v[8:16])
    p = (DDT[z.integer_representation()][y.integer_representation()]*
         DDT[(a*z).integer_representation()][(a*y).integer_representation()])
    if p > maxp:
        maxp = p
        maxat = (z,y)
print(maxp, [hex(x.integer_representation()) for x in maxat])

'''
#Returns the column with integers a,b,c and d
def getColumn(a,b,c,d):
    return Matrix(4,[[F.fetch_int(a)],[F.fetch_int(b)],[F.fetch_int(c)],[F.fetch_int(d)]])

def printColumn(Col):
    print([h(Col[i,0]) for i in range(4)])

def printInverseColumn(a,b,c,d): 
    return printColumn(MC_inv*getColumn(a,b,c,d)) 

def getDDT(a,b):
    return DDT[a.integer_representation(),b.integer_representation()]
#Utility function: compute j such that DDT[i,j] = 4:
def DDTForward(i):
    for j in range(256): 
       if DDT[i,j] == 4: 
            print(hex(j)) 
#Utility function: compute j such that DDT[j,i] = 4:
def DDTBackward(i):
    for j in range(256): 
       if DDT[j,i] == 4: 
            print(hex(j)) 
# tweakey = [(TK1)_0,(TK1)_1,(TK1)_2,(TK1)_3,(TK2)_0 ... (TK3)_3] de taille i*nombre de bits de tweakey actifs pour TKi        

# given an output vector in a kernel, prints all the tweakey differences in hexadecimal
# For parameters, see the Key function below
def prettyPrint(v,t,model,name):
    print(name+" TK1: ", end="")
    for i in range(t):
        x = F(v[i*8*model:i*8*model+8])
        print(hex(x.integer_representation()),end=", ")
    print("")
    if(model>=2):
        print(name+" TK2: ", end="")
        for i in range(t):
            x = F(v[i*8*model+8:i*8*model+16])
            print(hex(x.integer_representation()),end=", ")
    print("")
    if(model>=3):
        print(name+" TK3: ", end="")
        for i in range(t):
            x = F(v[i*8*model+16:i*8*model+24])
            print(hex(x.integer_representation()),end=", ")
    print("\n")

# From an element of the kernel, returns the i-th key element of round r
def getKeyElement(v,i,r,t,model):
    x = F.fetch_int(0)
    x += F(v[i*8*model:i*8*model+8])
    if(model>=2):
        x += F((LFSR2_matrix**r)*v[i*model*8 + 8:i*model*8 + 16])
    if(model>=3):
        x += F((LFSR3_matrix**r)*v[i*model*8 + 16:i*model*8 + 24])
    return x

def returnKey(el,t,model,keyPositions):
    l = [["00" for i in range(16)] for j in range(model)]
    for k in range(len(keyPositions)):
        for i in range(model):
            l[i][keyPositions[k]] = "{:02x}".format(F(el[k*8*model+8*i:k*8*model+8*i+8]).integer_representation())
    return l


def h(el):
    return hex(el.integer_representation())

# r : round number
# i : index of the tweakey material
# t : total number of active tweakey bytes with i<t
# model : 1-2-3 if in TK1,2,3
def Key(r,i,t,model):
    M = []
    for j in range(i*model):
        M.append(Zero) 
    M.append(One),
    if(model>=2):
        M.append(LFSR2_matrix**r)
    if(model>=3):
        M.append(LFSR3_matrix**r)
    for j in range((i+1)*model,t*model):
        M.append(Zero)
    return block_matrix(1,[M])
# Key(1,P[i],t,model) is a list of model*t 8x8 Matrixes 


def ZeroFunction(t,model):
    M = []
    for j in range(t*model):
        M.append(Zero) 
    return block_matrix(1,[M])


Key_Permutation = [1,6,11,12, 5,10,15,0, 9,14,3,4, 13,2,7,8]

Key_Permutation_inv = [0 for i in range(16)]

for i in range(16):
    Key_Permutation_inv[Key_Permutation[i]] = i

# Generation automatique de tweakey material
# Inscrit les valeurs dans le fichier file
# Exemple d'utilisation:
'''
if(__name__ == "__main__"):
    if len(argv) <= 1:
        print("Usage: sage script resultfile")
    else:
        f = open(argv[1],"w")

    el = (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0)
    el2 = (0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0)
    printInFile(f, el,el2,2,[11],[5],4,5)
'''

def printInFile(file, el,el2,model,keyPositions,keyPositions2,ru,rl):
    VS = VectorSpace(GF(2),len(el))
    VS2 = VectorSpace(GF(2),len(el2))
    t = len(keyPositions)
    t2 = len(keyPositions)
    el = VS(el)
    el2 = VS2(el2)
    kP = keyPositions.copy()
    kP2 = keyPositions2.copy()
    for r in range(ru+1):
        for i in range(16):
            if i not in keyPositions:
                f.write("utkv"+str(16*r+i) + " 00\n")
            else:
                index = keyPositions.index(i)
                f.write("utkv" + str(16*r + i) + " " + "{:02x}".format(getKeyElement(el,index,r,t,model).integer_representation()) + "\n") 

        for k in range(len(keyPositions)):
            keyPositions[k] = Key_Permutation_inv[keyPositions[k]]
    for i in range(ru-1):
        for k in range(len(keyPositions2)):
                keyPositions2[k] = Key_Permutation_inv[keyPositions2[k]]
    for r in range(rl+2):
        for i in range(16):
            if i not in keyPositions2:
                f.write("ltkv"+str(16*r + i) + " 00\n")
            else:
                index = keyPositions2.index(i)
                f.write("ltkv" + str(16*r + i) + " " + "{:02x}".format(getKeyElement(el2,index,r+ru-1,t2,model).integer_representation()) + "\n") 

        for k in range(len(keyPositions2)):
            keyPositions2[k] = Key_Permutation_inv[keyPositions2[k]]
    uk = returnKey(el,t,model,kP)
    lk = returnKey(el2,t2,model,kP2)
    for k in range(model):
        for i in range(16):
            f.write("deltautk"+str(k*16+i) + " " + uk[k][i]+"\n")
            f.write("deltaltk"+str(k*16+i) + " " + lk[k][i]+"\n")



# In the following, to understand annotations, replace x by w, y by x and z by y
#Relations = list of relations that sould be equal to 0
Relations = []

# TK3_fromCiphertext_5_6
# 4 active bytes in the tweakey material
# returned: final_el = (1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0)
# final_el2 = (1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1)

'''
t = 4 #[0,5,10,15]
model = 3
# Tweakey material of round 2 and 3 is inactive
for i in range(t):
    Relations.append([Key(2,i,t,model)])
    Relations.append([Key(3,i,t,model)])
# Difference in x_0 retracts to 1 byte of z_0
# Tweakey indexes at round 1, from top to bottom => 1,2,3,0
# Round 1:
P = [1,2,3,0]
K1 = block_matrix(4,[[Key(1,P[i],t,model)] for i in range(t)])
Relations.append([MC_inv_blockmatrix[0]*K1])
Relations.append([MC_inv_blockmatrix[1]*K1])
Relations.append([MC_inv_blockmatrix[2]*K1])

RelTot = block_matrix(11,Relations)

ker = RelTot.right_kernel()


# Lower trail
# 7,14  => 0,1
Relations = []

t = 2
for i in range(t):
    Relations.append([Key(7,i,t,model)])
    Relations.append([Key(8,i,t,model)])
P = [1,0]
K6 = block_matrix(4,[[Key(6,1,t,model)],[Key(6,0,t,model)],[ZeroFunction(t,model)],[ZeroFunction(t,model)]])
Relations.append([MC_inv_blockmatrix[2]*K6])
RelTot2 = block_matrix(5,Relations)

ker2 = RelTot2.right_kernel()

final_el = 0
final_el2 = 0
count = 0
for el in ker:
        if el.is_zero():
            continue
        if getKeyElement(el,0,5,4,model) != F.fetch_int(0xc6):
            continue
        for el2 in ker2:
            if el2.is_zero():
                continue
            x = getKeyElement(el,0,5,4,model)
            Column = Matrix(4,[[getKeyElement(el2,1,6,2,model)],[getKeyElement(el2,0,6,2,model)],[F.fetch_int(0)],[F.fetch_int(0)]])
            y = (MC_inv*Column)[3][0]
            if(BCT[x.integer_representation()][y.integer_representation()] == 6):
                final_el=el
                final_el2=el2
                break
                #prettyPrint(el,4,model,"Delta")
                #prettyPrint(el2,2,model,"Nabla")
        if final_el != 0:
            break
'''

# TK3_fromCiphertext_5_7
# Forward trail: 4 active tweakey bytes
# el = VS96([1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0])
# el2 = VS72([1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1])
    

'''
Relations=[]
t = 4
model = 3
# Tweakey material of round 2 and 3 is inactive
for i in range(t):
    Relations.append([Key(2,i,t,model)])
    Relations.append([Key(3,i,t,model)])
# Difference in x_0 retracts to 1 byte of z_0
# Tweakey indexes at round 1, from top to bottom => 1,2,3,0
# Round 1:
P = [1,2,3,0]
K1 = block_matrix(4,[[Key(1,P[i],t,model)] for i in range(t)])
Relations.append([MC_inv_blockmatrix[1]*K1])
Relations.append([MC_inv_blockmatrix[2]*K1])
Relations.append([MC_inv_blockmatrix[3]*K1])

RelTot = block_matrix(11,Relations)

ker = RelTot.right_kernel()

# Backward trail: 3 active tweakey bytes
Relations=[]
t = 3
# Tweakey material of round 8 and 9 is inactive
for i in range(t):
    Relations.append([Key(8,i,t,model)])
    Relations.append([Key(9,i,t,model)])
#tweakey material from round 7 should retract in round 6
P = [2,1,0]
K7 = block_matrix(4,[[Key(7,P[0],t,model)],[ZeroFunction(t,model)],[Key(7,P[1],t,model)],[Key(7,P[2],t,model)]])
Relations.append([MC_inv_blockmatrix[1]*K7])

RelTot2 = block_matrix(7,Relations)

ker2 = RelTot2.right_kernel()


bk = False
counter=0
#Due to memory leaks, we need to store the Ker elements
KerArray = []
Ker2Array = []

for el in ker:
    if el.is_zero():
        continue
    x1 = getKeyElement(el,2,5,4,model).integer_representation()
    x2 = getKeyElement(el,3,5,4,model).integer_representation()
    x3 = getKeyElement(el,0,5,4,model).integer_representation()
    KerArray.append([el,x1,x2,x3])


for el2 in ker2:
    if el2.is_zero():
        continue

    tk63 = getKeyElement(el2,1,6,3,model)
    #y6[2] is linearly dependant of tk6[3]. 14*y6[2] + 11*tk6[3] = 0 <=> tk6[3] *11/14 = y6[2]
    y62 = tk63*F.fetch_int(11)/F.fetch_int(14)
    Column3 = Matrix(4,[[F.fetch_int(0)],[F.fetch_int(0)],[y62],[tk63]])
    y3 = (MC_inv*Column3)[3,0]

    tk64 = getKeyElement(el2,0,6,3,model)
    #y6[7] is linearly dependant of tk6[4]. 13*y6[2] + 9*tk6[3] = 0 <=> tk6[3] *9/13 = y6[2]
    y67 = tk64*F.fetch_int(9)/F.fetch_int(13)
    Column2 = Matrix(4,[[tk64],[F.fetch_int(0)],[F.fetch_int(0)],[y67]])
    y2 = (MC_inv*Column2)[2,0]

    tk69 = getKeyElement(el2,2,6,3,model)
    #y6[8] is linearly dependant of tk6[9]. 14*y6[2] + 11*tk6[3] = 0 <=> tk6[3] *11/14 = y6[2]
    y68 = tk69*F.fetch_int(11)/F.fetch_int(14)
    Column1 = Matrix(4,[[y68],[tk69],[F.fetch_int(0)],[F.fetch_int(0)]])
    y1 = (MC_inv*Column1)[1,0]
    #Artificially remove tweakey candidates equal to 0
    if(y3==F.fetch_int(0) or y2==F.fetch_int(0) or y1==F.fetch_int(0)):
        continue
    y1 = y1.integer_representation()
    y2 = y2.integer_representation()
    y3 = y3.integer_representation()

    tk78 = getKeyElement(el2,2,7,3,model)
    tk710 = getKeyElement(el2,1,7,3,model)
    tk711 = getKeyElement(el2,0,7,3,model)
    Col = Matrix(4,[[tk78],[F(0)],[tk710],[tk711]])
    z6 = MC_inv*Col
    z68 = z6[0,0]
    z62 = z6[2,0]
    z67 = z6[3,0]
    ddt = DDT[y68.integer_representation(),z68.integer_representation()]*DDT[y67.integer_representation(),z67.integer_representation()]*DDT[y62.integer_representation(),z62.integer_representation()]

        
    if(ddt!=0):
        Ker2Array.append([el2,y1,y2,y3,ddt])


print(len(Ker2Array))
for l in KerArray:
    for l2 in Ker2Array:
        counter+=1
        el,x1,x2,x3 = l[0],l[1],l[2],l[3]
        el2,y1,y2,y3,ddt= l2[0],l2[1],l2[2],l2[3],l2[4]
        if(BCT[x1,y1]*BCT[x2,y2]*BCT[x3,y3]*ddt*ddt> vmax):
                vmax = BCT[x1,y1]*BCT[x2,y2]*BCT[x3,y3]*ddt*ddt
                final_el=el
                final_el2=el2
                f.write(str(vmax)+","+str(el) + ","+ str(el2)+"\n")
                print(str(vmax))
                if(vmax ==6*6*6*4*4*4):
                    bk = True
                    break

                #prettyPrint(el,4,model,"Delta")
                #prettyPrint(el2,2,model,"Nabla")    
        if(counter%100000 == 0):
            print(counter) 


            


'''
'''

# TK3_fromPlaintext_6_7
# A CHANGER CE NEST PAS LE BON TRAIL
# Forward trail: 3 active tweakey bytes
# Position 5,12,14
t = 3
model = 3
Relations=[]
# Tweakey material of round 2 and 3 is inactive
for i in range(t):
    Relations.append([Key(3,i,t,model)])
    Relations.append([Key(4,i,t,model)])
# Difference in x_1 retracts to 2 byte of z_1
# Round 2:
# Positions in the third column of K2: P = [14,-,12,5]
K2 = block_matrix(4,[[Key(2,2,t,model)],[ZeroFunction(t,model)],[Key(2,1,t,model)],[Key(2,0,t,model)]])
Relations.append([MC_inv_blockmatrix[0]*K2])
Relations.append([MC_inv_blockmatrix[1]*K2])

RelTot = block_matrix(8,Relations)

ker = RelTot.right_kernel()

# Backward trail : 2 active tweakey bytes
# Position 10,11

Relations = []
t=2
for i in range(t):
    Relations.append([Key(9,i,t,model)])
    Relations.append([Key(10,i,t,model)])

# x_7 retracts to 3 active bytes in z_7
# Position ok the tweakey bytes in the column: [-,-,10,11]
K8 = block_matrix(4,[[ZeroFunction(t,model)],[ZeroFunction(t,model)],[Key(8,0,t,model)],[Key(8,1,t,model)]])
Relations.append([MC_inv_blockmatrix[0]*K8])


RelTot2 = block_matrix(8,Relations)

ker2 = RelTot2.right_kernel()
KerArray = []
Ker2Array = []
for el in ker:
    if el.is_zero():
        continue
    x1 = getKeyElement(el,2,6,3,model).integer_representation()
    x2 = getKeyElement(el,0,5,3,model).integer_representation()
    KerArray.append([el,x1,x2])

t=2
for el2 in ker2:
    
    if el2.is_zero():
        continue

    tk73 = getKeyElement(el2,0,7,t,model)
    #y7[2] is linearly dependant of tk7[3]. 9*y7[2] + 14*tk7[3] = 0 <=> tk7[3] *9/14 = y7[2]
    y72 = tk73*F.fetch_int(9)/F.fetch_int(14)
    Column1 = Matrix(4,[[F.fetch_int(0)],[F.fetch_int(0)],[y72],[tk73]])
    y1 = (MC_inv*Column3)[0,0]

    tk74 = getKeyElement(el2,1,7,t,model)
    #y6[7] is linearly dependant of tk6[4]. 13*tk7[4] + 11*y7[7] = 0 <=> tk7[4] *11/13 = y7[7]
    y77 = tk74*F.fetch_int(11)/F.fetch_int(13)
    Column2 = Matrix(4,[[tk74],[F.fetch_int(0)],[F.fetch_int(0)],[y77]])
    y2 = (MC_inv*Column2)[3,0]

    y1 = y1.integer_representation()
    y2 = y2.integer_representation()

    tk810 = getKeyElement(el2,0,7,t,model)
    tk811 = getKeyElement(el2,1,7,t,model)
    Col = Matrix(4,[[F(0)],[F(0)],[tk810],[tk811]])
    z7 = MC_inv*Col
    z72 = z7[2,0]
    z77 = z7[3,0]
    ddt = DDT[y72.integer_representation(),z72.integer_representation()]*DDT[y77.integer_representation(),z77.integer_representation()]

        
    if(ddt!=0):
        Ker2Array.append([el2,y1,y2,ddt])

for l in KerArray:
    for l2 in Ker2Array:
        el,x1,x2, = l[0],l[1],l[2]
        el2,y1,y2,ddt= l2[0],l2[1],l2[2],l2[3]
        if(BCT[x1,y1]*BCT[x2,y2]*ddt*ddt> vmax):
                vmax = BCT[x1,y1]*BCT[x2,y2]*ddt*ddt
                final_el=el
                final_el2=el2
                f.write(str(vmax)+","+str(el) + ","+ str(el2)+"\n")
                print(str(vmax))
                if(vmax ==6*6*6*4*4*4):
                    bk = True
                    break

                #prettyPrint(el,4,model,"Delta")
                #prettyPrint(el2,2,model,"Nabla")    
        if(counter%100000 == 0):
            print(counter) 


#Main part: given optimal vectors el and el2, print into "resultfile" (= argv[1]) the real values of tweakey differences for all position, and the key value. 
#Result file is to be processed by the BeautifulBoomerang.py script afterwards

'''



# TK2_fromCiphertext_4_4
# 5 active bytes in the tweakey material
# Active tweakey bytes : [0,2,9,11,13]
# el = (0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0)
# el2 = (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1)

'''
Relations=[]
t = 5 # [0,2,9,11,13]
model = 2
# Only the 13th byte of round 1 is active in the tweakey
for i in range(4):
    Relations.append([Key(1,i,t,model)])

Relations.append([Key(2,4,t,model)])
# Difference in x_1 retracts to 1 byte of z_1
# Tweakey indexes at round 2, from top to bottom (4th column) => 9,0,11,2
# Round 1:
P = [1,3,0,2]
K2 = block_matrix(4,[[Key(2,P[i],t,model)] for i in range(4)])
Relations.append([MC_inv_blockmatrix[1]*K2])
Relations.append([MC_inv_blockmatrix[2]*K2])
Relations.append([MC_inv_blockmatrix[3]*K2])

RelTot = block_matrix(8,Relations)

ker = RelTot.right_kernel()

for el in ker:
    if DDT[getKeyElement(el,4,1,5,2).integer_representation(),getKeyElement(el,0,2,5,2).integer_representation()] == 4:
        print(el)
        break


# Lower trail
# 1 active byte
# 0 -> 0

Relations = []

t = 1
model=2

Relations.append([Key(5,0,t,model)])

RelTot2 = block_matrix(1,Relations)
ker2 = RelTot2.right_kernel()

for el2 in ker2:
    if el2 == 0:
        continue
    print(el2)
    break



model=2
maxel = el
maxel2 = el2
kP = [0,2,9,11,13]
kP2 = [0]
ru = 4
rl = 4
'''


# TK2_fromCiphertext_4_5
# 1 active bytes in the tweakey material
# Active tweakey bytes : [11]
# el = (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0)
# el2 = (0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0)

'''
Relations=[]
t = 1
model = 2

Relations.append([Key(2,0,t,model)])

RelTot = block_matrix(1,Relations)

ker = RelTot.right_kernel()

#for el in ker:
#    if el.is_zero():
#        continue
#    print(el)
#    break


# Lower trail
# 1 active byte
# [5] -> index 0

Relations = []

t = 1
model=2

Relations.append([Key(6,0,t,model)])

RelTot2 = block_matrix(1,Relations)
ker2 = RelTot2.right_kernel()

for el2 in ker2:
    if el2 == 0:
        continue
    x412 = getKeyElement(el2,0,5,t,model)
    Col = Matrix(4,[[x412],[F.fetch_int(0)],[F.fetch_int(0)],[F.fetch_int(0)]])
    z412 = (MC_inv*Col)[0,0]
    # 14 y412 + 11 tk413 = 0 => y412 = 11/14 * tk413
    y412 = getKeyElement(el2,0,4,t,model)*F.fetch_int(11)/F.fetch_int(14) 
    if(DDT[y412.integer_representation(),z412.integer_representation()] == 4):
        print(el2)
        break


model=2
maxel = (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0)
maxel2 = (0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0)
kP = [11]
kP2 = [5]
ru = 4
rl = 5
'''


# TK1 fromCiphertext 4_4
# Upper trail
# 1 active byte in position 7
# el = (1, 0, 0, 0, 0, 0, 0, 0)
# el2 = (1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1)
'''
t = 1
model = 1

# Zero constraints
Relations = [ZeroFunction(1,1)]


RelTot = block_matrix(1,Relations)

ker = RelTot.right_kernel()

for el in ker:
    if el.is_zero():
        continue
    print(el)
    break

# Lower trail
# 2 active bytes [5,15]
t=2
model = 1
# 
Relations = []
TK5 = block_matrix(4,[[Key(5,0,t,model)],[ZeroFunction(t,model)],[Key(5,1,t,model)],[ZeroFunction(  t,model)]])
Relations.append(MC_inv_blockmatrix[3]*TK5)

RelTot2 = block_matrix(1,Relations)

ker2 = RelTot2.right_kernel()
ddtmax = 0
el2max = 0
for el2 in ker2:
    if el2.is_zero():
        continue

    #13 y4[6] = 9 tk4[7]
    y46 = getKeyElement(el2,1,4,2,1)*F.fetch_int(9)/F.fetch_int(13)

    #13 y4[12] = 9 tk4[13]
    y412 = getKeyElement(el2,0,4,2,1)*F.fetch_int(9)/F.fetch_int(13)

    x4Col = Matrix(4,[[getKeyElement(el2,0,5,2,1)],[F.fetch_int(0)],[getKeyElement(el2,1,5,2,1)],[F.fetch_int(0)]])
    Inv = MC_inv*x4Col

    ddt = DDT[y46.integer_representation(),Inv[2,0].integer_representation()]*DDT[y412.integer_representation(),Inv[0,0].integer_representation()]
    if(ddt >= ddtmax):
        ddtmax = ddt
        print(ddtmax)
        el2max = el2

print(el2max)

model=1
maxel = (1, 0, 0, 0, 0, 0, 0, 0)
maxel2 = (1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1)
kP = [7]
kP2 = [5,15]
ru = 4
rl = 4
'''

# TK3 fromCiphertext 5_5
# Upper trail
# 4 active byte in position [2,7,8,13]
# el = (0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1)

# el2 = (1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1)
'''
t = 4
model = 3

for i in range(t):
    Relations.append(Key(2,i,t,model))
    Relations.append(Key(3,i,t,model))

P = [3,0,1,2]
Col = block_matrix(4,[[Key(1,P[i],t,model)] for i in range(4)])
Relations.append(MC_inv_blockmatrix[0]*Col)
Relations.append(MC_inv_blockmatrix[1]*Col)
Relations.append(MC_inv_blockmatrix[2]*Col)

RelTot = block_matrix(11,Relations)

ker = RelTot.right_kernel()


# Lower trail
# 2 active bytes [0,11]
t=2
model = 3
# 
Relations = []
for i in range(t):
    Relations.append(Key(7,i,t,model))
    Relations.append(Key(8,i,t,model))

TK6 = block_matrix(4,[[ZeroFunction(t,model)],[Key(6,1,t,model)],[Key(6,0,t,model)],[ZeroFunction(t,model)]])
Relations.append(MC_inv_blockmatrix[3]*TK6)

RelTot2 = block_matrix(5,Relations)

ker2 = RelTot2.right_kernel()
finished = 0
for el2 in ker2:
    if el2.is_zero():
        continue
    for el in ker:
        y5 = getKeyElement(el,3,5,4,model)
        if el.is_zero():
            continue

    
        x5Col = Matrix(4,[[F.fetch_int(0)],[getKeyElement(el2,1,6,t,model)],[getKeyElement(el2,0,6,t,model)],[F.fetch_int(0)]])
        Inv = MC_inv*x5Col

        bct = BCT[y5.integer_representation(),Inv[0,0].integer_representation()]
        if(bct == 6):
            print(el)
            print(el2)
            finished = 1
            break
    if finished:
        break
'''

# TK3 fromCiphertext 6 7 with equal bytes
# Upper trail
# 2 active byte in position [7,12]
# el 
# el2 
'''
t = 2
model = 3
Relations = []
for i in range(t):
    Relations.append(Key(3,i,t,model))
    Relations.append(Key(4,i,t,model))


TK2 = block_matrix(4,[[ZeroFunction(t,model)],[Key(2,0,t,model)],[Key(2,1,t,model)],[ZeroFunction(t,model)]])

Relations.append(MC_inv_blockmatrix[3]*TK2)

RelTot = block_matrix(5,Relations)

ker = RelTot.right_kernel()


# Lower trail
# 2 active bytes [1,2]
t=2
model = 3
# 
Relations = []
for i in range(t):
    Relations.append(Key(9,i,t,model))
    Relations.append(Key(10,i,t,model))

TK8 = block_matrix(4,[[ZeroFunction(t,model)],[Key(8,0,t,model)],[Key(8,1,t,model)],[ZeroFunction(t,model)]])
Relations.append(MC_inv_blockmatrix[3]*TK8)

RelTot2 = block_matrix(5,Relations)

ker2 = RelTot2.right_kernel()

maxddt = 0
maxel = ()
maxel2 = ()

for el2 in ker2:
    if el2.is_zero():
        continue
    tk81 = getKeyElement(el2,0,8,t,model)
    tk82 = getKeyElement(el2,1,8,t,model)
    MM = Matrix(4,[[F.fetch_int(0)],[tk81],[tk82],[F.fetch_int(0)]])
    Inv = MC_inv*MM
    z75 = Inv[1,0]
    z710 = Inv[2,0]

    tk711 = getKeyElement(el2,1,7,t,model)
    y710 = tk711*F.fetch_int(13)/F.fetch_int(11)
    tk76 = getKeyElement(el2,0,7,t,model)
    y75 = tk76*F.fetch_int(14)/F.fetch_int(9)
    ddt = DDT[y75.integer_representation(),z75.integer_representation()]*DDT[y710.integer_representation(),z710.integer_representation()]
    if(ddt == 0):
        continue
    x6Col2 = Matrix(4,[[F.fetch_int(0)],[F.fetch_int(0)],[y710],[tk711]])
    z62 = (MC_inv*x6Col2)[2,0]
    
    for el in ker:
        x1Col = Matrix(4,[[F.fetch_int(0)],[getKeyElement(el,0,2,t,model)],[getKeyElement(el,1,2,t,model)],[F.fetch_int(0)]])
        Inv = MC_inv*x1Col
        z113 = Inv[1,0]
        tk114 = getKeyElement(el,0,2,t,model)
        y113 = tk114*F.fetch_int(11)/F.fetch_int(14)
        y6 = getKeyElement(el,0,6,t,model)
        if el.is_zero():
            continue
    
        

        bct = BCT[y6.integer_representation(),z62.integer_representation()]
        ddt2 = DDT[y113.integer_representation(),z113.integer_representation()]
        if(bct*ddt*ddt*ddt2*ddt2 >= maxddt):
            maxel = el
            maxel2 = el2
            maxddt = bct*ddt*ddt*ddt2*ddt2
            print(maxddt)
'''

# TK3 fromCiphertext 6 7
# Upper trail
# 3 active byte in position [3,8,10]
# el 
# el2 
'''
maxel = (0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1)
maxel2 = (1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1)
rl = 8
ru = 6
kP = [3,8,10]
model = 3
kP2 = [5,6,7]


t = 3
model = 3
Relations = []
for i in range(t):
    Relations.append(Key(3,i,t,model))
    Relations.append(Key(4,i,t,model))


TK2 = block_matrix(4,[[Key(2,2,t,model)],[Key(2,0,t,model)],[Key(2,1,t,model)],[ZeroFunction(t,model)]])

Relations.append(MC_inv_blockmatrix[3]*TK2)
Relations.append(MC_inv_blockmatrix[1]*TK2)

RelTot = block_matrix(8,Relations)

ker = RelTot.right_kernel()


# Lower trail
# 3 active bytes [5,6,7]

t=3
model = 3
# 
Relations = []
for i in range(t):
    Relations.append(Key(9,i,t,model))
    Relations.append(Key(10,i,t,model))

TK8 = block_matrix(4,[[ZeroFunction(t,model)],[Key(8,0,t,model)],[Key(8,1,t,model)],[Key(8,2,t,model)]])
Relations.append(MC_inv_blockmatrix[0]*TK8)
Relations.append(MC_inv_blockmatrix[1]*TK8)

RelTot2 = block_matrix(8,Relations)

ker2 = RelTot2.right_kernel()

maxddt = 0
maxel = ()
maxel2 = ()

for el2 in ker2:
    if el2.is_zero():
        continue
    tk85 = getKeyElement(el2,0,8,t,model)
    tk86 = getKeyElement(el2,1,8,t,model)
    tk87 = getKeyElement(el2,2,8,t,model)
    MM = Matrix(4,[[F.fetch_int(0)],[tk85],[tk86],[tk87]])
    Inv = MC_inv*MM
    z714 = Inv[2,0]
    z73 = Inv[3,0]

    tk70 = getKeyElement(el2,2,7,t,model)
    y73 = tk70 * F.fetch_int(13)/F.fetch_int(11)
    tk715 = getKeyElement(el2,1,7,t,model)
    y714 = tk715*F.fetch_int(14)/F.fetch_int(9)
    tk710 = getKeyElement(el2,0,7,t,model)
    z68 = tk710*F.fetch_int(13)
    z613 = tk710*F.fetch_int(11)
    y68 = getKeyElement(el2,1,6,3,3)

    z612 = (MC_inv*getColumn(0,0,y714.integer_representation(),tk715.integer_representation()))[0,0]
    ddt = DDT[y73.integer_representation(),z73.integer_representation()]*DDT[y714.integer_representation(),z714.integer_representation()]*DDT[y68.integer_representation(),z68.integer_representation()]
    if(ddt == 0):
        continue
    
    for el in ker:
        if el.is_zero():
            continue
        tk612 = getKeyElement(el,2,6,t,model)
        tk613 = getKeyElement(el,0,6,t,model)
    
        

        bct = BCT[tk612.integer_representation(),z612.integer_representation()]*BCT[tk613.integer_representation(),z613.integer_representation()]
        if(bct*ddt*ddt > maxddt):
            maxel = el
            maxel2 = el2
            maxddt = bct*ddt*ddt
            print(h(y73) + h(z73) + h(y714) + h(z714) + h(y68) + h(z68) + h(z612) + h(z613))
            print(maxddt)


'''

# Deoxys_BC_TK2 5 5 fC
# Upper trail: 5 active bytes: [0,2,5,11,15]
# NOT BETTER THAN PREVIOUS TRAILS
'''
t = 5
model = 2
Relations = []

Relations.append(Key(1,2,t,model))
Relations.append(Key(2,4,t,model))
Relations.append(Key(3,0,t,model))
Relations.append(Key(3,3,t,model))
Relations.append(Key(4,1,t,model))



TK3 = block_matrix(4,[[Key(3,4,t,model)],[ZeroFunction(t,model)],[Key(3,2,t,model)],[Key(3,1,t,model)]])

Relations.append(MC_inv_blockmatrix[0]*TK3)
Relations.append(MC_inv_blockmatrix[1]*TK3)

TK24 = block_matrix(4,[[Key(2,1,t,model)],[Key(2,0,t,model)],[Key(2,3,t,model)],[Key(2,1,t,model)]])

Relations.append(MC_inv_blockmatrix[0]*TK24)
Relations.append(MC_inv_blockmatrix[1]*TK24)
Relations.append(MC_inv_blockmatrix[3]*TK24)





RelTot = block_matrix(10,Relations)

ker = RelTot.right_kernel()


# Lower trail
# 3 active bytes [4,13]

t=2
model = 2
# 
Relations = []

Relations.append(Key(7,0,t,model))
Relations.append(Key(7,1,t,model))

TK6 = block_matrix(4,[[ZeroFunction(t,model)],[ZeroFunction(t,model)],[Key(6,0,t,model)],[Key(6,1,t,model)]])
Relations.append(MC_inv_blockmatrix[0]*TK6)


RelTot2 = block_matrix(3,Relations)

ker2 = RelTot2.right_kernel()

maxddt = 0
maxddt2 = 0
maxel = ()
maxel2 = ()

for el2 in ker2:
    if el2.is_zero():
        continue

    tk610 = getKeyElement(el2,0,6,t,model)
    tk611 = getKeyElement(el2,1,6,t,model)
    Col = Matrix(4,[[F.fetch_int(0)],[F.fetch_int(0)],[tk610],[tk611]])
    z57 = (MC_inv*Col)[3,0]

    tk54 = getKeyElement(el2,1,5,t,model)

    y57 = tk54*F.fetch_int(11)/F.fetch_int(14)
    ddt2 = DDT[y57.integer_representation(),z57.integer_representation()]
    if ddt2 > maxddt2:
        maxel2 = el2
        maxddt2=ddt2
        if ddt2 == 4:
            break
print(maxddt2)
print(maxel)
print(maxel2)
t = 5
s=0
for el in ker:
    if el.is_zero():
        continue

    tk30 = getKeyElement(el,4,3,t,model)
    tk32 = getKeyElement(el,2,3,t,model)
    tk33 = getKeyElement(el,1,3,t,model)
    Col = Matrix(4,[[tk30],[F.fetch_int(0)],[tk32],[tk33]])

    z210 = (MC_inv*Col)[2,0]
    z215 = (MC_inv*Col)[3,0]

    tk212 = getKeyElement(el,1,2,t,model)
    y215 = tk212
    z16 = tk212
    tk113 = getKeyElement(el,1,1,t,model)
    y113 = tk113
    tk12 = getKeyElement(el,3,1,t,model)
    y12 = tk12      
    tk16 = getKeyElement(el,4,1,t,model)
    y16 = tk16
    tk17 = getKeyElement(el,0,1,t,model)
    y17 = tk17
    tk211 = getKeyElement(el,2,2,t,model)
    y210 = tk211*F.fetch_int(9)/F.fetch_int(13)

    Col = Matrix(4,[[F.fetch_int(0)],[F.fetch_int(0)],[y210],[tk211]])

    z12 = (MC_inv*Col)[2,0]
    z17 = (MC_inv*Col)[3,0]
    z113 = (MC_inv*Col)[1,0]


    ddt = getDDT(y16,z16)*getDDT(y12,z12)*getDDT(y17,z17)*getDDT(y113,z113)*getDDT(y210,z210)*getDDT(y215,z215)
    
    if ddt > maxddt:
        maxddt = ddt
        maxel = el



'''

# Deoxys_BC_TK2 4 6 fC
# Upper trail: 1 active byte: [1]
# (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0)
# (1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0)
'''
ru = 4
rl = 6
kP = [1]

t1 = 1
model = 2
Relations = []

Relations.append(Key(2,0,t1,model))
RelTot = block_matrix(1,Relations)
ker = RelTot.right_kernel()

for el in ker:
    if el.is_zero():
        continue
    maxel = el
    break
# lower trail: 2 active bytes [5,14]
Relations = []
t = 2
kP2 = [5,14]
Relations.append(Key(7,0,t,model))
Relations.append(Key(7,1,t,model))

TK6 = block_matrix(4,[[Key(6,1,t,model)], [ZeroFunction(t,model)],[ZeroFunction(t,model)],[Key(6,0,t,model)]])

Relations.append(MC_inv_blockmatrix[1]*TK6)
RelTot2 = block_matrix(3,Relations)
ker2 = RelTot2.right_kernel()

maxddt = 0
for el2 in ker2:
    if el2.is_zero():
        continue

    tk60 = getKeyElement(el2,1,6,t,model)
    tk63 = getKeyElement(el2,0,6,t,model)

    w5Col = Matrix(4,[[tk60],[F.fetch_int(0)],[F.fetch_int(0)],[tk63]])
    y50 = (MC_inv*w5Col)[0,0]
    y515 = (MC_inv*w5Col)[3,0]

    tk51 = getKeyElement(el2,1,5,t,model)
    tk512 = getKeyElement(el2,0,5,t,model)
    x50 = F.fetch_int(14)*tk51/F.fetch_int(9)
    x515 = F.fetch_int(13)*tk512/F.fetch_int(11)
    ddt = getDDT(x50,y50)*getDDT(x515,y515)
    if ddt > maxddt:
        print(ddt)
        maxddt = ddt
        maxel2 = el2
'''



# Deoxys_TK1_fP_5_4
# el = (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1)
# el2 = (1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1)

ru = 5
rl = 4
# Upper trail: 2 active bytes [0,2]
kP = [0,2]
t=2
model=1
Relations = []
TK2 = block_matrix(4,[[Key(2,1,t,model)],[ZeroFunction(t,model)],[Key(2,0,t,model)],[ZeroFunction(t,model)]])
Relations.append((MC_inv_blockmatrix[3]*TK2))
RelTot = block_matrix(1,Relations)
ker = RelTot.right_kernel()

#Lower trail: 2 active bytes [8,10]
kP2 = [8,10]
t = 2
Relations = []
TK6 = block_matrix(4,[[Key(2,1,t,model)],[ZeroFunction(t,model)],[Key(2,0,t,model)],[ZeroFunction(t,model)]])
Relations.append((MC_inv_blockmatrix[1]*TK6))
RelTot2 = block_matrix(1,Relations)
ker2 = RelTot2.right_kernel()

for el in ker:
    if el.is_zero():
        continue
    maxel = el
    break
maxddt2 = 0
for el2 in ker2:
    if el2.is_zero():
        continue
    
    tk57 = getKeyElement(el2,0,5,t,model)
    tk612 = getKeyElement(el2,1,6,t,model)
    tk614 = getKeyElement(el2,0,6,t,model)
    Colw = Matrix(4,[[tk612],[F.fetch_int(0)],[tk614],[F.fetch_int(0)]])
    y56 = (MC_inv*Colw)[2,0]
    x56 = tk57*F.fetch_int(9)/F.fetch_int(13)
    ddt = getDDT(x56,y56)

    if(ddt > maxddt2):
        maxddt2 = ddt
        maxel2 = el2 

el = maxel
el2 = maxel2

# Example: Writing concrete differences of 9-round Deoxys in the RTK1 model in the file given in first argument

if(__name__ == "__main__"):
    if len(argv) <= 1:
        print("Usage: sage script resultfile")
    else:
        f = open(argv[1],"w")
        printInFile(f, el,el2,model,kP,kP2,ru,rl)
        # Another example: we can also give already computed key values as input instead of re-computing them:
        #kP = [0,2,5,11,15]
        #kP2 = [4,13]
        #ru = 5
        #rl = 5
        #el = (0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1)
        #el2 = (1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1)
        #printInFile(f, el,el2,model,kP,kP2,ru,rl)
        


