import random
import argparse
import time

import networkx as nx # https://networkx.github.io/documentation/latest/reference/classes/digraph.html
from network2tikz import plot # https://pypi.org/project/network2tikz/
from networkx.drawing.nx_agraph import graphviz_layout

def gen_beautiful_graph(G, name="default_graph", prog=None, weighted_arrows=False, csv=False, color=True, canvax = 5, canvay = 5):
    weights = {a:b for a,b in G.nodes.data("weight")}
    def col_from_weights(wd):
        minw = -1
        maxw = -1
        for x in wd:
            if wd[x] < minw:
                minw = wd[x]
            if wd[x] > maxw:
                maxw = wd[x]

        dist = maxw - minw
        cola = (245, 203, 66)
        colb = (163, 0, 0)
        cols = {}
        vec = ((colb[0] - cola[0])/dist,(colb[1] - cola[1])/dist,(colb[2] - cola[2])/dist)
        for (i, x) in enumerate(wd):
            xdist = wd[x]
            cols[x] = (cola[0] + xdist*vec[0], cola[1] + xdist*vec[1], cola[2] + xdist*vec[2])
            if wd[x] < minw:
                minw = wd[x]
            if wd[x] > maxw:
                maxw = wd[x]
        return cols
        
    def wid_from_weights(wd):
        minw = -1
        maxw = -1
        for x in wd:
            if wd[x] < minw:
                minw = wd[x]
            if wd[x] > maxw:
                maxw = wd[x]
        dist = maxw - minw
        cola = 0.1
        colb = 4
        cols = {}
        vec = (colb - cola)/dist
        for (i, x) in enumerate(wd):
            xdist = wd[x]
            cols[x] = cola + xdist*vec
            if wd[x] < minw:
                minw = wd[x]
            if wd[x] > maxw:
                maxw = wd[x]
        
        wid = {}
        for u, v in G.edges:
            wid[(u,v)] = cols[u]
        return wid

    #G.nodes[1] to get dict
    #print(G.nodes.data("weight"))
    cols = col_from_weights(weights)
    widths = wid_from_weights(weights)
    #print(widths)
    
    # couldn't get this to work okay
    #pos = nx.spring_layout(G,pos=fixed_positions, fixed = fixed_nodes)

    # dot :hierarchical
    # neato : spring
    # fdp : other spring
    # twopi : radial
    # circo : circular?
    # root = something
    pos = graphviz_layout(G, prog=prog, root=1)
    viz = {
            "standalone": False, # true = can be imported directly
            "layout": pos, #replace with "fr" to get spring
            "vertex_label" : weights, # can be given as list or dict
            "edge_width" : widths,
            "edge_curved" : 0.11,
            "canvas" : (canvax,canvay),
            #"vertex_label_position" : "below" # inside b default
            "clean_tex": False,
            "vertex_label_size" : 11
            }
    if color == True:
        viz["vertex_color"] = cols
    else:
        viz["vertex_color"] = color
    if weighted_arrows == False:
        viz["edge_width"] = 1
    elif int(weighted_arrows) > 0:
        viz["edge_width"] = weighted_arrows

    print("Plotting..")
    plot(G, f"{name}.pdf", **viz)
    if csv:
        plot(G, f"{name}.csv", **viz)
        print("printed to", f"{name}.csv")
    print("printed to", f"{name}.pdf")
    print("printed to", f"{name}.tex")

# generate networkx graph
# random matrix DAG
def gen_DiGraph(n, max_size, ratio, allsame):
    G = nx.DiGraph()
    for i in range(0, n):
        w = max_size
        if not allsame:
            w = random.randint(1, max_size)
        G.add_node(i, weight=w)
    for i in range(0, n):
        for j in range(0, i):
            if random.random() > 1 - ratio:
                G.add_edge(i, j)
    return G
# m = amount of edges
def gen_WDiGraph(n, m, max_size, allsame):
    G = nx.DiGraph()
    for i in range(0, n):
        w = max_size
        if not allsame:
            w = random.randint(1, max_size)
        G.add_node(i, weight=w)
    if m == 0:
        return G
    calc = n*(n-1)//2
    l = list(range(0, calc))
    chosen = []
    for i in range(0, m):
        x = l[random.randint(0, calc-1)]
        while x in chosen:
            x = l[random.randint(0, calc-1)]
        chosen.append(x)
    chosen.sort()
    ind = 0
    x = 0
    b = False
    for i in range(0, n):
        if b:
            break
        for j in range(0, i):
            if chosen[x] == ind:
                G.add_edge(i, j)
                #print(chosen[x], " leads to ", i, j)
                x += 1
                if x == len(chosen):
                    b = True
                    break
            ind += 1

    return G

def print_Digraph(G):
    for (x,y,w) in nx.to_edgelist(G):
        print(x,y)

def writeto(filename, c):
    with open(filename, 'w') as f:
        f.write(c)

def graphto(filename, n, edges, weights):
    s_weights = ""
    for i in weights:
        s_weights += f"{i}\n"
    s_edges = ""
    for a,b in edges:
        s_edges += f"{a} {b}\n"
    writeto(filename, f"{n}\n{s_weights}{s_edges}")

def tograph_netz(filename):
    G = nx.DiGraph()
    s = None
    with open(filename, "r") as f:
        s = f.read()
    s = s.split("\n")
    n = int(s[0])
    for i in range(0, n):
        w = int(s[i+1])
        G.add_node(i, weight=w)

    for edg in s[1+n:-1]:
        d = edg.split(" ")
        a = int(d[0])
        b = int(d[1])
        G.add_edge(a, b)
    return G


def netz_graphto(filename, G):
    n = G.number_of_nodes()
    w = []
    for i in G.nodes():
        w.append(G.nodes[i]["weight"])

    e = list(G.edges())
    graphto(filename, n, e, w)

# generate neural like graph
def gen_netz(n, c, allsame, wess):
    G = nx.DiGraph()
    for i in range(0, c*n):
        weight = 0
        if allsame:
            weight = wess 
        else:
            weight = random.randint(1, wess)
        G.add_node(i, weight=weight)
    for x in range(0, n-1):
        for i in range(0, c):
            for j in range(0, c):
                G.add_edge(c*x + i, c*(x+1) + j)
    return G

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("prefix", type=str)
    parser.add_argument("depth", type=int)
    parser.add_argument("height", type=int)
    parser.add_argument("-p", help="print as pdf and latex", action="store_true")
    parser.add_argument("-a", help="weight = 1", action="store_true")
    args = parser.parse_args()

    G = gen_netz(args.depth,args.height, args.a, 10)
    name = f"{args.prefix}_{args.depth}_{args.height}"
    if args.p:
        gen_beautiful_graph(G, name, "dot", 1, canvax=8, canvay=8)

    netz_graphto(name + ".dag" , G)

