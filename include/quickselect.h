#ifndef _HDBSCAN_QUICKSELECT_H_
#define _HDBSCAN_QUICKSELECT_H_

double quickselect(double *list, int left, int right, int k);
// double iterative_quickselect(double *list, int left, int right, int k);
double iterative_quickselect(double *list, int n, int k);
void BuildPackMask();
#endif // _HDBSCAN_QUICKSELECT_H_