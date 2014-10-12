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
#include <random>

/**
 * ComputeHomography:
 *     INPUT:
 *          f1, f2: source feature sets
 *         matches: correspondences between f1 and f2
 *                  Each match in 'matches' contains two feature ids of 
 *                  matching features, id1 (in f1) and id2 (in f2).
 *
 *     OUTPUT:
 *         estimates a homography from the given matches and returns
 *         the homography as a CTransform3x3
 */
CTransform3x3 ComputeHomography(const FeatureSet &f1, const FeatureSet &f2,
                                const vector<FeatureMatch> &matches)
{
    // Get number of matches
    int numMatches = (int) matches.size();

    // first, we will compute the A matrix in the homogeneous linear equations Ah = 0
    int numRows = 2 * numMatches; // number of rows of A
    const int numCols = 9;        // number of columns of A

    // This allocates space for the A matrix
    AMatrixType A = AMatrixType::Zero(numRows, numCols);

    for (int i = 0; i < numMatches; i++) {
        const FeatureMatch &m = matches[i];
        const Feature &a = f1[m.id1];
        const Feature &b = f2[m.id2];

        // Fill in the matrix A in this loop.
        //
        //     +----------------------------------+
        //     | x  y  1  0  0  0  -xx'  -yx' -x' |
        // A = | 0  0  0  x  y  1  -xy'  -yy' -y' |
        //     |                :                 |
        //     +----------------------------------+
        //	
        A(2*i,0) = a.x;
        A(2*i,1) = a.y;
        A(2*i,2) = 1;
        A(2*i,3) = 0;
        A(2*i,4) = 0;
        A(2*i,5) = 0;
        A(2*i,6) = -a.x * b.x;
        A(2*i,7) = -a.y * b.x;
        A(2*i,8) = -b.x;
        A(2*i+1,0) = 0;
        A(2*i+1,1) = 0;
        A(2*i+1,2) = 0;
        A(2*i+1,3) = a.x;
        A(2*i+1,4) = a.y;
        A(2*i+1,5) = 1;
        A(2*i+1,6) = -a.x * b.y;
        A(2*i+1,7) = -a.y * b.y;
        A(2*i+1,8) = -b.y;
    }

    // Compute the SVD of the A matrix and get out the matrix V^T and the vector of singular values
    AMatrixType Vt;
    VectorXd sv;
    SVD(A, Vt, sv);

    // Initialize variables
    CTransform3x3 H;		  // a CTransform3x3 to contain homography
    int min_index;			  // index corresponding to lowest singular value in sv
    double min_value = 1E+37; // lowest singular value in sv

    // Find index that corresponds to the lowest singular value in sv 
    for (int i = 0; i < sv.size(); i++) {
        if (sv(i) < min_value) {
            min_index = i;
            min_value = sv(i);
        }
    }

    // Fill in homography H with values from row of V matrix corresponding to 
    // lowest singular value (since sv is sorted we use the last row)
    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 3; i++) {
            H[j][i] = Vt(min_index, 3*j+i);
        }
    }

    return H;
}


/**
 * alignPair:
 *    INPUT:
 *          f1, f2: source feature sets
 *         matches: correspondences between f1 and f2
 *                  Each match in 'matches' contains two feature ids of 
 *                  matching features, id1 (in f1) and id2 (in f2).
 *               m: motion model
 *         nRANSAC: number of RANSAC iterations
 *    RANSACthresh: RANSAC distance threshold
 *               M: transformation matrix (output)
 *
 *    OUTPUT:
 *        repeat for nRANSAC iterations:
 *            choose a minimal set of feature matches
 *            estimate the transformation implied by these matches
 *            count the number of inliers
 *        for the transformation with the maximum number of inliers,
 *        compute the least squares motion estimate using the inliers,
 *        and store it in M
 */
int alignPair(const FeatureSet &f1, const FeatureSet &f2,
          const vector<FeatureMatch> &matches, MotionModel m, 
          int nRANSAC, double RANSACthresh, CTransform3x3& M)
{
    // Initialize variables
    vector<int> bestInliers;  // vector of inliers
    CTransform3x3 transform;  // 3x3 transform matrix

    // Initialize uniform random distribution [0-<size of matches>-1]
    default_random_engine generator;
    uniform_int_distribution<int> distribution(0, matches.size()-1);

    // Repeat for nRANSAC iterations
    for (int i=0; i < nRANSAC; i++) {
        switch (m) {
            case eTranslate: {
                //====================================
                // TRANSLATION CASE
                //====================================
                // For spherically warped images, the transformation is a 
                // simple translation
                //
                // As a result, we randomly sample a set of matching features
                // and find the translation relating the feature in f1 to
                // its match in f2

                // Get random set of matching features
                int index = distribution(generator);
                const FeatureMatch &fm = matches[index];
                const Feature &a = f1[fm.id1];
                const Feature &b = f2[fm.id2];

                // Calculate translation vectors  
                double u = b.x - a.x;
                double v = b.y - a.y;

                // Build transformation matrix from translation vectors
                transform = CTransform3x3::Translation((float) u, (float) v);
                break;
            }
            case eHomography: {
                //====================================
                // HOMOGRAPHY CASE
                //====================================
                // For unwarped images, the transformation is a homography with
                // 8 degrees of freedom. As a result, 4 samples of matching 
                // features are required
                //
                // We randomly sample 4 different feature matches and use them
                // to compute a best fit homography

                // Initialize variables
                vector<int> indexes;				 // vector of chosen indexes to avoid repeats
                vector<FeatureMatch> matches_sample; // vector of FeatureMatches for computing homography
                int randIndex;						 // selection index

                // Get random set of 4 matching features
                while ((int)indexes.size() < min(4, (int)matches.size())) {
                    randIndex = distribution(generator);
                    if (find(indexes.begin(), indexes.end(), randIndex) == indexes.end()) {
                        matches_sample.push_back(matches[randIndex]);
                        indexes.push_back(randIndex);
                    }
                }

                // Compute homography from random set of matching features
                transform = ComputeHomography(f1, f2, matches_sample);
                break;
            }
        }

        // Count the number inliers for the transformation matrix transform
        vector<int> inliers;
        int numInliers = countInliers(f1, f2, matches, m, transform, RANSACthresh, inliers);

        // If the number of inliers is greater than the max recorded number of inliers
        // save both the number of inliers and the vector inliers
        if (numInliers > (int) bestInliers.size()) {
            bestInliers = inliers;
        }
    }

    // Compute the transformation matrix using the best set of inliers
    leastSquaresFit(f1, f2, matches, m, bestInliers, M);

	// Output results
    fprintf(stderr, "num_inliers: %d / %d\n", (int) bestInliers.size(), (int) matches.size()); 

    return 0;
}

/**
 * countInliers:
 *    INPUT:
 *         f1, f2: source feature sets
 *        matches: correspondences between f1 and f2
 *                 Each match in 'matches' contains two feature ids of 
 *                 matching features, id1 (in f1) and id2 (in f2).
 *              m: motion model
 *              M: transformation matrix
 *   RANSACthresh: RANSAC distance threshold
 *        inliers: inlier feature IDs
 *
 *    OUTPUT:
 *        transform the features in f1 by M
 *
 *        count the number of features in f1 for which the transformed
 *        feature is within Euclidean distance RANSACthresh of its match
 *        in f2
 *
 *        store these features IDs in inliers
 */
int countInliers(const FeatureSet &f1, const FeatureSet &f2,
                 const vector<FeatureMatch> &matches, MotionModel m, 
                 CTransform3x3 M, double RANSACthresh, vector<int> &inliers)
{
    inliers.clear();

    for (unsigned int i = 0; i < matches.size(); i++) {
        // Determine if the ith matched feature f1[id1], when transformed by M,
        // is within RANSACthresh of its match in f2
        //
        // If so, append i to inliers
		
        // Get set of matching features
        const FeatureMatch &fm = matches[i];
        const Feature &a = f1[fm.id1];
        const Feature &b = f2[fm.id2];

        // Initialize vectors
        CVector3 p;
        CVector3 p2;

        // Feature vector for f1
        p[0] = a.x;
        p[1] = a.y;
        p[2] = 1;

        // Transform feature vector by M
        p2 = M*p;

        // Normalize vector for transformed feature
        p2[0] /= p2[2];
        p2[1] /= p2[2];
        p2[2] /= p2[2];

        // Get euclidean distance between transformed feature f1 and its match
        double distance = sqrt(pow(p2[0]-b.x,2)+pow(p2[1]-b.y,2));

        // If distance is less than or equal to RANSACthresh, append index to inliers
        if (distance <= RANSACthresh) {
            inliers.push_back(i);
        }
    }

    return (int) inliers.size();
}

/**
 * leastSquaresFit:
 *    INPUT:
 *         f1, f2: source feature sets
 *        matches: correspondences between f1 and f2
 *                 Each match in 'matches' contains two feature ids of 
 *                 matching features, id1 (in f1) and id2 (in f2).
 *              m: motion model
 *        inliers: inlier match indices (indexes into 'matches' array)
 *              M: transformation matrix (output)
 *	OUTPUT:
 *        compute the transformation from f1 to f2 using only the inliers
 *        and return it in M
 */
int leastSquaresFit(const FeatureSet &f1, const FeatureSet &f2,
            const vector<FeatureMatch> &matches, MotionModel m, 
            const vector<int> &inliers, CTransform3x3& M)
{
    // This function needs to handle two possible motion models, 
    // pure translations and full homographies.

    switch (m) {
        case eTranslate: {
            //====================================
            // TRANSLATION CASE
            //====================================
            // For spherically warped images, the transformation is a 
            // translation and only has two degrees of freedom
            //
            // Therefore, we simply compute the average translation vector
            // between the feature in f1 and its match in f2 for all inliers
            
            // Initialize variables
            double u = 0; // xoffset
            double v = 0; // yoffset
            int index;    // inlier index

            // Compute the average translation vector over all inliers 
            for (int i=0; i < (int) inliers.size(); i++) {
                // Get inlier index from inliers
                index = inliers[i];

                // Get set of matching features
                const FeatureMatch &fm = matches[index];
                const Feature &a = f1[fm.id1];
                const Feature &b = f2[fm.id2];

                // Accumulate offsets between f1 and f2
                u += b.x - a.x;
                v += b.y - a.y;
            }

            // Divide by size of inliers to compute average translation
            u /= inliers.size();
            v /= inliers.size();

            // Build transformation matrix from average translation vectors
            M = CTransform3x3::Translation((float) u, (float) v);
            break;
        } 

        case eHomography: {
            //====================================
            // HOMOGRAPHY CASE
            //====================================
            // For unwarped images, we compute the best fit homogaphy using 
            // only inlier feature matches 
			
            // Initialize variables
            M = CTransform3x3();				 // homography matrix
            vector<FeatureMatch> inlier_matches; // vector of inlier feature matches 

            // Populate inlier_matches with inlier feature matches
            int index;
            for (int i=0; i < (int) inliers.size(); i++) {
                index = inliers[i];	
                inlier_matches.push_back(matches[index]);
            }

            // Compute the homography using only inlier feature matches
            M = ComputeHomography(f1, f2, inlier_matches);
            break;
        }
    }    

    return 0;
}

