solver = SAT()
n = 5
r = 3
d = r - 2

# assumes sorted *lists* l1 and l2
def get_inversions_sign(l1, l2):
    ret = 1

    for el in l2:
        num = len({x for x in l1 if el < x})
        ret *= -1 if (num % 2 == 1) else 1
    
    return ret

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
def add_acyclic_clause(solver):
    solver.add_clause(tuple(range(1, len(subset_index_cache) + 1)))
    solver.add_clause(tuple(range(-1, -len(subset_index_cache) - 1, -1)))
def add_nonrealizable_2d_clause(solver):
    subset_pairs = Subsets(set(subset_index_cache), 2)
    for pair_set in subset_pairs:
        pair = list(pair_set)
        pair_list = [list(pair[0]), list(pair[1])]
        if (pair_list[0][0] == pair_list[1][0]) and (pair_list[0][1] == pair_list[1][1]):
            max_el = max(pair_list[0][2], pair_list[1][2])
            min_el = min(pair_list[0][2], pair_list[1][2])
            ind1 = get_index(pair_list[0])
            ind2 = get_index(pair_list[1])
            ind3 = get_index([pair_list[0][0], min_el, max_el])
            solver.add_clause((ind1, ind2, ind3))
            solver.add_clause((-ind1, -ind2, -ind3))

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

add_acyclic_clause(solver)
add_nonrealizable_2d_clause(solver)
#solver.add_clause((-1,))
#solver.add_clause((1,))
#solver.add_clause((3,))

print(subset_index_cache)
print(solver())
print("hi")