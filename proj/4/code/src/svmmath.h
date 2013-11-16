#ifndef SVMMATH_H
#define SVMMATH_H

#include "svm.h"
#include "vec.h"
#include "mat.h"
#include <list>
#include <vector>

// Compute the best fit intersection of >2 lines
SVMPoint BestFitIntersect(const std::list<SVMLine> &lines, int imgWidth, int imgHeight);

void ComputeHomography(CTransform3x3 &H, CTransform3x3 &Hinv, 
					const std::vector<SVMPoint>& points,
					std::vector<Vec3d>& basisPts,
                    bool isRefPlane);

void ConvertToPlaneCoordinate(const std::vector<SVMPoint>& points, std::vector<Vec3d>& basisPts, double &uScale, double &vScale);

inline double Determinant(const double H[3][3])
{
	return 
		  H[0][0] * (H[1][1]*H[2][2] - H[1][2]*H[2][1])
		- H[0][1] * (H[1][0]*H[2][2] - H[1][2]*H[2][0])
		+ H[0][2] * (H[1][0]*H[2][1] - H[1][1]*H[2][0]);
}

// project a point p onto a line l
Vec3d ProjectToLine(Vec3d p, Vec3d l);


#endif
