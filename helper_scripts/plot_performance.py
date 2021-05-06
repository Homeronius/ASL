import sys
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.metrics import adjusted_rand_score
from matplotlib.colors import ListedColormap

def read_dataset(path):
    data = np.loadtxt(open(path, "rb"), delimiter=",", skiprows=1)
    return data[:,0], data[:,1], data[:,2], data[:,3]

def plot_performance(N, flops, cycles, time):

    sns.set_theme()

    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.set_title('Preliminary performance plot (flop/cycle)')

    ## to obtain Gflops
    # flops *= 1e-9
    y = np.divide(flops, cycles)

    ax.plot(N, y)
    ax.set_xlabel('dataset_size')
    ax.set_ylabel('flop/cycle')

    plt.show()

    fig.savefig('first_plot_1.svg')

def main():
    if len(sys.argv) < 2:
        print('Usage: plot_performance.py <performance_data>')
        return

    plot_performance(*read_dataset(sys.argv[1]))



if __name__ == "__main__":
    main()
