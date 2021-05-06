#include <fstream>
#include <sstream>
#include <string>

#include <cluster.h>
/**
 * @brief Function to load a dataset (X and y) in csv format
 *        First line being metadata header of form (n_samples, n_features, std
 * of blobs)
 *
 * @param dataset NULL-pointer to store dataset to.
 * @param labels NULL-pointer to store corresponding labels
 * @param shape of X matrix of datapoints [n, d]
 * @param fname path to dataset
 */
void read_csv(double **dataset, int **labels, int (*shape)[2],
              const char *fname) {

  // Open filestream
  std::ifstream delimited_file(fname);

  if (!delimited_file.is_open())
    throw std::runtime_error(
        "Could not open file, check if passing the correct relative path to "
        "the data/ directory, or if it even exists");

  std::string line, size, element;

  if (delimited_file.good()) {
    std::getline(delimited_file, line);
    std::stringstream header_stream(line);

    // extract the header information (m, n, std) : (n_samples, n_features, std
    // of blobs)
    std::getline(header_stream, size, ',');
    (*shape)[0] = std::atoi(size.c_str());
    std::getline(header_stream, size, ',');
    (*shape)[1] = std::atoi(size.c_str());

    // allocate memory for dataset
    int n = (*shape)[0]; // n_samples
    int d = (*shape)[1]; // n_features

    // allocate memory (todo check if aligned memory is needed here)
    *dataset = reinterpret_cast<double *>(malloc(n * d * sizeof(double)));
    *labels = reinterpret_cast<int *>(malloc(n * sizeof(int)));

    // extract data
    for (int i = 0; i < n; ++i) {
      std::getline(delimited_file, line);
      // create a stringstream of the current line
      std::stringstream ss(line);

      // read one data point at a time
      for (int j = 0; j < d; ++j) {
        std::getline(ss, element, ',');
        (*dataset)[i * d + j] = std::stod(element.c_str());
      }

      // read label for corre i-th data point
      std::getline(ss, element);
      (*labels)[i] = static_cast<int>(std::stod(element.c_str()));
    }
  }

  delimited_file.close();
}

void write_csv(double *dataset, int *labels, int shape[2], const char *fname) {

  // Open filestream
  std::ofstream delimited_file(fname);

  int n = shape[0];
  int d = shape[1];

  if (!delimited_file.is_open())
    throw std::runtime_error(
        "Could not open file, check if passing the correct relative path to "
        "the data/ directory, or if it even exists");

  if (delimited_file.good()) {
    // write header
    delimited_file << n << ",";
    delimited_file << d << "\n";

    // write data
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < d; ++j) {
        delimited_file << dataset[i * d + j] << ",";
      }
      delimited_file << labels[i] << "\n";
    }
  }

  delimited_file.close();
}

void condensed_tree_to_csv(std::vector<Cluster *> tree, const char *fname) {
  // Open filestream
  std::ofstream delimited_file(fname);
  if (!delimited_file.is_open())
    throw std::runtime_error(
        "Could not open file, check if passing the correct relative path to "
        "the data/ directory, or if it even exists");

  if (delimited_file.good()) {
    // assuming preorder

    delimited_file << "id,weight,size,selected,child1_id,child2_id\n";
    for (size_t i = 0; i < tree.size(); i++) {
      Cluster *c = tree[i];
      delimited_file << c << "," << c->get_cluster_weight() << ","
                     << c->components.size() << "," << c->selected << ","
                     << c->child1 << "," << c->child2 << "\n";
    }
  }
  delimited_file.close();
}
