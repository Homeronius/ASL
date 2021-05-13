import os
import subprocess
import shutil

import numpy as np
from sklearn.metrics import adjusted_rand_score

def read_dataset(path):
    data = np.loadtxt(open(path, "rb"), delimiter=",", skiprows=1)
    return data[:,:-1], data[:,-1]


def main():
    # Define the reference binary and test binary to use for the End-to-End test
    bin_rel_path = os.path.relpath('../build/bin/')
    reference_bin = 'hdbscan'
    test_bin  = 'hdbscan_vec'
    # Directory containing the datasets to test against
    test_data_dir = './test_data/'
    
    reference_bin = os.path.join(bin_rel_path, reference_bin)
    test_bin = os.path.join(bin_rel_path, test_bin)

    # Create temporary dir to store outputs to
    out_dir = 'output_tmp'
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    passed = 0
    sorted_filelist = sorted(os.listdir(test_data_dir))
    n_tests = len(sorted_filelist)
    # Actual testing
    for i, file in enumerate(sorted_filelist):
        if file.endswith('.csv'):
            file_full_path = os.path.join(test_data_dir, file)
            
            test_path = '{}/test_{}.csv'.format(out_dir, i)
            reference_path = '{}/ref_{}.csv'.format(out_dir, i)
            # Compute cluster labels
            subprocess.run([test_bin, file_full_path, test_path], capture_output=True)
            subprocess.run([reference_bin, file_full_path, reference_path], capture_output=True)

            # Compare the outputs with adjusted_rand_score
            _, y_test = read_dataset(test_path)
            _, y_reference = read_dataset(reference_path)

            score = adjusted_rand_score(y_test, y_reference)
            eps = 1e-5
            if score < 1.0-eps:     # failed
                print("[\u2717] {}".format(file))
            else:                   # passed
                print("[\u2713] {}".format(file))
                passed += 1

    print("====================")
    print("{} / {} tests failed!".format(n_tests-passed, n_tests))

    # Remove temporary output dir
    shutil.rmtree('output_tmp')


if __name__ == "__main__":
    main()
