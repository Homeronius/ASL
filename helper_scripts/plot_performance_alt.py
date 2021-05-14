import sys
import argparse
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.metrics import adjusted_rand_score
from matplotlib.colors import ListedColormap


def read_dataset(path, system='intel'):
    data = np.loadtxt(open(path, "rb"), delimiter=",", skiprows=1)
    if system == 'intel':
        return data[:, 0], data[:, 1], data[:, 2], data[:, 3]
    else:
        return data[:, 0], data[:, 6], data[:, 2], data[:, 3]

def main(args):
    sns.set_theme()

    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.set_title('Preliminary performance plot (flops/cycle)')

    for file in args.data[0]:
        N, flops, cycles, time = read_dataset(file, args.system)
        # flops *= 1e-9
        y = np.divide(flops, cycles)
        line, = ax.plot(N, y)
        line.set_label(file)
    
    ax.set_xlabel('dataset size')
    ax.set_ylabel('flops/cycle')
    ax.legend()

    plt.show()

    fig.savefig('test_amd.png')


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--system",
        type=str,
        default='intel',
        choices=['intel', 'amd']
    )
    parser.add_argument(
        "--data",
        type=str,
        nargs='+',
        required=True,
        action='append',
        help='path to one or more .csv performance measurements'
    )

    args = parser.parse_args()
    main(args)
