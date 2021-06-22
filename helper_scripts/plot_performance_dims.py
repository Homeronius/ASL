from os import path
import argparse
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import pandas as pd

AMD = r" $\bf{AMD\ Ryzen\ 7\ 4800H\ (Zen\ 2),\ 2.9GHz}$"
INTEL = r" $\bf{Intel\ i5-7300U\ (Kaby\ Lake),\ 2.6GHz}$"


def read_dataset(path):
    data = np.loadtxt(open(path, "rb"), delimiter=",", skiprows=1)
    return data[:, 0], data[:, 6], data[:, 2], data[:, 1]


def main(args):
    datadir = path.relpath(args.data_path)
    if not path.exists(datadir):
        raise Exception("directory {} does not exist".format(datadir))

    sns.set_theme()

    fig = plt.figure()
    ax = fig.add_subplot()
    system_name = INTEL if args.system == "intel" else AMD
    title = r"$\bf{Performance\ on}$"

    title += system_name
    title += "\n [flops/cycle]"
    ax.set_title(title, loc="left")

    X = np.array([])
    Y = np.array([])
    S = np.array([])
    for file in args.files[0]:
        fpath = path.join(datadir, file)
        N, flops, cycles, d = read_dataset(fpath)
        p = np.divide(flops, cycles)
        X = np.concatenate((X, N))
        Y = np.concatenate((Y, d))
        S = np.concatenate((S, p))

    df = pd.DataFrame.from_dict(np.array([X, Y, S]).T)
    df.columns = ['n', 'd', 'perf']
    pivotted = df.pivot('d', 'n', 'perf').iloc[::-1]

    yticks = [int(d) for d in pivotted.index]

    xticks = [int(d) for d in pivotted.columns]

    sns.heatmap(pivotted, cmap='RdBu_r', linewidths=.7,
                vmin=0, yticklabels=yticks, xticklabels=xticks)
    plt.yticks(rotation=0)
    plt.ylabel('d',rotation=0)
    
    if args.save_path is None:
        plt.show()
    else:
        fig.savefig(args.save_path, bbox_inches="tight")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--system", type=str,
                        default="intel", choices=["intel", "amd"])
    parser.add_argument(
        "--data-path",
        type=str,
        default="data/timings/",
        help="relative path to where timings are stored",
    )
    parser.add_argument(
        "--files",
        type=str,
        nargs="+",
        required=True,
        action="append",
        help="name of one or more .csv performance measurements",
    )
    parser.add_argument(
        "--save-path",
        type=str,
        default=None,
        help="path or filename to store plot to",
    )

    args = parser.parse_args()
    main(args)
