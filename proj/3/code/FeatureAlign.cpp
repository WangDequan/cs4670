///////////////////////////////////////////////////////////////////////////
//
// NAME
//  FeatureAlign.cpp -- image registration using feature matching
//
// SEE ALSO
//  FeatureAlign.h      longer description
//
// Based on code by Richard Szeliski, 2001.
// (modified for CSE576 Spring 2005, and for CS4670, Fall 2012-2013)
//
///////////////////////////////////////////////////////////////////////////

#include "ImageLib/ImageLib.h"
#include "FeatureAlign.h"
#include "SVD.h"

#include <math.h>
#include <iostream>

CTransform3x3 ComputeHomography(const FeatureSet &f1, const FeatureSet &f2,
                                const vector<FeatureMatch> &matches)
{
    int numMatches = (int) matches.size();

    // first, we will compute the A matrix in the homogeneous linear equations Ah = 0
    int numRows = 2 * numMatches; // number of rows of A
    const int numCols = 9;        // number of columns of A

    // this allocates space for the A matrix
    AMatrixType A = AMatrixType::Zero(numRows, numCols);

    for (int i = 0; i < numMatches; i++) {
        const FeatureMatch &m = matches[i];
        const Feature &a = f1[m.id1];
        const Feature &b = f2[m.id2];

        // BEGIN TODO
        // fill in the matrix A in this loop.
        // To access an element of A, use parentheses, e.g. A(0,0)
printf("TODO: %s:%d\n", __FILE__, __LINE__); 

        // END TODO
    }

    // Compute the SVD of the A matrix and get out the matrix V^T and the vector of singular values
    AMatrixType Vt;
    VectorXd sv;
    SVD(A, Vt, sv);

    CTransform3x3 H;
    // BEGIN TODO
    // fill the homography H with the appropriate elements of the SVD
    // To extract, for instance, the V matrix, use svd.matrixV()
printf("TODO: %s:%d\n", __FILE__, __LINE__); 

    // END TODO

    return H;
}


/******************* TO DO *********************
 * alignPair:
 *	INPUT:
 *		f1, f2: source feature sets
 *		matches: correspondences between f1 and f2
 *               Each match in 'matches' contains two feature ids of 
 *               matching features, id1 (in f1) and id2 (in f2).
 *		m: motion model
 *		nRANSAC: number of RANSAC iterations
 *		RANSACthresh: RANSAC distance threshold
 *		M: transformation matrix (output)
 *
 *	OUTPUT:
 *		repeat for nRANSAC iterations:
 *			choose a minimal set of feature matches
 *			estimate the transformation implied by these matches
 *			count the number of inliers
 *		for the transformation with the maximum number of inliers,
 *		compute the least squares motion estimate using the inliers,
 *		and store it in M
 */
int alignPair(const FeatureSet &f1, const FeatureSet &f2,
          const vector<FeatureMatch> &matches, MotionModel m, 
          int nRANSAC, double RANSACthresh, CTransform3x3& M)
{
    // BEGIN TODO
    // Write this entire method.  You need to handle two types of 
    // motion models, pure translations (m == eTranslation) and 
    // full homographies (m == eHomography).  However, you should
    // only have one outer loop to perform the RANSAC code, as 
    // the use of RANSAC is almost identical for both cases.
    //
    // Your homography handling code should call ComputeHomography.
    // This function should also call countInliers and, at the end,
    // leastSquaresFit.
printf("TODO: %s:%d\n", __FILE__, __LINE__); 

    // END TODO

    return 0;
}

/******************* TO DO *********************
 * countInliers:
 *	INPUT:
 *		f1, f2: source feature sets
 *		matches: correspondences between f1 and f2
 *               Each match in 'matches' contains two feature ids of 
 *               matching features, id1 (in f1) and id2 (in f2).
 *		m: motion model
 *		M: transformation matrix
 *		RANSACthresh: RANSAC distance threshold
 *		inliers: inlier feature IDs
 *	OUTPUT:
 *		transform the features in f1 by M
 *
 *		count the number of features in f1 for which the transformed
 *		feature is within Euclidean distance RANSACthresh of its match
 *		in f2
 *
 *		store these features IDs in inliers
 *
 */
int countInliers(const FeatureSet &f1, const FeatureSet &f2,
                 const vector<FeatureMatch> &matches, MotionModel m,
                 CTransform3x3 M, double RANSACthresh, vector<int> &inliers)
{
    inliers.clear();

    for (unsigned int i = 0; i < matches.size(); i++) {
        // BEGIN TODO
        // determine if the ith matched feature f1[id1], when transformed by M,
        // is within RANSACthresh of its match in f2
        //
        // if so, append i to inliers
printf("TODO: %s:%d\n", __FILE__, __LINE__); 

        // END TODO
    }

    return (int) inliers.size();
}

/******************* TO DO *********************
 * leastSquaresFit:
 *	INPUT:
 *		f1, f2: source feature sets
 *		matches: correspondences between f1 and f2
 *		m: motion model
 *      inliers: inlier match indices (indexes into 'matches' array)
 *		M: transformation matrix (output)
 *	OUTPUT:
 *		compute the transformation from f1 to f2 using only the inliers
 *		and return it in M
 */
int leastSquaresFit(const FeatureSet &f1, const FeatureSet &f2,
            const vector<FeatureMatch> &matches, MotionModel m, 
            const vector<int> &inliers, CTransform3x3& M)
{
    // This function needs to handle two possible motion models, 
    // pure translations and full homographies.

    switch (m) {
        case eTranslate: {
            // for spherically warped images, the transformation is a 
            // translation and only has two degrees of freedom
            //
            // therefore, we simply compute the average translation vector
            // between the feature in f1 and its match in f2 for all inliers
            double u = 0;
            double v = 0;

            for (int i=0; i < (int) inliers.size(); i++) {
			    // BEGIN TODO
			    // use this loop to compute the average translation vector
			    // over all inliers
printf("TODO: %s:%d\n", __FILE__, __LINE__); 

                // END TODO
            }

            u /= inliers.size();
            v /= inliers.size();

            M = CTransform3x3::Translation((float) u, (float) v);

            break;
        } 

        case eHomography: {
			M = CTransform3x3();

            // BEGIN TODO
		    // Compute a homography M using all inliers.
		    // This should call ComputeHomography.
printf("TODO: %s:%d\n", __FILE__, __LINE__); 

            // END TODO
        
            break;
        }
    }    

    return 0;
}

