solver = SAT()
n = 5
r = 3
d = r - 2

def get_inversions_to_sorted(l):
    ret = 1
    finished = False
    l_temp = l[:]

    # bubblesort, keeping track of inversions
    while not finished:
        finished = True

        for i in range(len(l_temp) - 1):
            if l_temp[i] > l_temp[i + 1]:
                l_temp[i], l_temp[i + 1] = l_temp[i + 1], l_temp[i]
                ret *= -1
                finished = False
    
    return ret

# assumes sorted *lists* l1 and l2
def get_inversions_sign(l1, l2):
    l_temp = l1[:]
    l_temp.extend(l2)
    return get_inversions_to_sorted(l_temp)

# takes in lists (not necessarily sorted) and outputs a unique index
# For large values of n, probably bad.
# Very simple and inefficient. Don't care.
subset_index_cache = Subsets(n, r).list()
def get_index(l1, l2 = None):
    l1_temp = l1[:] # to get around python's annoying in-place extending
    if l2 != None:
        l1_temp.extend(l2)
    l_total = sorted(l1_temp)
    return subset_index_cache.index(set(l_total)) + 1 # SAT solver does not enjoy 0-indexing

def add_grassman_clauses(solver, a, b, c, d, e, f):
    S = [-1, 1]
    all_relations = Tuples(S, 6)
    for el in all_relations:
        if not {-1, 1}.issubset({ el[0]*el[1], -el[2]*el[3], el[4]*el[5] }):
            solver.add_clause((-el[0] * a, -el[1] * b, -el[2] * c, -el[3] * d, -el[4] * e, -el[5] * f))

# This excludes the circuit given from appearing in the solution.
# Note that excluding a circuit will also exclude its negative
def exclude_circuit(solver, circuit):
    if len(circuit) != n:
        print("ERROR: not right dimensions")
    
    non_zero_indices = [i for i,e in enumerate(circuit) if circuit[i] != 0]
    if len(non_zero_indices) != r + 1:
        print("ERROR: circuit support is the wrong size")
    
    # temporary, for ease of implementation
    if r != 3:
        print("ERROR: wrong dimension")

    # the 4 chirotopes that this circuit affects
    triples = []
    triples.append([non_zero_indices[3] + 1, non_zero_indices[1] + 1, non_zero_indices[2] + 1])
    triples.append([non_zero_indices[0] + 1, non_zero_indices[3] + 1, non_zero_indices[2] + 1])
    triples.append([non_zero_indices[0] + 1, non_zero_indices[1] + 1, non_zero_indices[3] + 1])
    triples.append([non_zero_indices[0] + 1, non_zero_indices[1] + 1, non_zero_indices[2] + 1])

    # chirotope indices cooresponding to the triples
    chi = [get_index(triple) * get_inversions_to_sorted(triple) for triple in triples]

    # construct the solution in terms of chirotopes
    # with chi[3] = +1
    solution = []
    for i in range(3):
        solution.append(1 if circuit[non_zero_indices[i]] != circuit[non_zero_indices[3]] else -1)
    solution.append(1)
    
    # now remove its two negations
    solver.add_clause(( solution[0] * chi[0], solution[1] * chi[1], solution[2] * chi[2], solution[3] * chi[3] ))
    solver.add_clause(( -solution[0] * chi[0], -solution[1] * chi[1], -solution[2] * chi[2], -solution[3] * chi[3] ))

def exclude_all_circuit_permutations(solver, num_positive, num_negative):
    all_circuits = Tuples([0, -1, 1], n)

    for circuit in all_circuits:
        positive = sum(1 if x == 1 else 0 for x in circuit)
        negative = sum(1 if x == -1 else 0 for x in circuit)
        if num_positive == positive and num_negative == negative:
            exclude_circuit(solver, circuit)

E = set(range(1, n+1))
tau = Subsets(n, d)
for sigma_set in tau:

    sigma = sorted(list(sigma_set))
    
    E_minus_sigma = Subsets(E.difference(sigma_set), 4)
    for xs_set in E_minus_sigma:
        xs = sorted(list(xs_set))

        l12 = [xs[0], xs[1]]
        l34 = [xs[2], xs[3]]
        l13 = [xs[0], xs[2]]
        l24 = [xs[1], xs[3]]
        l14 = [xs[0], xs[3]]
        l23 = [xs[1], xs[2]]

        chi_12 = get_inversions_sign(sigma, l12) * get_index(sigma, l12)
        chi_34 = get_inversions_sign(sigma, l34) * get_index(sigma, l34)
        chi_13 = get_inversions_sign(sigma, l13) * get_index(sigma, l13)
        chi_24 = get_inversions_sign(sigma, l24) * get_index(sigma, l24)
        chi_14 = get_inversions_sign(sigma, l14) * get_index(sigma, l14)
        chi_23 = get_inversions_sign(sigma, l23) * get_index(sigma, l23)

        add_grassman_clauses(solver, chi_12, chi_34, chi_13, chi_24, chi_14, chi_23)

# acyclic
#exclude_all_circuit_permutations(solver, 4, 0)

# no quadrilaterals 
exclude_all_circuit_permutations(solver, 2, 2)

print(subset_index_cache)
solutions = solver()
print(solutions)

# drawing routine. Brute force
# only works for r = 3 (2-dimensions) because sage only plots in 2d (i think)
from random import random
for i in range(1000):

    # generate n random 2-dimensional points
    v = [(random(), random()) for j in range(n)]

    # check if this is a realization
    found = True
    for i, subset_set in enumerate(subset_index_cache):
        subset = sorted(list(subset_set))

        v0 = v[subset[0] - 1]
        v1 = v[subset[1] - 1]
        v2 = v[subset[2] - 1]

        output = (v1[0] - v0[0]) * (v2[1] - v0[1]) - (v1[1] - v0[1]) * (v2[0] - v0[0])
        if solutions[i + 1] == True and output <= 0.0:
            found = False
            break
        if solutions[i + 1] == False and output >= 0.0:
            found = False
            break
    
    if not found:
        continue

    G = points(v)
    for i,p in enumerate(v):
        G += text('  %d'%(i + 1), p, horizontal_alignment='left', color='red')
    G.show()
    break