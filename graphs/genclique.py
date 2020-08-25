import random
import argparse
import time
import argparse

def gen(x):
    edge = []
    k = 0
    for i in range(0, x):
        for j in range(i+1, x):
            edge.append((i, j))

    weights = []
    for i in range(0, x):
        weights.append(random.randint(1,1))
    return len(weights), edge, weights



# Argument parsing
parser = argparse.ArgumentParser()
parser.add_argument("size", type=int)
args = parser.parse_args()
n, e, w = gen(args.size)
print(n)
for i in w:
    print(i)
for a,b in e:
    print(a,b)
