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
    for index, row in df.iterrows():
        if row["child1_id"] != "0":
            DG.add_edge(row["id"], row["child1_id"])
            DG.add_edge(row["id"], row["child2_id"])
        sizes.append(row["weight"])

    # print(DG.edges)
    # print(nx.is_tree(DG))

    if not nx.is_tree(DG):
        print("Generated tree is not actually a tree. Something went wrong!")
        return

    # Rescale sizes
    sizes = [10 * s for s in sizes]

    # same layout using matplotlib with no labels
    plt.subplot(121)
    plt.title("condensed_tree")
    pos = graphviz_layout(DG, prog="dot")
    nx.draw(DG, pos, with_labels=False, arrows=True, node_size=sizes)
    plt.show()


def main():
    if len(sys.argv) != 2:
        print('Usage: plot_clusters.py <tree_path>')
        return

    df = read_dataset(sys.argv[1])

    create_graph(df)


if __name__ == "__main__":
    main()
