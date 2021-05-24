from os import path
import argparse
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns


def read_dataset(path, system='intel'):
    data = np.loadtxt(open(path, "rb"), delimiter=",", skiprows=1)
    if system == 'intel':
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
    ax.set_title(r"$\bf{Preliminary\ performance\ plot}$" + "\n [flops/cycle]",
                 loc='left')

    for file in args.files[0]:
        fpath = path.join(datadir, file)
        N, flops, cycles, time = read_dataset(fpath, args.system)
        # flops *= 1e-9
        y = np.divide(flops, cycles)
        # line, = ax.plot(N, y, linestyle='-', marker='o')
        line, = ax.semilogx(N, y, linestyle='-', marker='o', base=2)
        line.set_label(file)

    ax.set_xlabel('n')
    # ax.set_ylabel('flops/cycle')
    ax.legend()

    plt.show()

    fig.savefig(args.save_path)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--system",
        type=str,
        default='intel',
        choices=['intel', 'amd']
    )
    parser.add_argument(
        "--data-path",
        type=str,
        default='data/timings/',
        help='relative path to where timings are stored'
    )
    parser.add_argument(
        "--files",
        type=str,
        nargs='+',
        required=True,
        action='append',
        help='name of one or more .csv performance measurements'
    )
    parser.add_argument(
        "--save-path",
        type=str,
        default='plot.png',
        help='path or filename to store plot to'
    )

    args = parser.parse_args()
    main(args)
