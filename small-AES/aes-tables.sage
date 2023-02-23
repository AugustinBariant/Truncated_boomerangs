n = 8

if len(sys.argv) == 2:
    n = int(sys.argv[1])

assert(n == 4 or n == 8)
print ("// Pre-computed tables for {}-bit AES".format(n))
    
if n == 8:
    F = GF(2^n, name='a', modulus=x^8 + x^4 + x^3 + x +1)
    L = Matrix(GF(2), n, n, [
        1,0,0,0,1,1,1,1,
        1,1,0,0,0,1,1,1,
        1,1,1,0,0,0,1,1,
        1,1,1,1,0,0,0,1,
        1,1,1,1,1,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,1,1,1,1,1,0,
        0,0,0,1,1,1,1,1,
    ])
    C = F.fetch_int(0x63)
    fmt = "0x{:02x}{:02x}{:02x}{:02x}"
elif n == 4:
    F = GF(2^n, name='a', modulus=x^4 + x +1)
    L = Matrix(GF(2), n, n, [
        1,1,1,0,
        0,1,1,1,
        1,0,1,1,
        1,1,0,1])
    C = F.fetch_int(0x6)
    fmt = "0x{:01x}{:01x}{:01x}{:01x}"

def sbox(x):
    # Inversion
    y = x^(2^n-2)
    # Linear layer
    y = [ y.polynomial()[i] for i in range(n) ]
    y = L*vector(y)
    y = (sum(y[i]*F.gen()^i for i in range(n)))
    y += C
    return y

S  = [sbox(F.fetch_int(i)).integer_representation() for i in range(2^n)]
SI = S[:]
for i in range(2^n):
    SI[S[i]] = i

MC = Matrix(F,4,4,[
    F.fetch_int(2), F.fetch_int(3), F.fetch_int(1), F.fetch_int(1),
    F.fetch_int(1), F.fetch_int(2), F.fetch_int(3), F.fetch_int(1),
    F.fetch_int(1), F.fetch_int(1), F.fetch_int(2), F.fetch_int(3),
    F.fetch_int(3), F.fetch_int(1), F.fetch_int(1), F.fetch_int(2)])

for b in range(4):
    print ("word Te{}[] = {{".format(b))
    for i in range(2^n):
        u = F.fetch_int(i)
        v = vector([F(0),F(0),F(0),F(0)])
        v[b] = sbox(u)
        v = MC*v
        v = (w.integer_representation() for w in v)
        print (("  "+fmt+",").format(*v))
    print ("};")

for b in range(4):
    print ("word Td{}[] = {{".format(b))
    for i in range(2^n):
        u = F.fetch_int(i)
        v = vector([F(0),F(0),F(0),F(0)])
        v[b] = F.fetch_int(SI[u.integer_representation()])
        v = (MC)^-1*v
        v = (w.integer_representation() for w in v)
        print (("  "+fmt+",").format(*v))
    print ("};")

print ("word Te4[] = {")
for i in range(2^n):
    u = sbox(F.fetch_int(i))
    v = vector([u,u,u,u])
    v = (w.integer_representation() for w in v)
    print (("  "+fmt+",").format(*v))
print ("};")

print ("word Td4[] = {")
for i in range(2^n):
    u = F.fetch_int(SI[i])
    v = vector([u,u,u,u])
    v = (w.integer_representation() for w in v)
    print (("  "+fmt+",").format(*v))
print ("};")

for b in range(4):
    print ("word MC{}[] = {{".format(b))
    for i in range(2^n):
        u = F.fetch_int(i)
        v = vector([F(0),F(0),F(0),F(0)])
        v[b] = u
        v = MC*v
        v = (w.integer_representation() for w in v)
        print (("  "+fmt+",").format(*v))
    print ("};")
