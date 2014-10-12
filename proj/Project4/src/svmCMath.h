/* svmCMath.h */

#ifndef __svm_c_math_h__
#define __svm_c_math_h__

#ifdef __cplusplus
extern "C" {
#endif

void matrix_ident(int n, double *A);
void matrix_transpose(int m, int n, double *A, double *AT);
void matrix_scale(int m, int n, double *A, double s, double *R);
void matrix_diff(int Am, int An, int Bm, int Bn, double *A, double *B, double *R);
void matrix_product(int Am, int An, int Bm, int Bn, 
                    const double *A, const double *B, double *R);
void matrix_transpose_product(int Am, int An, int Bm, int Bn, double *A, double *B, double *R);
void matrix_transpose_product2(int Am, int An, int Bm, int Bn, double *A, double *B, double *R);

#ifdef __cplusplus
}
#endif

#endif /* __svm_c_math_h__ */
