from os import path
import argparse
from typing import Type
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns


def read_dataset(path, system="intel"):
    data = np.loadtxt(open(path, "rb"), delimiter=",", skiprows=1)
    if system == "intel":
        return data[:, 0], data[:, 6], data[:, 2], data[:, 3]
    else:
        return data[:, 0], data[:, 6], data[:, 2], data[:, 3]


def main(args):
    datadir = path.relpath(args.data_path)
    if not path.exists(datadir):
        raise Exception("directory {} does not exist".format(datadir))

    sns.set_theme()

    fig = plt.figure()
    ax = fig.add_subplot(111)

    title = r"$\bf{Preliminary\ performance\ plot}$"
    if args.metric == "fp/c":
        title += "\n [flops/cycle]"
    elif args.metric == "time":
        title += "\n [seconds]"
    elif args.metric == "cycles":
        title += "\n [" + r"$\times 10^9$" + " cycles]"
    ax.set_title(title, loc="left")

    freq = 2.6e9 if args.system == "intel" else 2.9e9
    for file in args.files[0]:
        fpath = path.join(datadir, file)
        N, flops, cycles, time = read_dataset(fpath, args.system)
        # flops *= 1e-9
        if args.metric == "fp/c":
            y = np.divide(flops, cycles)
        elif args.metric == "time":
            y = cycles / freq
        elif args.metric == "cycles":
            y = cycles / 1e9
        else:
            raise TypeError("unsupported metric to plot")

        if args.x_scale == "linear":
            (line,) = ax.plot(N, y, linestyle="-", marker="o")
        elif args.x_scale == "log":
            (line,) = ax.semilogx(N, y, linestyle="-", marker="o", base=2)
        else:
            raise TypeError("unsupported scale for x-axis")

        line.set_label(path.splitext(file)[0])

    ax.set_xlabel("n")
    # ax.set_ylabel('flops/cycle')
    ax.legend()

    if args.save_path is None:
        plt.show()
    else:
        fig.savefig(args.save_path)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--system", type=str, default="intel", choices=["intel", "amd"])
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
    parser.add_argument(
        "--metric",
        type=str,
        default="fp/c",
        help="which metric to plot on y axis",
        choices=["fp/c", "cycles", "time"],
    )
    parser.add_argument(
        "--x-scale",
        type=str,
        default="log",
        help="scaling of x-axis",
        choices=["log", "linear"],
    )

    args = parser.parse_args()
    main(args)
