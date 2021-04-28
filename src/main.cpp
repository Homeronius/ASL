#include "distance.h"
#include "utils.h"

#include <cstdlib>
#include <cstdio>
#include <cstddef>

int main() {

    double *dataset = NULL;
    int *labels = NULL;
    int shape[2];
    const char *filename = "../data/blobs_0.csv";

    read_csv(&dataset, &labels, &shape, filename);

    int n = shape[0];
    int d = shape[1];
	printf("Loaded dataset of size [%d, %d]\n\n", n, d);

	printf("DATA\n");
	for(int i = 0; i < 3; ++i){
		for(int j = 0; j < d; ++j){
			printf("%f, ", dataset[i*d + j]);
		}
		printf("\n");
	}
	printf("...\n\n");

	printf("LABELS\n");
	for(int i = 0; i < 3; ++i){
		printf("%d\n", labels[i]);
	}
	printf("...\n\n");

    free(dataset);
    free(labels);

    return 0;

}
