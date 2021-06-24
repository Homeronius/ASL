import matplotlib.pyplot as plt
import numpy as np
from math import log2
from collections import namedtuple
import seaborn as sns

sns.set_theme()


def create_roofline(beta, roof_1=None, roof_2=None, out_filename=None, system="Intel"):

    # Define Rooflines
    roofline = lambda i: np.minimum(i * beta, roof_1.pi)
    if roof_2 is not None:
        roofline_vectorized = lambda i: np.minimum(i * beta, roof_2.pi)

    # Create data for rooflines (with padding left and right)
    N = 100
    if roof_2 is not None:
        x_extent = [min(min(roof_1.OI, roof_2.OI)), max(max((roof_1.OI, roof_2.OI)))]
        x_padding = 2 * (x_extent[1] - x_extent[0])
        # x_range = [
        # log2((min(x_extent[0], min(roof_1.pi, roof_2.pi) / beta)) - x_padding),
        # log2((max(x_extent[1], max(roof_1.pi, roof_2.pi) / beta)) + x_padding)
        # ]
        x_range = [
            # Some heuristics
            log2(0.01),
            log2((max(x_extent[1], max(roof_1.pi, roof_2.pi) / beta)) + x_padding),
        ]
    else:
        x_extent = [min(roof_1.OI), max(roof_1.OI)]
        x_padding = 2 * (x_extent[1] - x_extent[0])
        # x_range = [
        # log2((min(x_extent[0], roof_1.pi / beta)) - x_padding),
        # log2((max(x_extent[1], roof_1.pi / beta)) + x_padding)
        # ]
        x_range = [
            log2((min(x_extent[0], roof_1.pi / beta))),
            log2((max(x_extent[1], roof_1.pi / beta))),
        ]

    x = np.logspace(x_range[0], x_range[1], N, base=2.0)
    y = roofline(x)
    if roof_2 is not None:
        y_vectorized = roofline_vectorized(x)

    fig = plt.figure()
    # fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111)

    # Plot 1st roofline
    ax.plot(x, y, label="Scalar roofline", linewidth=2)
    # Plot 2nd roofline
    if roof_2 is not None:
        ax.plot(x, y_vectorized, label="Vector roofline", linewidth=2)

    ax.set_xlabel("I [flops/byte]")
    # ax.set_ylabel("Performance [FLOPs / cycle]")
    title = r"$\bf{Roofline\ plot\ on\ AMD\ Ryzen\ 7\ 4800H\ (Zen\ 2),\ 2.9GHz}$"
    title += "\n Performance [flops/cycle]"
    ax.set_title(title, loc="left")

    ax.set_yscale("log", base=2)
    ax.set_xscale("log", base=2)

    P_restricted = []
    OI_merged = []
    labels_merged = []
    markers_merged = []

    # Gather measurements from all rooflines
    OI_merged.extend(roof_1.OI)
    P_restricted.extend(
        [min(roofline(roof_1.OI[i]), roof_1.P[i]) for i in range(len(roof_1.OI))]
    )
    labels_merged.extend(roof_1.labels)
    markers_merged.extend(roof_1.markers)

    # Optionally add data from second roofline
    if roof_2 is not None:
        OI_merged.extend(roof_2.OI)
        P_restricted.extend(
            [
                min(roofline_vectorized(roof_2.OI[i]), roof_2.P[i])
                for i in range(len(roof_2.OI))
            ]
        )
        labels_merged.extend(roof_2.labels)
        markers_merged.extend(roof_2.markers)

    # Plot all measurements
    for i in range(len(P_restricted)):
        ax.scatter(
            OI_merged[i],
            P_restricted[i],
            label=labels_merged[i],
            marker=markers_merged[i],
            s=110,
        )

    # ax.legend()
    ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))

    # Save to file if requested
    if out_filename is not None:
        out_filename = "plots/roofline/" + out_filename + "_" + system + ".pdf"
        fig.savefig(out_filename, bbox_inches="tight")


def get_basic_data_full(system):
    if system == "Intel":
        OI = [
            0.184961962919771,
            0.287174387196608,
            0.300471007575244,
            0.317883872804069,
            1.09707165841223,
            1.70252181905201,
        ]
        P = [
            0.556348234141028,
            0.871948888609612,
            0.960406357898148,
            1.09540163823838,
            0.937031545550276,
            0.937452388154464,
        ]
        labels = [
            "basic",
            "basic_distvec",
            "basic_distvec_quickvec",
            "basic_distvec_quickvec_primvec",
            "basic_blocked",
            "basic_triang",
        ]
        markers = ["x", "+", "1", "2", "3", "4"]
    else:
        OI = [0.17909045424534, 0.278472602157544, 0.291366471616787, 0.308249510493958]
        P = [0.635809892483325, 0.937797158145406, 1.08584416268159, 1.28755371235177]
        labels = [
            "basic",
            "basic_distvec",
            "basic_distvec_quickvec",
            "basic_distvec_quickvec_primvec",
        ]
        markers = ["x", "+", "1", "3"]

    return OI, P, labels, markers


def get_basic_data_short(system):
    if system == "Intel":
        OI = [
            0.184961962919771,
            0.317883872804069,
        ]
        P = [0.556348234141028, 1.09540163823838]
    else:
        OI = [0.17909045424534, 0.308249510493958]
        P = [0.635809892483325, 1.28755371235177]

    labels = [
        "basic",
        "basic_distvec_quickvec_primvec",
    ]
    markers = ["x", "+"]
    return OI, P, labels, markers


def get_advprim_data_full(system):
    if system == "Intel":
        OI = [
            0.259328495378513,
            0.291563710353463,
            0.300761044847042,
            0.286720553179578,
        ]
        P = [0.671533159591363, 1.22019982801778, 1.37466157995285, 1.20271114530254]
    else:
        OI = [
            0.256525267221385,
            0.288558240639693,
            0.297660768031818,
            0.282307425294498,
        ]
        P = [0.87654620238442, 1.48726132928545, 1.69627398352711, 1.34414452719806]
    labels = [
        "advprim",
        "advprim_distvec",
        "advprim_distvec_quickvec",
        "advprim_distvec_quickvec_primvec",
    ]
    markers = ["s", "o", "^", "v"]
    return OI, P, labels, markers


def get_advprim_data_report(system):
    OI = [
        0.165667840808535,
        0.573198429579793,
    ]
    P = [0.894395719556098, 2.42976790208966]
    labels = [
        "advprim",
        "advprim_distvec",
    ]
    markers = ["x", "+"]
    return OI, P, labels, markers


def get_basic_data_report(system):
    OI = [
        0.110449336624287,
        0.546927204666479,
    ]
    P = [0.891624333757501, 2.3236811319306]
    labels = [
        "basic",
        "basic_distvec_quickvec_primvec",
    ]
    markers = ["s", "o"]
    return OI, P, labels, markers


def get_advprim_data_short(system):
    if system == "Intel":
        OI = [
            0.259328495378513,
            0.300761044847042,
        ]
        P = [0.671533159591363, 1.37466157995285]
    else:
        OI = [
            0.256525267221385,
            0.297660768031818,
        ]
        P = [0.87654620238442, 1.69627398352711]
    labels = [
        "advprim",
        "advprim_distvec_quickvec",
    ]
    markers = ["s", "^"]
    return OI, P, labels, markers


def showcase():
    # Solution from Exercise 4.3 c)

    # Define hardware properties
    pi = 4  # [FLOP / cycle]
    pi_vec = 16  # [FLOP / cycle]
    beta = 25  # [Byte / cycle]

    # Define algorithms which are limited by the 1st roofline
    OI = [0.1875, 0.1875, 0.15]
    P = [3, 2, 3.75]
    labels = ["comp1", "comp2", "comp3"]
    markers = ["s", "o", "v"]

    # Define algorithms which are limited by the 2nd roofline
    OI_vec = [0.1875, 0.1875, 0.15]
    P_vec = [4.6875, 4.6875, 3.75]
    labels_vec = ["comp1_vec", "comp2_vec", "comp3_vec"]
    markers_vec = ["*", "+", "x"]

    Roofline = namedtuple("Roofline", ["pi", "OI", "P", "labels", "markers"])

    sequential = Roofline(pi, OI, P, labels, markers)
    vectorized = Roofline(pi_vec, OI_vec, P_vec, labels_vec, markers_vec)

    create_roofline(
        beta, roof_1=sequential, roof_2=vectorized, out_filename="showcase_roofline.svg"
    )


def main():
    # system = "Intel"
    system = "AMD"

    # Define hardware properties
    if system == "Intel":
        pi = 4  # [FLOP / cycle]
        pi_vec = 16  # [FLOP / cycle]
        beta = 31.79  # [Byte / cycle]
    else:
        pi = 6  # [FLOP / cycle]
        pi_vec = 24  # [FLOP / cycle]
        beta = 68.27  # [Byte / cycle]

    Roofline = namedtuple("Roofline", ["pi", "OI", "P", "labels", "markers"])

    # All optimizations
    sequential = Roofline(pi, *get_basic_data_report(system))
    vectorized = Roofline(pi_vec, *get_advprim_data_report(system))

    # Only baseline and best
    # sequential = Roofline(pi, *get_basic_data_short(system))
    # vectorized = Roofline(pi_vec, *get_advprim_data_short(system))

    create_roofline(
        beta,
        roof_1=sequential,
        roof_2=vectorized,
        out_filename="roofline",
        system=system,
    )


if __name__ == "__main__":
    main()
