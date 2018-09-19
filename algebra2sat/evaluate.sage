
solver = SAT()
solver.read("dimacs.txt")
solution = solver()

#print(solution)

lines = []
with open('watches.txt') as f:
    lines = f.readlines()
#print(lines)

for line in lines:
    toks = line.split(' ')
    bits = []
    for i in range(int(toks[0])):
        bit = int(toks[int(toks[0]) - i])
        bits.append('1' if solution[bit] else '0')
    
    print(''.join(toks[int(toks[0]) + 1:]).rstrip() + ' : ' + ''.join(bits))