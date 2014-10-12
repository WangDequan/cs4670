/* MinEig.cpp */

#include "MinEig.h"
#include "Eigen/SVD"

#include <cstdio>

using namespace Eigen;

/*
 * Helper function:
 * void MinEig(M, eval, evec, n)
 *		Given an n by n semi-positive-definite matrix M,
 *		returns the eigenvector for the smallest eigenvalue in evec.
 *		Will be useful for both line intersection and homography
 */	
// template <int N> 
// void MinEig(Matrix<double, N, N, RowMajor> A, double &eval, double *evec)
template <class T>
void MinEig(T A, double &eval, double *evec)
{	
	int numCols = A.cols();

	// compute the SVD
	JacobiSVD<T> svd(A, ComputeFullV);

	// return the smallest eigenvector
	for (int i = 0; i < numCols; i++) {
		evec[i] = (svd.matrixV())(i,numCols-1);
	}

	// return the smallest eigenvalue
	if (svd.singularValues().size() < numCols)
		eval = 0.0;
	else
		eval = (svd.singularValues())(numCols-1);

	// eigenvector associated with smallest eigenvalue put in x,y,z 
	printf("eigenvector associated with smallest eigenvalue:\n");
	for (int i = 0; i < numCols ; i++) {
		printf("%f\n", evec[i]);
	}
	printf("\n");
}

// this is a silly function that will 'pre-compile' templated versions of MinEig in order to reduce compile times
void dummy_init_template()
{
	typedef Matrix<double, Dynamic, 3, RowMajor> Matrix3;
	Matrix3 A3;
	double eval, evec3[3];
	MinEig(A3, eval, evec3);

	typedef Matrix<double, Dynamic, 9, RowMajor> Matrix9;
	Matrix9 A9;
	double evec9[9];
	MinEig(A9, eval, evec9);
}
