///////////////////////////////////////////////////////////////////////////
//
// NAME
//  WarpSpherical.h -- warp a flat (perspective) image into spherical
//      coordinates and/or undo radial lens distortion
//
// SEE ALSO
//  WarpSpherical.h   longer description
//
//  R. Szeliski and H.-Y. Shum.
//  Creating full view panoramic image mosaics and texture-mapped models.
//  Computer Graphics (SIGGRAPH'97), pages 251-258, August 1997.
//
// Copyright ?Richard Szeliski, 2001.  See Copyright.h for more details
// (modified for CSE576 Spring 2005)
//
///////////////////////////////////////////////////////////////////////////

#include "ImageLib/ImageLib.h"
#include "WarpSpherical.h"
#include <math.h>

/**
 * warpSphericalField:
 *	INPUT:
 *		srcSh: shape (width, height, number of channels) of source image
 *		dstSh: shape of destination image
 *		f: focal length in pixels (provided on the project web page, or measured by yourself)
 *		k1, k2: radial distortion parameters (ditto)
 *		r: rotation matrix
 *	OUTPUT:
 *		Return an image containing (u,v) coordinates for mapping pixels from
 *		spherical coordinates to planar image coordinates and applying
 *		radial distortion.
 *			Note that this is inverse warping, i.e., this routine will be
 *		actually used to map from planar coordinates with radial distortion
 *		to spherical coordinates without radial distortion.
 *
 */
CFloatImage WarpSphericalField(CShape srcSh, CShape dstSh, float f,
	float k1, float k2, const CTransform3x3 &r)
{
    // Set up the pixel coordinate image
    dstSh.nBands = 2;
    CFloatImage uvImg(dstSh);   // (u,v) coordinates

    // Initialize unit vector
    CVector3 p;
    p[0] = sin(0.0) * cos(0.0);
    p[1] = sin(0.0);
    p[2] = cos(0.0) * cos(0.0);

    // Apply rotation 
    p = r * p;
 
    // Find corresponding minimum y value
    double min_y = p[1];

    // Fill in the values
    for (int y = 0; y < dstSh.height; y++) {
        float *uv = &uvImg.Pixel(0, y, 0);
        for (int x = 0; x < dstSh.width; x++, uv += 2) {
            // (x,y) is the spherical image coordinates. 
            // (xf,yf) is the spherical coordinates, e.g., xf is the angle theta
            // and yf is the angle phi

            float xf = (float) ((x - 0.5f*dstSh.width ) / f);
            float yf = (float) ((y - 0.5f*dstSh.height) / f - min_y);

            // (xt,yt,zt) are intermediate coordinates to which we
            // apply the spherical correction and radial distortion
			float xt, yt, zt;
            CVector3 p;

            // Convert to Euclidean coordinates
            xt = sin(xf) * cos(yf);
            yt = sin(yf);
            zt = cos(xf) * cos(yf);

            // Store Euclidean coordinates in vector
            p[0] = xt;
            p[1] = yt;
            p[2] = zt;

            // Apply rotation
            CVector3 rp = r*p;
			
            // Project to z=1 plane
            xt = rp[0] / rp[2];
            yt = rp[1] / rp[2];

            // Apply radial distortion
            float r2 = xt*xt + yt*yt;
            xt = xt * (1 + k1 * r2 + k2 * r2 * r2);
            yt = yt * (1 + k1 * r2 + k2 * r2 * r2);

            // Convert back to regular pixel coordinates and store
            float xn = 0.5f*srcSh.width  + xt*f;
            float yn = 0.5f*srcSh.height + yt*f;
            uv[0] = xn;
            uv[1] = yn;
        }
    }
    return uvImg;
}