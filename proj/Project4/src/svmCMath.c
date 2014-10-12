/* svmCMath.c */

#include "svmCMath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

/* Fill a given matrix with an n x n identity matrix */
void matrix_ident(int n, double *A) {
    int i, j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i == j)
                A[i * n + j] = 1.0;
            else
                A[i * n + j] = 0.0;
        }
    }
}

/* Transpose the m x n matrix A and put the result in the n x m matrix AT */
void matrix_transpose(int m, int n, double *A, double *AT) {
    int i, j;
    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            AT[j * m + i] = A[i * n + j];
}

/* Scale a matrix by a scalar */
void matrix_scale(int m, int n, double *A, double s, double *R) {
    int i;
    int entries = m * n;

    for (i = 0; i < entries; i++) {
        R[i] = A[i] * s;
    }
}

/* Compute the matrix difference R = A - B */
void matrix_diff(int Am, int An, int Bm, int Bn, double *A, double *B, double *R) {
    int r = Am;
    int c = An;
    int n = r * c, i;

    if (Am != Bm || An != Bn) {
        printf("[matrix_sum] Error: mismatched dimensions\n");
        return;
    }

    for (i = 0; i < n; i++) {
        R[i] = A[i] - B[i];
    }    
}

/* Compute the matrix product R = AB */
void matrix_product(int Am, int An, int Bm, int Bn, 
                    const double *A, const double *B, double *R) {
    int r = Am;
    int c = Bn;
    int m = An;

    int i, j, k;

    if (An != Bm) {
        printf("[matrix_product] Error: the number of columns of A and the "
               "number of rows of B must be equal\n");
        return;
    }

    for (i = 0; i < r; i++) {
        for (j = 0; j < c; j++) {
            R[i * c + j] = 0.0;
            for (k = 0; k < m; k++) {
                R[i * c + j] += A[i * An + k] * B[k * Bn + j];
            }
        }
    }
}

/* Compute the matrix product R = A^T B */
void matrix_transpose_product(int Am, int An, int Bm, int Bn, double *A, double *B, double *R) {
    int r = An;
    int c = Bn;
    int m = Am;

    int i, j, k;

    if (Am != Bm) {
        printf("Error: the number of rows of A and the "
               "number of rows of B must be equal\n");
        return;
    }

    for (i = 0; i < r; i++) {
        for (j = 0; j < c; j++) {
            R[i * c + j] = 0.0;
            for (k = 0; k < m; k++) {
                R[i * c + j] += A[k * An + i] * B[k * Bn + j];
            }
        }
    }
}

/* Compute the matrix product R = A B^T */
void matrix_transpose_product2(int Am, int An, int Bm, int Bn, double *A, double *B, double *R) {
    int r = Am;
    int c = Bm;
    int m = An;

    int i, j, k;

    if (An != Bn) {
        printf("Error: the number of columns of A and the "
               "number of columns of B must be equal\n");
        return;
    }
    
    for (i = 0; i < r; i++) {
        for (j = 0; j < c; j++) {
            R[i * c + j] = 0.0;
            for (k = 0; k < m; k++) {
                R[i * c + j] += A[i * An + k] * B[j * Bn + k];
            }
        }
    }
}
