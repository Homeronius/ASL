import matplotlib.pyplot as plt
import numpy as np

from sklearn.datasets import make_blobs


def blobs_dataset(n_samples, n_features, n_clusters, cluster_std):
    """Small helper to create dataset with respective parameters and return dict.

    Parameters
    ----------
    n_samples :
        n_samples (n)
    n_features :
        n_features (d)
    n_clusters :
        n_clusters
    cluster_std :
        standard deviation of each cluster
    """

    X, y = make_blobs(n_samples, n_features, n_clusters, cluster_std)
    return {"data": X, "labels": y, "std": cluster_std}


def save_datasets(basename, dataset_list):
    """Save list of datasets given as dict to location specified in basename.

    Parameters
    ----------
    basename :
        relative path and basename of datasets (i.e. '../data/blobs')
    dataset_list :
        dataset_list
    """

    for i, dataset in enumerate(dataset_list):
        X, y, std = dataset.values()
        n_samples, n_features = X.shape
        header = "{},{},{}".format(n_samples, n_features, std)

        fname = "{}_{}.csv".format(basename, i)
        np.savetxt(
            fname,
            np.concatenate((X, y[:, None]), axis=1, dtype=np.object_),
            delimiter=",",
            comments="",
            header=header,
        )


def plot_dataset2D(X, y):
    """Plot first two features of dataset.

    Parameters
    ----------
    X :
        datapoints (n x d)
    y :
        labels (n)
    """

    fig = plt.figure()
    ax = fig.add_subplot(111)

    ax.scatter(X[:, 0], X[:, 1], marker="o", c=y, s=20, edgecolor="k")

    plt.show()


def main():
    # Number of datasets to be generated
    n_datasets = 5
    datasets = []

    # Number of samples (n)
    n_samples = 1000
    # Dimension of each data point (d)
    n_features = 4
    # Number of clusters
    n_centers = 3

    # Define standard deviations of the clusters
    std_min = 1.0
    std_max = 2.0
    std_range = np.linspace(std_min, std_max, n_datasets)

    # Generate multiple datasets with increasing std
    for std in std_range:
        datasets.append(blobs_dataset(n_samples, n_features, n_centers, std))

    # Set basename of datasets
    basename = "../data/blobs"
    save_datasets(basename, datasets)

    print("Generated {} datasets with std {}.".format(n_datasets, std_range))


if __name__ == "__main__":
    main()
