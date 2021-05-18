#include "union_find.h"
#include <assert.h>
#include <cassert>
#include <vector>

Union_find::Union_find(int set_size, int cluster_size) {
  minimum_cluster_size = cluster_size;
  assert(minimum_cluster_size > 2 &&
         "Smallest clusters must be greater than 2");

  size = set_size;
  number_of_components = size;
  sz = (int *)malloc(size * sizeof(int));
  id = (int *)malloc(size * sizeof(int));

  for (size_t i = 0; i < size; i++) {
    sz[i] = 1;
    id[i] = i;
  }
}

Union_find::~Union_find() {
  free(sz);
  free(id);
}

int Union_find::find(int p) {
  int root = p;
  // find root
  while (root != id[root])
    root = id[root];

  // path compression
  while (p != root) {
    int next = id[p];
    id[p] = root;
    p = next;
  }

  return root;
}

void Union_find::merge_clusters(int root1, int root2, float_t distance) {

#ifdef HDBSCAN_VERBOSE
  printf("Merging Cluster: %04d-%04d\tsizes: %04d,%04d\t", root1, root2,
         sz[root1], sz[root2]);
#endif

  float_t lambda = 1 / distance;

  Cluster *c1 = nullptr;
  Cluster *c2 = nullptr;
  for (auto &&c : open_clusters) {
    if (c->root_id == root1) {
      c1 = c;
    } else if (c->root_id == root2) {
      c2 = c;
    }
  }

  if (c1 == nullptr && c2 == nullptr) { // create new cluster
#ifdef HDBSCAN_VERBOSE
    printf("Case 1\n");
#endif

    std::vector<int> components;
    int root_id;
    if (sz[root1] < sz[root2]) {
      root_id = root2;
    } else {
      root_id = root1;
    }
    for (size_t i = 0; i < number_of_components; i++) {
      if (find(i) == root1 || find(i) == root2) {
        components.push_back(i);
      }
    }
    open_clusters.push_back(new Cluster(lambda, root_id, components));

#ifdef HDBSCAN_VERBOSE
    printf("Merged Cluster: %04d\tsize: %04zu\n", root_id, components.size());
#endif

  } else if (c1 == nullptr && c2 != nullptr) { // add leaf to c2
#ifdef HDBSCAN_VERBOSE
    printf("Case 2a\n");
#endif

    std::vector<int> components;
    for (size_t i = 0; i < number_of_components; i++) {
      if (find(i) == root1) {
        components.push_back(i);

#ifdef HDBSCAN_VERBOSE
        printf("comps:%zu\n", components.size());
#endif
      }
    }
    c2->add_leaf(components, lambda);

#ifdef HDBSCAN_VERBOSE
    printf("comps:%zu\n", components.size());
    printf("Merged Cluster: %04d\tsize: %04zu\n", c2->root_id,
           c2->components.size());
#endif

  } else if (c1 != nullptr && c2 == nullptr) { // add leaf to c1
#ifdef HDBSCAN_VERBOSE
    printf("Case 2b\n");
#endif

    std::vector<int> components;
    for (size_t i = 0; i < number_of_components; i++) {
      if (find(i) == root2) {
        components.push_back(i);
      }
    }
    c1->add_leaf(components, lambda);

#ifdef HDBSCAN_VERBOSE
    printf("comps:%zu\n", components.size());
    printf("Merged Cluster: %04d\tsize: %04zu\n", c1->root_id,
           c1->components.size());
#endif
  } else if (c1 != nullptr && c2 != nullptr) { // add merge c1 and c2
#ifdef HDBSCAN_VERBOSE
    printf("Case 3\n");
#endif

    Cluster *c_new = new Cluster(c1, c2, lambda);
    c1->finalize(c_new, lambda);
    c2->finalize(c_new, lambda);
    finished_clusters.push_back(c1);
    finished_clusters.push_back(c2);
    open_clusters.remove(c1);
    open_clusters.remove(c2);
    open_clusters.push_back(c_new);

#ifdef HDBSCAN_VERBOSE
    printf("Merged Cluster: %04d\tsize: %04zu\n", c_new->root_id,
           c_new->components.size());
#endif

    for (auto &&c : open_clusters) {
      assert(c1 != c);
      assert(c2 != c);
    }
  } else {
    assert(false && "no case chosen");
  }
}

void Union_find::unify(int p, int q, float_t distance) {

  if (connected(p, q))
    return; // may be redundant for our use?

  int root1 = find(p);
  int root2 = find(q);

  if (sz[root1] + sz[root2] >=
      minimum_cluster_size) { // union will be valid cluster
    merge_clusters(root1, root2, distance);
  }

  if (sz[root1] < sz[root2]) {
    sz[root2] += sz[root1];
    id[root1] = root2;
  } else {
    sz[root1] += sz[root2];
    id[root2] = root1;
  }

  number_of_components--;
}

bool Union_find::connected(int p, int q) { return find(p) == find(q); }

int Union_find::component_size(int p) { return sz[find(p)]; }

void Union_find::finalize() {
  for (auto &&c : open_clusters) {
    c->finalize(nullptr, 0);
    finished_clusters.push_back(c);
  }
}
