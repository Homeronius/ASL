import sys
import numpy
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.metrics import adjusted_rand_score
from matplotlib.colors import ListedColormap

def read_dataset(path):
    data = numpy.loadtxt(open(path, "rb"), delimiter=",", skiprows=1)
    return data[:,:-1], data[:,-1]


def plot_dataset2D(X, y, y_true, score=None, labels=False):
    """Plot first two features of dataset.

    Parameters
    ----------
    X :
        datapoints (n x d)
    y :
        labels (n)
    """
    cb_cmap = ListedColormap(sns.color_palette('colorblind'))

    # construct cmap
    flatui = ["#9b59b6", "#3498db", "#95a5a6", "#e74c3c", "#34495e", "#2ecc71"]
    my_cmap = ListedColormap(sns.color_palette(flatui).as_hex())
    new_cmap = sns.light_palette("Navy", as_cmap=True)

    fig = plt.figure()

    ax = fig.add_subplot(111)

    if score is not None:
        ax.set_title(f'Adjusted Rand Index : {score:.4f}')

    ax.scatter(X[:, 0], X[:, 1], marker="o", c=y, s=20, edgecolor="k", cmap=my_cmap)

    if labels:
        for i in range(X.shape[0]):
            ax.annotate(f"{int(y_true[i])}:{int(y[i])}", (X[i, 0]+.1, X[i, 1]+.1))
        ax.legend(y)

    # plt.axis('equal')
    plt.axis('square')

    plt.show()
    plt.savefig(sys.argv[1][:-4]+'_prediction.png')



def main():
    if len(sys.argv) < 2:
        print('Usage: plot_clusters.py <prediction_dataset_path>')
        return

    elif len(sys.argv) == 2:
        print('To plot with similarity score use: plot_clusters.py <prediction_dataset_path> <true_dataset_path>')

    X_pred, y_pred = read_dataset(sys.argv[1])
    score = None
    y_true = None
    if(len(sys.argv) > 2):
        X_true, y_true = read_dataset(sys.argv[2])
        # Compute similarity score
        score = adjusted_rand_score(y_pred, y_true)

    plot_dataset2D(X_pred, y_pred, y_true, score)



if __name__ == "__main__":
    main()
