/***************************************************************
 * CS4670/5670, Fall 2012 Project 4
 * File to be modified #1:
 * svmmath.cpp
 *      a routine for intersecting >2 lines (for vanishing point
 *      computation);
 *      routines for computing the homography for the reference
 *      plane and arbitrary polygons
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
#include <random>

using namespace Eigen;
using namespace std;

/**
 * BestFitIntersect():
 *     INPUT:
 *              lines: the list of 3 or more lines to be intersected
 *           imgWidth: width of image
 *          imgHeight: height of image
 *     OUTPUT:
 *          bestfit: finds the best fit intersection point
 *
 *     See http://www-2.cs.cmu.edu/~ph/869/www/notes/vanishing.txt
 */	
SVMPoint BestFitIntersect(const std::list<SVMLine> &lines, int imgWidth, int imgHeight)
{
    // check that at least 3 lines are provided to be intersected
    if (lines.size() < 2)
    {
        fprintf(stderr, "Not enough lines to compute the best fit.");
        abort();
    }

    // To accumulate stuff
    typedef Matrix<double, Dynamic, 3, RowMajor> Matrix3;

    // Initialize variables
    SVMPoint bestfit;                       // best fit intersection point 
    list<SVMLine>::const_iterator iter;     // iterator for iterating through lines
    int numLines = (int) lines.size();      // number of lines to be intersected
    Matrix3 A = Matrix3::Zero(numLines, 3);	// numLines x 3 matrix used for calculating best fit intersect 

    // Transformation for numerical stability ****DO WE HAVE TO DO THIS****

printf("BestFitIntersect(): %s:%d\n", __FILE__, __LINE__);

    // iterate through lines, filling in matrix A with cross product coefficients
    int i = 0;
    for (iter = lines.begin(); iter != lines.end(); iter++) {
        // Get endpoints of line
        SVMPoint *p1 = iter->pnt1;
        SVMPoint *p2 = iter->pnt2;

        // Store endpoint homogeneous coordinates, where w is taken as 1, into a Vec3d
        Vec3d e1 = Vec3d(p1->u, p1->v, 1);
        Vec3d e2 = Vec3d(p2->u, p2->v, 1);

        printf("Line%d: e1 = %f %f, e2 = %f %f\n", i+1, p1->u, p1->v, p2->u, p2->v); 

        // Compute a homogeneous coordinate vector representing the line as the cross
        // product of its two endpoints
        Vec3d xp = cross(e1,e2); 

        // Store cross product coefficients in matrix A
        A(i,0) = xp[0];
        A(i,1) = xp[1];
        A(i,2) = xp[2];
     
        i++;
    }

    // Find the eigenvector associated with the smallest eigenvalue
    double eval, evec[3];
    MinEig(A, eval, evec);

    // Scale eigenvector so that w = 1, store in bestfit
    bestfit.u = evec[0]/evec[2];
    bestfit.v = evec[1]/evec[2];
    bestfit.w = evec[2]/evec[2];

    return bestfit;
}


/**
 * ConvertToPlaneCoordinate():
 *     INPUT:
 *          points: points in 3d space to be converted into plane coordinates
 *     OUTPUT:
 *          basisPts: vector of points with corresponding plane coordinates
 *            uScale: the final divisor applied to the u coordinates to ensure they are 
 *                    between [0,1]
 *            vScale: the final divisor applied to the v coordinates to ensure they are 
 *                    between [0,1]
 *          
 *     see http://www.cs.cornell.edu/courses/cs4670/2012fa/projects/p4/homography.pdf
 */	
void ConvertToPlaneCoordinate(const vector<SVMPoint>& points, vector<Vec3d>& basisPts, double &uScale, double &vScale)
{
    int numPoints = points.size();  // number of points

    // Initialize uniform random distribution [0-<# of points>-1]
    default_random_engine generator;
    uniform_int_distribution<int> distribution(0, numPoints-1);
    
    // Initialize variables
    vector<int> indexes;			// vector of chosen indexes to avoid repeats
    vector<SVMPoint> points_sample; // vector of SVMPoints to define plane
    int randIndex;					// selection index
    CTransform3x3 Pts;              // matrix used to check for collinearity

    // Get random set of 3 noncollinear points to define a plane 
    while ((int)indexes.size() < 3) {
        // Grab random index
        randIndex = distribution(generator);
        
        // If index has not already been selected, add index and points to their
        // corresponding vectors
        if (find(indexes.begin(), indexes.end(), randIndex) == indexes.end()) {
            points_sample.push_back(points[randIndex]);
            indexes.push_back(randIndex);
            printf("%d -> (%f, %f, %f)\n", randIndex, points[randIndex].X, points[randIndex].Y, points[randIndex].Z); 
        }

        // Upon selecting a third point, check that the 3 points are not collinear
        if ((int)indexes.size() == 3) {
            // Store points into Vec3d's
            Vec3d a = Vec3d(points[0].X, points[0].Y, points[0].Z);
            Vec3d b = Vec3d(points[1].X, points[1].Y, points[1].Z);
            Vec3d c = Vec3d(points[2].X, points[2].Y, points[2].Z);
            
            // Find vectors represented by AB, AC
            Vec3d ab = b - a;
            Vec3d ac = c - a;

            // Calculate cross product AB x AC
            Vec3d xp = cross(ab, ac);

            // If AB x AC == 0, the points are collinear, select a new third point 
            if (xp.iszero()) {
                printf("Points are collinear!\n");
                points_sample.pop_back();
                indexes.pop_back();
            }
        }
    }

    // Convert the 3 sampled points to Vec3d's, arbitrarily selecting one to be the origin
    // These will be used to define a unique plane
    Vec3d p = Vec3d(points_sample[0].X, points_sample[0].Y, points_sample[0].Z);
    Vec3d q = Vec3d(points_sample[1].X, points_sample[1].Y, points_sample[1].Z);
    Vec3d r = Vec3d(points_sample[2].X, points_sample[2].Y, points_sample[2].Z);
    
    // Calculate base vector for x axis in the plane
    Vec3d ex = p - r;
    ex.normalize();

    // Decompose vector q - r into two components (parallel + orthogonal to ex)
    Vec3d s = ((q - r) * ex) * ex;  // component that is parallel to ex
    Vec3d t = (q - r) - s;          // component that is orthogonal to ex
    
    // Calculate base vector for y axis in the plane
    Vec3d ey = t;
    ey.normalize();

    // Initialize variables for keeping track of max/min u and v values
    double uMax = 0, uMin = 1E+37;
    double vMax = 0, vMin = 1E+37;

    // Iterate through points, converting from 3d to planar coordinates
    // while keeping track of max/min u and v values
    for (int i=0; i < numPoints; i++) {
        // Convert point to Vec3d
        Vec3d a = Vec3d(points[i].X, points[i].Y, points[i].Z);
        
        // Calculate two dimensional coordinates u and v
        double u = (a - r) * ex;
        double v = (a - r) * ey;

        // Keep track of max/min u values
        if (u > uMax) {
            uMax = u;
        }
        if (u < uMin) {
            uMin = u;
        }

        // Keep track of max/min v values
        if (v > vMax) {
            vMax = v;
        }
        if (v < vMin) {
            vMin = v;
        }
        
        printf("u:%f, v:%f, uMin:%f, uMax:%f, vMin:%f, vMax%f\n", u, v, uMin, uMax, vMin, vMax); 

        // Push Vec3d of two dimensional planar coordinates to basisPts
        Vec3d tmp = Vec3d(u, v, 1); 
        basisPts.push_back(tmp);
    }

    // Calculate the divisors to to apply to u, v coordinates to ensure they are between [0,1]
    uScale = (uMax - uMin);
    vScale = (vMax - vMin);

    // Iterate through basisPts and normalize
    for (int i=0; i < numPoints; i++) {
        basisPts[i][0] = (basisPts[i][0] - uMin) / uScale;
        basisPts[i][1] = (basisPts[i][1] - vMin) / vScale;
        printf("u:%f, v:%f\n", basisPts[i][0], basisPts[i][1]);
    }
}

/**
 * ComputeHomography():
 *     INPUT:
 *              points: points in 3d space specifying a plane
 *          isRefPlane: boolean specifying whether or not given points determine the 
 *                      reference plane
 *     OUTPUT:
 *                 H: homography from the plane specified by "points" to the image plane
 *              Hinv: inverse of aforementioned homography H
 *          basisPts: vector of points with corresponding plane coordinates
 *          
 *     If the plane is the reference plane (isRefPlane == true), don't convert the
 *     coordinate system to the plane. Only do this for polygon patches where
 *     texture mapping is necessary.
 *     Coordinate system conversion is to be implemented in a separate routine
 *     ConvertToPlaneCoordinate().
 *     see http://www.cs.cornell.edu/courses/cs4670/2012fa/projects/p4/homography.pdf
 */
void ComputeHomography(CTransform3x3 &H, CTransform3x3 &Hinv, const vector<SVMPoint> &points, vector<Vec3d> &basisPts, bool isRefPlane)
{
    // Get number of points, check that at least 4 points are provided 
    int numPoints = (int) points.size();
    assert( numPoints >= 4 );

    basisPts.clear();

    // Convert from 3d coordinates ("points") to 2d plane coordinates ("basisPts")
    int i;
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
    
    for (i = 0; i < numPoints; i++) {
        const SVMPoint a = points[i];
        const Vec3d b = basisPts[i];

        // Fill in the matrix A in this loop for the call to MinEig
        //
        //     +----------------------------------+
        //     | u  v  1  0  0  0  -uu'  -vu' -u' | where (u,v) are plane coordinates and
        // A = | 0  0  0  u  v  1  -uv'  -vv' -v' | (u', v') are image coordinates) 
        //     |                :                 |
        //     +----------------------------------+ 
        //	
        A(2*i,0) = b[0];
        A(2*i,1) = b[1];
        A(2*i,2) = 1;
        A(2*i,3) = 0;
        A(2*i,4) = 0;
        A(2*i,5) = 0;
        A(2*i,6) = -b[0] * a.u;
        A(2*i,7) = -b[1] * a.u;
        A(2*i,8) = -a.u;
        A(2*i+1,0) = 0;
        A(2*i+1,1) = 0;
        A(2*i+1,2) = 0;
        A(2*i+1,3) = b[0];
        A(2*i+1,4) = b[1];
        A(2*i+1,5) = 1;
        A(2*i+1,6) = -b[0] * a.v;
        A(2*i+1,7) = -b[1] * a.v;
        A(2*i+1,8) = -a.v;
    }
    
    // Find the eigenvector associated with the smallest eigenvalue
    double eval, h[9];
    MinEig(A, eval, h);

    // Fil in homography H with values from the eigenvector associated with 
    // the smallest eigenvalue
    H[0][0] = h[0];
    H[0][1] = h[1];
    H[0][2] = h[2];

    H[1][0] = h[3];
    H[1][1] = h[4];
    H[1][2] = h[5];

    H[2][0] = h[6];
    H[2][1] = h[7];
    H[2][2] = h[8];

    // Compute inverse of H
    if (H.Determinant() == 0)
        fl_alert("Computed homography matrix is uninvertible \n");
    else
        Hinv = H.Inverse();

    // Output H and Hinv matrices
    int ii;
    printf("\nH=[\n");
    for (ii=0; ii<3; ii++)
        printf("%e\t%e\t%e;\n", H[ii][0]/H[2][2], H[ii][1]/H[2][2], H[ii][2]/H[2][2]);
    printf("]\nHinv=[\n");

    for (ii=0; ii<3; ii++)
        printf("%e\t%e\t%e;\n", Hinv[ii][0]/Hinv[2][2], Hinv[ii][1]/Hinv[2][2], Hinv[ii][2]/Hinv[2][2]);

    printf("]\n\n");
}

