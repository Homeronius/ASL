from os import path
import argparse
from typing import Type
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from math import log2

AMD = r" $\bf{AMD\ Ryzen\ 7\ 4800H\ (Zen\ 2),\ 2.9GHz}$"
INTEL = r" $\bf{Intel\ i5-7300U\ (Kaby\ Lake),\ 2.6GHz}$"


def format_func(value, tick_number):
    # find exponent for tick
    return r"$2^{{ {:} }}$".format(int(log2(value)))


def read_dataset(path, system="intel"):
    data = np.loadtxt(open(path, "rb"), delimiter=",", skiprows=1)
    if system == "intel":
        return data[:, 0], data[:, 1], data[:, 6], data[:, 2], data[:, 3], data[:, 10]
    else:
        return data[:, 0], data[:, 1], data[:, 6], data[:, 2], data[:, 3], data[:, 11]


def main(args):
    datadir = path.relpath(args.data_path)
    if not path.exists(datadir):
        raise Exception("directory {} does not exist".format(datadir))

    sns.set_theme()

    fig = plt.figure()
    ax = fig.add_subplot(111)
    system_name = INTEL if args.system == "intel" else AMD
    title = (
        r"$\bf{Performance\ on}$" if args.metric == "fp/c" else r"$\bf{Runtime\ on}$"
    )

    title += system_name
    if args.metric == "fp/c":
        title += "\n [flops/cycle]"
    elif args.metric == "time":
        title += "\n [seconds]"
    elif args.metric == "cycles":
        title += "\n [" + r"$\times 10^9$" + " cycles]"
    ax.set_title(title, loc="left")

    mpts_mode = False

    freq = 2.6e9 if args.system == "intel" else 2.9e9
    for file in args.files[0]:
        fpath = path.join(datadir, file)
        N, D, flops, cycles, time, mpts = read_dataset(fpath, args.system)

        # hack hack hack
        mpts_mode = all(elem == N[0] for elem in N) and all(elem == D[0] for elem in D)
        dim_mode = not mpts_mode and all(elem == N[0] for elem in N)
        # flops *= 1e-9
        if args.metric == "fp/c":
            y = np.divide(flops, cycles)
        elif args.metric == "time":
            y = cycles / freq
        elif args.metric == "cycles":
            y = cycles / 1e9
        else:
            raise TypeError("unsupported metric to plot")

        if not mpts_mode and not dim_mode:
            N = N[2:]
            y = y[2:]

        X_axis = []
        if mpts_mode:
            X_axis = mpts
        elif dim_mode:
            X_axis = D
        else:
            X_axis = N

        if args.x_scale == "linear":
            (line,) = ax.plot(X_axis, y, linestyle="-", marker="o")
            ax.xaxis.set_major_locator(plt.MultipleLocator(2048))
            # ax.xaxis.set_major_locator(plt.LogLocator(base=2, subs=[16]))
            # ax.xaxis.set_major_formatter(plt.FuncFormatter(format_func))
            # ax.set_xlim(right=17000)
        elif args.x_scale == "log":
            (line,) = ax.semilogx(X_axis, y, linestyle="-", marker="o", base=2)
        else:
            raise TypeError("unsupported scale for x-axis")

        line.set_label(path.splitext(file)[0])

    ax.set_xlabel("k" if mpts_mode else ("d" if dim_mode else "n"))
    # ax.set_ylabel('flops/cycle')
    ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))

    if args.save_path is None:
        plt.show()
    else:
        fig.savefig(args.save_path, bbox_inches="tight")


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
