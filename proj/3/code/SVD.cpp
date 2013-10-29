/* SVD.cpp */

#include "SVD.h"
#include "Eigen/SVD"

/* Compute the SVD of a matrix A, returning the matrix V^T and the singular values */
void SVD(const AMatrixType &A, AMatrixType &Vt, VectorXd &sv)
{
	// compute the svd of A
	JacobiSVD<AMatrixType> svd(A, ComputeFullV);

	Vt = svd.matrixV().transpose();
	sv = svd.singularValues();
}
