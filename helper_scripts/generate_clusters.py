import sys
from os import path
from os import makedirs
from typing import no_type_check
import matplotlib.pyplot as plt
import numpy as np

from sklearn.datasets import make_blobs, make_moons


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

    X, y = make_blobs(n_samples, n_features, n_clusters,
                      cluster_std, random_state=seed)
    return {"data": X, "labels": y+1, "std": cluster_std}


def moons_with_blobs(n_samples, cluster_std=0.25, seed=None):
    """Small helper to create a 2D dataset similar to the 
       "How HDBSCAN works" page https://hdbscan.readthedocs.io/en/latest/how_hdbscan_works.html.

    Parameters
    ----------
    n_samples :
        n_samples (n)
    cluster_std :
        standard deviation of each cluster
    seed:
        seed for reproducible results
    """
    moons, moons_y = make_moons(
        n_samples=n_samples, noise=0.05, random_state=seed)
    blobs, blobs_y = make_blobs(n_samples=n_samples, centers=[(-0.75, 2.25), (1.0, 2.0)],  random_state=seed,
                                cluster_std=cluster_std)
    return {"data": np.concatenate((moons, blobs)), "labels": np.concatenate((moons_y + 1, blobs_y + 3)), "std": cluster_std}


def random_data(n_samples, n_features, seed=None):
    """Fully random data.

    Parameters
    ----------
    n_samples :
        n_samples (n)
    n_features :
        n_features (d)
    seed:
        seed for reproducible results
    """
    np.random.seed(seed)
    X = np.random.random(size=(n_samples, n_features))
    y = np.ones(n_samples)
    return {"data": X, "labels": y, "std": None}


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

        fname = "{}_d{}_{}.csv".format(basename, n_features, i)
        np.savetxt(
            path.join(datadir, fname),
            np.concatenate((X, y[:, None]), axis=1, dtype=np.object_),
            delimiter=",",
            comments="",
            header=header,
        )


def main():
    if len(sys.argv) < 4 or len(sys.argv) > 5:
        print("Usage: generate_clusters.py <relative_data_folder_path> " + \
            "<n_clusters> <d_dimensions> [random|blobs|moonblobs]")
        return

    # Set seed for reproducible outcomes
    seed = 42
    dataset_name = "blobs" if len(sys.argv) != 5 else sys.argv[4]
    # Number of datasets per std / n to be generated
    n_datasets = 1
    datasets = []

    # Number of samples (n)
    pow_min = 5
    pow_mid = 12
    pow_max = 15
    power_range = [pow_min, pow_mid]
    n_samples = [2**i for i in range(*power_range)]
    n_samples += [2 ** pow_mid + i * (2 ** (pow_mid - 1))
                  for i in range(pow_max - pow_mid + 3)]

    # Dimension of each data point (d)
    n_features = int(sys.argv[3])
    # Number of clusters
    n_centers = int(sys.argv[2])
    if dataset_name == "moonblobs" and n_features != 2:
        raise Exception(
            "moonblobs only supports d=2. d={} was given".format(n_features))
    # Define standard deviations of the clusters
    std_range = [1.0, 1.0]
    std_range = np.linspace(*std_range, n_datasets)

    # Generate multiple datasets with increasing std
    for n in n_samples:
        for std in std_range:
            if dataset_name == "blobs":
                datasets.append(blobs_dataset(n, n_features, n_centers, std, seed))
            elif dataset_name == "moonblobs":
                datasets.append(moons_with_blobs(n, seed=seed))
            elif dataset_name == "random":
                datasets.append(random_data(n, n_features, seed))
            else:
                raise TypeError("unknown dataset")

    # Set relative location and basename of datasets
    datadir = path.relpath(sys.argv[1])
    if not path.exists(datadir):
        makedirs(datadir)
    basename = dataset_name + "_perf_data" if dataset_name != "blobs" else "perf_data"

    print("Writing dataset {} with basename \"{}\" to {}".format(dataset_name, basename, str(datadir)))
    
    save_datasets(datadir, basename, datasets)

    print("Generated {} datasets with sizes {}.".format(
        n_datasets*len(n_samples)*len(std_range), n_samples))


if __name__ == "__main__":
    main()
