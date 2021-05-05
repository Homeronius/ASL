import sys
import numpy
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

import networkx as nx
from networkx.drawing.nx_agraph import write_dot, graphviz_layout


def read_dataset(path):
    return pd.read_csv(path)


def create_graph(df):
    DG = nx.DiGraph()

    sizes = []
    colors = []
    selected_nodes = []
    unselected_nodes = []
    selected_sizes = []
    unselected_sizes = []
    labels = {}

    for index, row in df.iterrows():
        if row["child1_id"] != "0":
            DG.add_edge(row["id"], row["child1_id"])
            DG.add_edge(row["id"], row["child2_id"])
        sizes.append(row["weight"])
        labels[row["id"]] = "{}:{}".format(index+1, row["size"])
        # colors.append('#1f78b4' if row["selected"]==0 else '#f54823')
        if row['selected'] == 1:
            selected_nodes.append(row["id"])
            selected_sizes.append(row["weight"])
        else:
            unselected_nodes.append(row["id"])
            unselected_sizes.append(row["weight"])


    # print(DG.edges)
    # print(nx.is_tree(DG))

    if not nx.is_tree(DG):
        print("Generated tree is not actually a tree. Something went wrong!")
        # return

    # Rescale sizes
    selected_sizes = [1 * s for s in selected_sizes]
    unselected_sizes = [1 * s for s in unselected_sizes]

    # same layout using matplotlib with no labels
    plt.subplot(121)
    plt.title("Condensed_tree")
    pos = graphviz_layout(DG, prog="dot")
    nx.draw(DG, pos, with_labels=False, arrows=True, node_size=sizes,
            node_color=colors)

    nodes = nx.draw_networkx_nodes(DG, pos, nodelist=unselected_nodes, edgecolors='#000000', node_size=unselected_sizes, node_color=unselected_sizes, cmap=plt.cm.viridis)
    nodes = nx.draw_networkx_nodes(DG, pos, nodelist=selected_nodes, edgecolors='#ff0000', linewidths=2, node_size=selected_sizes, node_color=selected_sizes, cmap=plt.cm.viridis)
    edges = nx.draw_networkx_edges(
        DG,
        pos,
        node_size=sizes,
        arrowstyle="->",
        arrowsize=10,
        edge_cmap=plt.cm.Blues,
        width=2,
    )

    pos_left = {}
    x_off = 100  # offset on the y axis
    for k, v in pos.items():
        pos_left[k] = (v[0]+x_off, v[1])
    
    nx.draw_networkx_labels(DG, pos_left, labels, font_size=10)

    plt.show()

def main():
    if len(sys.argv) != 2:
        print('Usage: plot_clusters.py <tree_path>')
        return

    df = read_dataset(sys.argv[1])

    create_graph(df)


if __name__ == "__main__":
    main()
