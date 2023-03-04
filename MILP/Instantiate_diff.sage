from sys import argv
from sage.crypto.sboxes import AES as SBox


#AES parameters


F.<a> = GF(2^8, modulus=x^8+x^4+x^3+x+1) # AES field

V = VectorSpace(F,4)
MC = Matrix(F,4,4,[F.fetch_int(i) for i in [2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2]]) # AES MixColumns
MC_inv = MC.inverse()

MC_inv_blockmatrix = [block_matrix(1,4,[el.matrix() for el in MC_inv[i]])  for i in range(4)]


M = block_matrix(4,4,[[F.fetch_int(i).matrix() for i in [2,3,1,1]],[F.fetch_int(i).matrix() for i in [1,2,3,1]],[F.fetch_int(i).matrix() for i in [1,1,2,3]],[F.fetch_int(i).matrix() for i in [3,1,1,2]]])

MS = MatrixSpace(GF(2),8,8)

Zero = MS(0)
One = MS(1)

DDT = SBox.difference_distribution_table()
BCT = SBox.boomerang_connectivity_table()


F2x.<z> = GF(2)[] # Polynomials over GF(2)
LFSR2_matrix = companion_matrix(z^8+z^2+1,format='right').antitranspose()

LFSR3_matrix = LFSR2_matrix.inverse()

Key_Permutation = [1,6,11,12, 5,10,15,0, 9,14,3,4, 13,2,7,8]

Key_Permutation_inv = [0 for i in range(16)]

for i in range(16):
    Key_Permutation_inv[Key_Permutation[i]] = i


#Utility functions


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
# tweakey = [(TK1)_0,(TK1)_1,(TK1)_2,(TK1)_3,(TK2)_0 ... (TK3)_3] of len i*number of active tweakey bits for TKi        

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

# From an element v of the kernel, returns the i-th tweakey byte difference of round r, in the model model.
def getKeyElement(v,i,r,model):
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

# Iterate r times on the Tweakey permutation
def KeyPermutationIteration(i,r):
    for k in range(r):
        i = Key_Permutation[i]
    return i

def ZeroFunction(t,model):
    M = []
    for j in range(t*model):
        M.append(Zero) 
    return block_matrix(1,[M])


# Automated generation of optimal tweakey values


# The following function prints the tweakey difference in the file f.
# [model,ru,rl] encode the model  and number of rounds of the boomerang attack.
# [el,el2,keyPositions,keyPositions2] encode the tweakey differences and positions.
def printInFile(f, el,el2,model,keyPositions,keyPositions2,ru,rl):
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
                f.write("utkv" + str(16*r + i) + " " + "{:02x}".format(getKeyElement(el,index,r,model).integer_representation()) + "\n") 

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
                f.write("ltkv" + str(16*r + i) + " " + "{:02x}".format(getKeyElement(el2,index,r+ru-1,model).integer_representation()) + "\n") 

        for k in range(len(keyPositions2)):
            keyPositions2[k] = Key_Permutation_inv[keyPositions2[k]]
    uk = returnKey(el,t,model,kP)
    lk = returnKey(el2,t2,model,kP2)
    for k in range(model):
        for i in range(16):
            f.write("deltautk"+str(k*16+i) + " " + uk[k][i]+"\n")
            f.write("deltaltk"+str(k*16+i) + " " + lk[k][i]+"\n")



# Example of optimal tweakey generation for the 11-round attack on Deoxys-BC in the RTK3 model.
# -----------------------------------------------------------------------------------------------

# To get the figure squeleton: 
# Run:
#
# make build
# ./Truncated_boomerang_MILP -d3 -c 5 7
# make pdf
#
# WARNING: This model solving take some time (more than several hours on 8 threads).
#
# The following difference instantiation takes several minutes (with 1 thread).
#
# Figure squeleton is available in output/Deoxys_TK3_boomerang_fromCiphertext_5R_7R.pdf
# The figure squeleton corresponds to the 12-round attack on Deoxys-BC in the RTK3 model of the full version of the paper (on eprint).

model = 3

# Upper trail 

upper_rounds = 5

# The tweakey differences are subject to linear relations in two cases:
#  - When a particular round tweakey is inactive
#  - When the inverse MixColumn applied to a round tweakey column cancels out in any position.
# Each linear relation is added to the list Relation.

upper_relations = []


# The squeleton has 4 active upper tweakey bytes in positions [0,5,10,15].

upper_tweakey_positions = [0,5,10,15]
upper_tweakey_len = len(upper_tweakey_positions)

# A relation is encoded as a 8x(8*model*upper_tweakey_len) matrix, corresponding to a 8x8 block matrix for each active position of each (\Delta TK^i), i<model.

# In our example, \Delta TK^0[0] is encoded as:

# \Delta TK^0[0]    \Delta TK^0[5]   \Delta TK^0[10]  \Delta TK^0[15] \Delta TK^1[0]    \Delta TK^1[5]   \Delta TK^1[10]  \Delta TK^1[15] \Delta TK^2[0]    \Delta TK^2[5]   \Delta TK^2[10]  \Delta TK^2[15]
#
# 1 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0
# 0 1 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0
# 0 0 1 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0
# 0 0 0 1 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0
# 0 0 0 0 1 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0
# 0 0 0 0 0 1 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 1 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0
# 0 0 0 0 0 0 0 1  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0

# No need to bother with this encoding. 
# In practice, we want to get the representation of a round tweakey of round r and of index i in order to create a linear relation.
# To get the matrix of the k-th byte of the round tweakey of round r:
# First determine the position in the base tweakey that it corresponds to: it is x = Key_Permutation^(r) [k]
# Then, determine i = upper_tweakey_positions.index(x)
# Then, call Key(r,i,upper_tweakey_len,model)


# The upper tweakey is inactive in round 2 and 3. Add the corresponding linear relations.

for i in range(upper_tweakey_len):
    upper_relations.append([Key(2,i,upper_tweakey_len,model)])
    upper_relations.append([Key(3,i,upper_tweakey_len,model)])

# Difference in w_0[4,5,6,7] retracts to y_0[4]
# Tweakey byte positions at round 1, from top to bottom => [4,5,6,7]
# Tweakey indexes at round 1, from top to bottom => [1,2,3,0]
# Round 1:
upper_active_bytes_R1 = [4,5,6,7]
upper_base_tweakey_indexes_for_R1 = [upper_tweakey_positions.index(Key_Permutation[upper_active_bytes_R1[i]]) for i in range(len(upper_active_bytes_R1))]

assert(upper_base_tweakey_indexes_for_R1 == [1,2,3,0])

K1 = block_matrix(4,[[Key(1,upper_base_tweakey_indexes_for_R1[i],upper_tweakey_len,model)] for i in range(4)])

# Condition y_0[5] = 0
upper_relations.append([MC_inv_blockmatrix[1]*K1])

# Condition y_0[6] = 0
upper_relations.append([MC_inv_blockmatrix[2]*K1])

# Condition y_0[7] = 0
upper_relations.append([MC_inv_blockmatrix[3]*K1])

# Now compute the kernel of the matrix. Every element satisfies all the linear relations that were added in upper_relations.
upper_ker = block_matrix(upper_relations).right_kernel()


# Lower trail

lower_rounds = 7

# The squeleton has 3 active lower tweakey bytes in positions [2,5,15]

lower_tweakey_positions = [2,5,15]
lower_tweakey_len = len(lower_tweakey_positions)


lower_relations = []

# Tweakey inactive in rounds 8 and 9.

for i in range(lower_tweakey_len):
    lower_relations.append([Key(8,i,lower_tweakey_len,model)])
    lower_relations.append([Key(9,i,lower_tweakey_len,model)])

# tweakey material from round 7 should retract in round 6: y_6[13] = 0.

lower_active_bytes_R7 = [8,10,11]
lower_base_tweakey_indexes_for_R7 = [lower_tweakey_positions.index(KeyPermutationIteration(lower_active_bytes_R7[0],7)),
                                     lower_tweakey_positions.index(KeyPermutationIteration(lower_active_bytes_R7[1],7)),
                                     lower_tweakey_positions.index(KeyPermutationIteration(lower_active_bytes_R7[2],7))]

K7 = block_matrix(4,[[Key(7,lower_base_tweakey_indexes_for_R7[0],lower_tweakey_len,model)],
                     [ZeroFunction(lower_tweakey_len,model)],
                     [Key(7,lower_base_tweakey_indexes_for_R7[1],lower_tweakey_len,model)],
                     [Key(7,lower_base_tweakey_indexes_for_R7[2],lower_tweakey_len,model)]])

lower_relations.append([MC_inv_blockmatrix[1]*K7])

lower_ker = block_matrix(7,lower_relations).right_kernel()


#Due to memory leaks, we need to store the Ker elements
upper_ker_array = []
lower_ker_array = []


# Store the possible tweakey differences for the upper trail in upper_key_array
for el in upper_ker:
    if el.is_zero():
        continue

    # The variable el is a binary vector.
    # To get the tweakey difference value from this binary vector, use the function getKeyElement.
    # To get the key element in byte positions k in round r, first compute:
    # i = upper_tweakey_positions.index(Key_Permutation[upper_active_bytes_R1[k]])
    # Then call getKeyElement(el,i,r,model).

    tk5_13 = getKeyElement(el,2,5,model).integer_representation()
    tk5_14 = getKeyElement(el,3,5,model).integer_representation()
    tk5_15 = getKeyElement(el,0,5,model).integer_representation()
    upper_ker_array.append([el,tk5_13,tk5_14,tk5_15])


# Store the possible tweakey differences for the lower trail in lower_key_array, and for each possible difference,
# compute the number of possible transitions x_6 <-> y_6 on 3 bytes (2,7,8).
for el2 in lower_ker:
    if el2.is_zero():
        continue

    tk6_3 = getKeyElement(el2,1,6,model)
    #y6[2] is linearly dependant of tk6[3]. 14*x6[2] + 11*tk6[3] = 0 <=> tk6[3] *11/14 = x6[2]
    x6_2 = tk6_3*F.fetch_int(11)/F.fetch_int(14)
    Column3 = Matrix(4,[[F.fetch_int(0)],[F.fetch_int(0)],[x6_2],[tk6_3]])
    y5_15 = (MC_inv*Column3)[3,0]

    tk6_4 = getKeyElement(el2,0,6,model)
    #x6[7] is linearly dependant of tk6[4]. 13*x6[7] + 9*tk6[4] = 0 <=> tk6[4] *9/13 = x6[7]
    x6_7 = tk6_4*F.fetch_int(9)/F.fetch_int(13)
    Column2 = Matrix(4,[[tk6_4],[F.fetch_int(0)],[F.fetch_int(0)],[x6_7]])
    y5_14 = (MC_inv*Column2)[2,0]

    tk6_9 = getKeyElement(el2,2,6,model)
    #x6[8] is linearly dependant of tk6[9]. 14*x6[8] + 11*tk6[9] = 0 <=> tk6[9] *11/14 = x6[8]
    x6_8 = tk6_9*F.fetch_int(11)/F.fetch_int(14)
    Column1 = Matrix(4,[[x6_8],[tk6_9],[F.fetch_int(0)],[F.fetch_int(0)]])
    y5_13 = (MC_inv*Column1)[1,0]

    y5_15 = y5_15.integer_representation()
    y5_14 = y5_14.integer_representation()
    y5_13 = y5_13.integer_representation()

    tk7_8 = getKeyElement(el2,2,7,model)
    tk7_10 = getKeyElement(el2,1,7,model)
    tk7_11 = getKeyElement(el2,0,7,model)
    Col = Matrix(4,[[tk7_8],[F.fetch_int(0)],[tk7_10],[tk7_11]])
    y6_antidiag = MC_inv*Col
    y6_8 = y6_antidiag[0,0]
    y6_2 = y6_antidiag[2,0]
    y6_7 = y6_antidiag[3,0]

    # Compute the number of possible transitions from x_6 <-> y_6 on 3 bytes, using the DDT table.
    ddt = DDT[x6_8.integer_representation(),y6_8.integer_representation()]*DDT[x6_7.integer_representation(),y6_7.integer_representation()]*DDT[x6_2.integer_representation(),y6_2.integer_representation()]

        
    if(ddt!=0):
        lower_ker_array.append([el2,y5_13,y5_14,y5_15,ddt])

# We expect len(upper_ker_array) = 2^8 elements (12 degrees of freedom, 11 linear conditions)
# We expect len(lower_ker_array) = 2^16/2^3 elements (9 degrees of freedom, 7 linear conditions + 3 DDT values !=0)


# We now need to compute the middle round transition, using the BCT table.

# Counter for debugging
vmax = 0
counter=0
for upper_list in upper_ker_array:
    for lower_list in lower_ker_array:
        
        upper_el,utk5_13,utk5_14,utk5_15= upper_list[0],upper_list[1],upper_list[2],upper_list[3]
        lower_el,ly5_13,ly5_14,ly5_15,ddt= lower_list[0],lower_list[1],lower_list[2],lower_list[3],lower_list[4]
        # v is directly linked to the probability of the trail. Maximize v = Maximize the trail probability.
        v = BCT[utk5_13,ly5_13]*BCT[utk5_14,ly5_14]*BCT[utk5_15,ly5_15]*ddt*ddt
        if(v > vmax):
                vmax = v
                final_upper_el=upper_el
                final_lower_el=lower_el
                print(str(vmax))
                print(final_upper_el)
                print(final_lower_el)

                #prettyPrint(el,4,model,"Delta")
                #prettyPrint(el2,2,model,"Nabla")    
        if(counter%100000 == 0):
            print(counter) 
        counter+=1


# final_upper_el = (1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0)
# final_lower_el = (1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1)
#vmax = 24576

#use the function printInFile function to print the tweakey differences in a file that can be given as input to the script make_figure.py.
    
f = open("output/Deoxys_TK3_boomerang_fromCiphertext_5R_7R_tweakeyvalues.txt","w")
printInFile(f,final_upper_el,final_lower_el,model,upper_tweakey_positions,lower_tweakey_positions,upper_rounds,lower_rounds)

# -----------------------------------------------------------------------------------------------


