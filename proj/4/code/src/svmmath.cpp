/***************************************************************
 * CS4670/5670, Fall 2012 Project 4
 * File to be modified #1:
 * svmmath.cpp
 *		a routine for intersecting >2 lines (for vanishing point
 *		computation);
 *		routines for computing the homography for the reference
 *		plane and arbitrary polygons
 **************************************************************/

#pragma warning(disable : 4996)

#include "Eigen/Core"
#include "MinEig.h"

#include "svmmath.h"
#include "jacob.h"
#include "vec.h"
#include <cstring>
#include <cstdio>
#include <assert.h>
#include <iostream>

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define DOT(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define PI 3.14159265358979

using namespace Eigen;
using namespace std;

//
// TODO 1: BestFitIntersect()
//		Given lines, the list of 3 or more lines to be intersected,
//		find the best fit intersection point.
//		See http://www-2.cs.cmu.edu/~ph/869/www/notes/vanishing.txt.
//	
SVMPoint BestFitIntersect(const std::list<SVMLine> &lines, int imgWidth, int imgHeight)
{
    // check
    if (lines.size() < 2)
	{
            fprintf(stderr, "Not enough lines to compute the best fit.");
            abort();
	}

    SVMPoint bestfit;
    list<SVMLine>::const_iterator iter;

    // To accumulate stuff
    typedef Matrix<double, Dynamic, 3, RowMajor> Matrix3;

    int numLines = (int) lines.size();
    Matrix3 A = Matrix3::Zero(numLines, 3);	

    // Transformation for numerical stability

    // Note: iterate through the lines list as follows:
    //		for (iter = lines.begin(); iter != lines.end(); iter++) {
    //			...iter is the pointer to the current line...
    //		}
    // Note: Function to find eigenvector with smallest eigenvalue is MinEig(A, eval, evec)
    //
    /******** BEGIN TODO ********/
    double w = ((double)(imgWidth + imgHeight)) / 4;

    if (numLines == 2){
      SVMLine l1 = lines.front(), l2 = lines.back();
      Vec3d e1 = cross(Vec3d(l1.pnt1->u, l1.pnt1->v, w), Vec3d(l1.pnt2->u, l1.pnt2->v, w));
      Vec3d e2 = cross(Vec3d(l2.pnt1->u, l2.pnt1->v, w), Vec3d(l2.pnt2->u, l2.pnt2->v, w));
      Vec3d ex = cross(e1, e2);
      bestfit = SVMPoint(w * ex[0]/ex[2], w * ex[1]/ex[2]); // w to scale back up
    } else {
      double eigenvalue = 0.0;
      double &eigref = eigenvalue;
      double *eigenvector = nrvector(0,2);
      for (iter = lines.begin(); iter != lines.end(); iter++) {
        Vec3d e1 = Vec3d(iter->pnt1->u, iter->pnt1->v, w);
        Vec3d e2 = Vec3d(iter->pnt2->u, iter->pnt2->v, w);
        Vec3d ex = cross(e1, e2);
        for (int i=0;i<3;i++){
          for (int j=0;j<3;j++){
            A(i,j) += ex[MIN(i, j)] * ex[MAX(i,j)];
          }
        }
      }
      MinEig(A, eigref, eigenvector);
      bestfit = SVMPoint(w*eigenvector[0]/eigenvector[2], w*eigenvector[1]/eigenvector[2]);
      free_nrvector(eigenvector, 0, 2);
    }

    /******** END TODO ********/
	
    return bestfit;
}


//
// TODO 2: ConvertToPlaneCoordinate()
//		Given a plane defined by points, converts their coordinates into
//		plane coordinates. See the following document for more detail.
//		http://www.cs.cornell.edu/courses/cs4670/2012fa/projects/p4/homography.pdf.
//      The final divisors you apply to the u and v coordinates should be saved uScale and vScale
//
void ConvertToPlaneCoordinate(const vector<SVMPoint>& points, vector<Vec3d>& basisPts, double &uScale, double &vScale)
{
    int numPoints = points.size();
    assert(numPoints > 2); // in order to define a plane, we need three points
    /******** BEGIN TODO ********/
    Vec4d p = Vec4d(points[0].X, points[0].Y, points[0].Z, points[0].W);
    Vec4d r = Vec4d(points[1].X, points[1].Y, points[1].Z, points[1].W);
    Vec4d prnorm = (p - r); // aka ex
    prnorm.normalize();
    double bestAngle = 1000; // too high value will always be thrown away on first point
    int bestq = -1;
// p and r chosen arbitrarily, but lets find the best q
    for (int i=2;i<numPoints;i++){
        Vec4d q = Vec4d(points[i].X, points[i].Y, points[i].Z, points[i].W);
        Vec4d qrnorm = (q - r);
        qrnorm.normalize();
        double angle = acos(MIN(DOT(prnorm, qrnorm), 1)) * 180 / PI;
        if (abs(angle - 90) < abs(bestAngle - 90)){
          bestAngle = angle;
          bestq = i;
        }
    }
    assert(bestq > 1); // something would have to be horribly wrong for this to fail
    Vec4d q = Vec4d(points[bestq].X, points[bestq].Y, points[bestq].Z, points[bestq].W);

    Vec4d qr = q - r;

    double sum = DOT(prnorm, qr );
    Vec4d s = Vec4d(qr[0] * sum, qr[1] * sum, qr[2] * sum, qr[3] * sum);
    Vec4d ey = qr - s;
    ey.normalize();

    double umin;
    double vmin;
    double umax;
    double vmax;

    for (int i=0;i<numPoints;i++){
      Vec4d a = Vec4d(points[i].X, points[i].Y, points[i].Z, points[i].W);
      Vec3d p = Vec3d(DOT(a - r, prnorm), DOT(a - r, ey), 1);
      basisPts.push_back(p);
      umin = (i == 0 ? p[0] : MIN(umin, p[0]));
      umax = (i == 0 ? p[0] : MAX(umax, p[0]));
      vmin = (i == 0 ? p[1] : MIN(vmin, p[1]));
      vmax = (i == 0 ? p[1] : MAX(vmax, p[1]));
    }

    uScale = umax - umin;
    vScale = vmax - vmin;

    /******** END TODO ********/
}



//
// TODO 3: ComputeHomography()
//		Computes the homography H from the plane specified by "points" to the image plane,
//		and its inverse Hinv.
//		If the plane is the reference plane (isRefPlane == true), don't convert the
//		coordinate system to the plane. Only do this for polygon patches where
//		texture mapping is necessary.
//		Coordinate system conversion is to be implemented in a separate routine
//		ConvertToPlaneCoordinate.
//		For more detailed explaination, see
//		http://www.cs.cornell.edu/courses/cs4670/2012fa/projects/p4/homography.pdf.
//
void ComputeHomography(CTransform3x3 &H, CTransform3x3 &Hinv, const vector<SVMPoint> &points, vector<Vec3d> &basisPts, bool isRefPlane)
{
    int i;
    int numPoints = (int) points.size();
    assert( numPoints >= 4 );

    basisPts.clear();
    if (isRefPlane) // reference plane
    {
        for (i=0; i < numPoints; i++) {
            Vec3d tmp = Vec3d(points[i].X, points[i].Y, points[i].W); // was Z, not W
            basisPts.push_back(tmp);
        }
    } 
    else // arbitrary polygon
    {
        double uScale, vScale; // unused in this function
        ConvertToPlaneCoordinate(points, basisPts, uScale, vScale);
    }

    // A: 2n x 9 matrix where n is the number of points on the plane
    //    as discussed in lecture
    int numRows = 2 * numPoints;
    const int numCols = 9;

    typedef Matrix<double, Dynamic, 9, RowMajor> MatrixType;
    MatrixType A = MatrixType::Zero(numRows, numCols);

    /******** BEGIN TODO ********/
    /* Fill in the A matrix for the call to MinEig */

    for (i=0;i<numPoints; i++){
      double x1 = basisPts[i][0], y1 = basisPts[i][1];
      double x1p = -points[i].u, y1p = -points[i].v;
      A(2*i,0) = x1;        A(2*i,0) = y1;          A(2*i,2) = 1;
      A(2*i,3) = 0;         A(2*i,4) = 0;           A(2*i,5) = 0;
      A(2*i,6) = x1p * x1;  A(2*i,7) = x1p * y1;    A(2*i,8) = x1p;

      A(2*i+1,0) = 0;       A(2*i+1,1) = 0;         A(2*i+1,2) = 0;
      A(2*i+1,3) = x1;      A(2*i+1,4) = y1;        A(2*i+1,5) = 1;
      A(2*i+1,6) = y1p*x1;  A(2*i+1,7) = y1p*x1;    A(2*i+1,8) = y1p;
    }

    double eval, h[9];
    MinEig(A, eval, h);

    H[0][0] = h[0];
    H[0][1] = h[1];
    H[0][2] = h[2];

    H[1][0] = h[3];
    H[1][1] = h[4];
    H[1][2] = h[5];

    H[2][0] = h[6];
    H[2][1] = h[7];
    H[2][2] = h[8];

    /******** END TODO ********/

    // compute inverse of H
    if (H.Determinant() == 0)
        fl_alert("Computed homography matrix is uninvertible \n");
    else
        Hinv = H.Inverse();

    int ii;
    printf("\nH=[\n");
    for (ii=0; ii<3; ii++)
        printf("%e\t%e\t%e;\n", H[ii][0]/H[2][2], H[ii][1]/H[2][2], H[ii][2]/H[2][2]);
    printf("]\nHinv=[\n");

    for (ii=0; ii<3; ii++)
        printf("%e\t%e\t%e;\n", Hinv[ii][0]/Hinv[2][2], Hinv[ii][1]/Hinv[2][2], Hinv[ii][2]/Hinv[2][2]);

    printf("]\n\n");
}

