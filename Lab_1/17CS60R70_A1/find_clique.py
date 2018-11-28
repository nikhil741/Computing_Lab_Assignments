
"""This Programme Makes the Graph and Highlight the Clique of Given Size"""
import sys
import itertools
from subprocess import call
import graphviz as gv
ALL_TEXT = []
CLIQUE_DETAILS = []
NODE_DETAIL = []
ADJACENCY_LIST = []
POSSIBLE_COMBINATIONS = []
GRAPH_DETAIL = gv.Graph(format='dot')
def make_clique_graph():
    """Calculate Graph Using Graphviz And Store Result"""
    for single_node in NODE_DETAIL:
        if len(CLIQUE_DETAILS) and single_node in CLIQUE_DETAILS[0]:
            GRAPH_DETAIL.node(single_node, color='green', style='filled', shape='circle')
        else:
            GRAPH_DETAIL.node(single_node)
    for single_node in ADJACENCY_LIST:
        for connecting_edge in single_node:
            if single_node[0] < connecting_edge:
                if len(CLIQUE_DETAILS) and [single_node[0], connecting_edge] in CLIQUE_DETAILS[0]:
                    GRAPH_DETAIL.edge(single_node[0], connecting_edge, color='red')
                else:
                    GRAPH_DETAIL.edge(single_node[0], connecting_edge)
    file_output = open('outputGraph.dot', 'w')
    file_output.write(str(GRAPH_DETAIL))
    file_output.close()
    call(["dot", "-Tpng", "outputGraph.dot", "-o", "graph_image.png"])

#input to python programme
FILE1 = open(sys.argv[1], "r")
#Storing Element in array
for single_line in FILE1:
    ALL_TEXT.append(single_line.strip('\n'))
try:
    k = int(ALL_TEXT[0])
    ALL_TEXT.pop(0)
except:
    FILE_OUT = open('outputGraph.dot', 'w')
    FILE_OUT.write(str(GRAPH_DETAIL))
    FILE_OUT.close()
    call(["dot", "-Tpng", "outputGraph.dot", "-o", "graph_image.png"])
    sys.exit()
for single_line in ALL_TEXT:
    if len(single_line) > 0 and single_line[0] != ' ':
        ADJACENCY_LIST.append(single_line.split())
        NODE_DETAIL.append(single_line.split()[0])
POSSIBLE_COMBINATIONS = list(itertools.combinations(NODE_DETAIL, k))
for part_comb in POSSIBLE_COMBINATIONS:
    count = 0
    for node_of_comb in part_comb:
        node_neighbour_list = []
        for node_neighbour_list in ADJACENCY_LIST:
            if node_neighbour_list[0] == node_of_comb:
                for matchPointer in part_comb:
                    if matchPointer in node_neighbour_list:
                        count = count + 1
    if count == k*k:
        CLIQUE_DETAILS.append(part_comb)
for clique in CLIQUE_DETAILS:
    print ' '.join(clique)
FILE1.close()
make_clique_graph()
