import matplotlib.pyplot as plt
import numpy as np
from math import log2
from collections import namedtuple
import seaborn as sns

sns.set_theme()

def create_roofline(beta, roof_1=None, roof_2=None, out_filename=None):
    
    # Define Rooflines
    roofline = lambda i : np.minimum(i*beta, roof_1.pi)
    if roof_2 is not None: roofline_vectorized = lambda i : np.minimum(i*beta, roof_2.pi)
    
    # Create data for rooflines (with padding left and right)
    N = 100
    if roof_2 is not None:
        x_extent = [min(min(roof_1.OI, roof_2.OI)), max(max((roof_1.OI, roof_2.OI)))]
        x_padding = 2 * (x_extent[1] - x_extent[0])
        x_range = [log2((min(x_extent[0], min(roof_1.pi, roof_2.pi)/beta)) - x_padding), \
                   log2((max(x_extent[1], max(roof_1.pi, roof_2.pi)/beta)) + x_padding)]
    else:
        x_extent = [min(roof_1.OI), max(roof_1.OI)]
        x_padding = 2 * (x_extent[1] - x_extent[0])
        x_range = [log2((min(x_extent[0], roof_1.pi/beta)) - x_padding), \
                   log2((max(x_extent[1], roof_1.pi/beta)) + x_padding)]
                                                  
    x = np.logspace(x_range[0], x_range[1], N, base=2.0)
    y = roofline(x)
    if roof_2 is not None: y_vectorized = roofline_vectorized(x)

    fig = plt.figure(figsize=(8,8))
    ax = fig.add_subplot(111)

    # Plot 1st roofline
    ax.plot(x, y, label='scalar_op',  linewidth=2)
    # Plot 2nd roofline
    if roof_2 is not None: ax.plot(x, y_vectorized, label='vec_op',  linewidth=2)

    ax.set_xlabel('OI [FLOPs / byte]')
    ax.set_ylabel('Performance [FLOPs / cycle]')

    ax.set_yscale('log', base=2)
    ax.set_xscale('log', base=2)

    P_restricted = []
    OI_merged = []
    labels_merged = []
    markers_merged = []
    
    # Gather measurements from all rooflines
    OI_merged.extend(roof_1.OI)
    P_restricted.extend([min(roofline(roof_1.OI[i]), roof_1.P[i]) for i in range(len(roof_1.OI))])
    labels_merged.extend(roof_1.labels)
    markers_merged.extend(roof_1.markers)
    
    if roof_2 is not None:
        OI_merged.extend(roof_2.OI)
        P_restricted.extend([min(roofline_vectorized(roof_2.OI[i]), roof_2.P[i]) for i in range(len(roof_2.OI))])
        labels_merged.extend(roof_2.labels)
        markers_merged.extend(roof_2.markers)
    
    # Plot all measurements
    for i in range(len(P_restricted)):
        ax.scatter(OI_merged[i], P_restricted[i], label=labels_merged[i], marker=markers_merged[i], s=90)
        
    ax.legend()
    # Save to file if requested
    if out_filename is not None:
        fig.savefig(out_filename)

def main():
	# Solution from Exercise 4.3 c)

	# Define hardware properties
	pi = 4 # [FLOP / cycle]
	pi_vec = 16 # [FLOP / cycle]
	beta = 25 # [Byte / cycle]

	# Define algorithms which are limited by the 1st roofline
	OI = [0.1875, 0.1875, 0.15]
	P = [3, 2, 3.75]
	labels = ['comp1', 'comp2', 'comp3']
	markers = ['s', 'o', 'v']

	# Define algorithms which are limited by the 2nd roofline
	OI_vec = [0.1875, 0.1875, 0.15]
	P_vec = [4.6875, 4.6875, 3.75]
	labels_vec = ['comp1_vec', 'comp2_vec', 'comp3_vec']
	markers_vec = ['*', '+' ,'x']


	Roofline = namedtuple('Roofline', ['pi', 'OI', 'P', 'labels', 'markers'])

	sequential = Roofline(pi, OI, P, labels, markers)
	vectorized = Roofline(pi_vec, OI_vec, P_vec, labels_vec, markers_vec)

	create_roofline(beta, roof_1=sequential, roof_2=vectorized,
                 out_filename='showcase_roofline.svg')

if __name__ == "__main__":
    main()
