import random
import argparse
import time
import gen

import networkx as nx # https://networkx.github.io/documentation/latest/reference/classes/digraph.html
from network2tikz import plot # https://pypi.org/project/network2tikz/
from networkx.drawing.nx_agraph import graphviz_layout

def dag():
    G = nx.DiGraph()
    for i in range(1, 7):
        G.add_node(i, weight=i)
    #G.add_edge(1, 2)
    G.add_edges_from([(1,2), (1,5), (1,6), (2,3), (2,4)])
    gen.gen_beautiful_graph(G, "fig1_dag", "dot", csv=True, color="lightgray")


def normal():
    G = nx.Graph()
    for i in range(1, 7):
        G.add_node(i, weight=i)

    G.add_edges_from([(1,2), (1,5), (1,6), (2,3), (2,4), (1,3), (1,4)])
    gen.gen_beautiful_graph(G, "fig1_cstr", "dot", csv=True, color="lightgray")

normal()
dag()
