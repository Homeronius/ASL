import sys
import numpy
import matplotlib.pyplot as plt
import seaborn as sns

def read_dataset(path):
    data = numpy.loadtxt(open(path, "rb"), delimiter=",", skiprows=1)
    return data[:,:-1], data[:,-1]


def plot_dataset2D(X, y):
    """Plot first two features of dataset.

    Parameters
    ----------
    X :
        datapoints (n x d)
    y :
        labels (n)
    """
    sns.set_theme()
    sns.color_palette('colorblind')

    fig = plt.figure()

    ax = fig.add_subplot(111)

    ax.scatter(X[:, 0], X[:, 1], marker="o", c=y, s=20, edgecolor="k")
    # for i in range(X.shape[0]):
    #     ax.annotate(i, (X[i, 0]+.1, X[i, 1]+.1))

    # plt.axis('equal')
    plt.axis('square')

    plt.show()



def main():
    if len(sys.argv) != 2:
        print('Usage: plot_clusters.py <dataset_path>')
        return

    X, y = read_dataset(sys.argv[1])
    plot_dataset2D(X, y)



if __name__ == "__main__":
    main()
