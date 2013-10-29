/* SVD.h */

#include "Eigen/Core"
using namespace Eigen;

typedef Matrix<double, Dynamic, 9, RowMajor> AMatrixType;

/* SVD function */
void SVD(const AMatrixType &A, AMatrixType &Vt, VectorXd &sv);
