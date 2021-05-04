import sys
import numpy
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.metrics import adjusted_rand_score

def read_dataset(path):
    data = numpy.loadtxt(open(path, "rb"), delimiter=",", skiprows=1)
    return data[:,:-1], data[:,-1]


def plot_dataset2D(X, y, score=None):
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

    if score is not None:
        ax.set_title(f'Adjusted Rand Index : {score:.4f}')

    ax.scatter(X[:, 0], X[:, 1], marker="o", c=y, s=20, edgecolor="k")
    # for i in range(X.shape[0]):
    #     ax.annotate(i, (X[i, 0]+.1, X[i, 1]+.1))

    # plt.axis('equal')
    plt.axis('square')

    plt.show()



def main():
    if len(sys.argv) < 2:
        print('Usage: plot_clusters.py <prediction_dataset_path>')
        return

    elif len(sys.argv) == 2:
        print('To plot with similarity score use: plot_clusters.py <prediction_dataset_path> <true_dataset_path>')

    X_pred, y_pred = read_dataset(sys.argv[1])
    score = None
    if(len(sys.argv) > 2):
        X_true, y_true = read_dataset(sys.argv[2])
        # Compute similarity score
        score = adjusted_rand_score(y_pred, y_true)

    plot_dataset2D(X_pred, y_pred, score)



if __name__ == "__main__":
    main()
