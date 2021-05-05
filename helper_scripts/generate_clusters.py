import sys
from os import path
from os import makedirs
import matplotlib.pyplot as plt
import numpy as np

from sklearn.datasets import make_blobs


def blobs_dataset(n_samples, n_features, n_clusters, cluster_std, seed=None):
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
    seed:
        seed for reproducible results
    """

    X, y = make_blobs(n_samples, n_features, n_clusters, cluster_std, random_state=seed)
    return {"data": X, "labels": y, "std": cluster_std}


def save_datasets(datadir, basename, dataset_list):
    """Save list of datasets given as dict to location specified in basename.

    Parameters
    ----------
    datadir:
        relative path to directory to store datasets to (i.e. '../data')
    basename :
        basename of datasets
    dataset_list :
        dataset_list
    """

    for i, dataset in enumerate(dataset_list):
        X, y, std = dataset.values()
        n_samples, n_features = X.shape
        header = "{},{},{}".format(n_samples, n_features, std)

        fname = "{}_{}.csv".format(basename, i)
        np.savetxt(
            path.join(datadir, fname),
            np.concatenate((X, y[:, None]), axis=1, dtype=np.object_),
            delimiter=",",
            comments="",
            header=header,
        )


def main():
    if len(sys.argv) != 2:
        print("Usage: generate_clusters.py <relative_data_folder_path>")
        return

    # Set seed for reproducible outcomes
    seed = 42

    # Number of datasets to be generated
    n_datasets = 5
    datasets = []

    # Number of samples (n)
    n_samples = 25
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
        datasets.append(blobs_dataset(n_samples, n_features, n_centers, std, seed))

    # Set relative location and basename of datasets
    datadir = path.relpath(sys.argv[1])
    if not path.exists(datadir):
        makedirs(datadir)

    print("Writing datasets to", str(datadir))

    basename = "blobs"

    save_datasets(datadir, basename, datasets)

    print("Generated {} datasets with std {}.".format(n_datasets, std_range))


if __name__ == "__main__":
    main()
