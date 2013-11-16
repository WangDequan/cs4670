#ifndef JACOB_H
#define JACOB_H

void nrerror(char error_text[]);

double *nrvector(long nl, long nh);
double **nrmatrix(long nrl, long nrh, long ncl, long nch);
void free_nrvector(double *v, long nl, long nh);
void free_nrmatrix(double **m, long nrl, long nrh, long ncl, long nch);

void jacobi(double **a, int n, double d[], double **v, int *nrot);

#endif