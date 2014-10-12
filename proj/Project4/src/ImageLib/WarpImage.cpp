///////////////////////////////////////////////////////////////////////////
//
// NAME
//  WarpImage.cpp -- warp an image either through a global parametric
//      transform or a local (per-pixel) transform
//
// SEE ALSO
//  WarpImage.h         longer description of functions
//
// DESIGN
//  For cubic interpolation, we have a choice of several (variants of)
//  interpolation functions.  In general, we can use any family
//  of 4-tap filters.  In particular, we will use piecewise-cubic
//  functions that are interpolating and constant and linear preserving.
//
//  We furthermore make the restriction that the interpolant is C1
//  (not all interpolants do this).  This leaves us with only one
//  degree of freedom:  the slope at (x=1), which we call "a".
//
//  For the implementation, we form a LUT for the cubic interpolation
//  function, and initialize it at the beginning of each image warp
//  (in case "a" has changed).
//
// Copyright ?Richard Szeliski, 2001.  See Copyright.h for more details
//
///////////////////////////////////////////////////////////////////////////

#include "Image.h"
#include "Transform.h"
#include "WarpImage.h"
#include <math.h>
#include <vector>

//
//  Bilinear interpolation: cascade horizontal and vertical resamplings
//

static inline float ResampleLinear(float v0, float v1, float f)
{
    return v0 + f * (v1 - v0);
}

template <class T>
static inline T ResampleBiLinear(T src[], int oH, int oV, float xf, float yf)
{
    // Resample a pixel using bilinear interpolation
    float h1 = ResampleLinear(src[0 ], src[   oH], xf);
    float h2 = ResampleLinear(src[oV], src[oV+oH], xf);
    float  v = ResampleLinear(h1, h2, yf);
    return (T) v;
}

//
//  Bicubic interpolation: cascade horizontal and vertical resamplings
//

static const int cubicLUTsize = 256;
static float cubicInterp[cubicLUTsize][4];

static void InitializeCubicLUT(float a)
{
    float zero = 0.0;      // not implemented yet
    float error = 1.0f / zero;
}

static inline float ResampleCubic(float v0, float v1, float v2, float v3, float f)
{
    int fi = int(f*cubicLUTsize);
    float *c = cubicInterp[fi];
    float v = c[0]*v0 + c[1]*v1 + c[2]*v2 + c[3]*v3;
    return v;
}


///////////////////////////////////////////////////////////////////////////
// TEMPLATE IMPLEMENTATIONS
///////////////////////////////////////////////////////////////////////////

template <class T>
static T ResampleBiCubic(T src[], int oH, int oV, float xf, float yf)
{
    // Resample a pixel using bilinear interpolation
    float h[4];
    for (int i = 0; i < 4; i++)
    {
        int j = (i-1)*oV;
        h[i] = ResampleCubic(src[j-oH], src[j], src[j+oH], src[j+2*oH], xf);
    }
    float  v = ResampleCubic(h[0], h[1], h[2], h[3], yf);
    return (T) v;
}

//
//  Resample a complete line, given the source pixel addresses
//

template <class T>
void WarpLine(CImageOf<T> src, T* dstP, float *xyP, int n, int nBands,
              EWarpInterpolationMode interp, T minVal, T maxVal)
{
    // Determine the interpolator's "footprint"
    const int o0 = int(interp)/2;       // negative extent
    const int o1 = int(interp) - o0;    // positive extent
    const int oH = nBands;              // horizonal offset between pixels
    const int oV = &src.Pixel(0, 1, 0) -
                   &src.Pixel(0, 0, 0); // vertical  offset between pixels
    CShape sh = src.Shape();

    // Resample a single output scanline
    for (int i = 0; i < n; i++, dstP += nBands, xyP += 2)
    {
        if (nBands == 4 && dstP[3] == 0)
            continue; // don't fill in if alpha = 0

        // Round down pixel coordinates
        int x = int(floor(xyP[0]));
        int y = int(floor(xyP[1]));

        // Check if all participating pixels are in bounds
        if (! (sh.InBounds(x-o0, y-o0) && sh.InBounds(x+o1, y+o1)))
        {
            for (int j = 0; j < nBands; j++)
                dstP[j] = 0;
            continue;
        }
        T* srcP = &src.Pixel(x, y, 0);

        // Nearest-neighbor: just copy pixels
        if (interp == eWarpInterpNearest)
        {
            for (int j = 0; j < nBands; j++)
                dstP[j] = srcP[j];
            continue;
        }

        float xf = xyP[0] - x;
        float yf = xyP[1] - y;

        // Bilinear and bi-cubic
        if (interp == eWarpInterpLinear)
        {
            for (int j = 0; j < nBands; j++)
                dstP[j] = __max(minVal, __min(maxVal,
                    ResampleBiLinear(&srcP[j], oH, oV, xf, yf)));
        }
        if (interp == eWarpInterpCubic)
        {
            for (int j = 0; j < nBands; j++)
                dstP[j] = __max(minVal, __min(maxVal,
                    ResampleBiCubic(&srcP[j], oH, oV, xf, yf)));
        }
    }
}


//
//  Resample a complete image, given source pixel addresses
//

template <class T>
void WarpLocal(CImageOf<T> src, CImageOf<T>& dst,
               CFloatImage uv, bool relativeCoords,
               EWarpInterpolationMode interp, float cubicA)
{
    // Check that dst is of the right shape
    CShape sh(uv.Shape().width, uv.Shape().height, src.Shape().nBands);
    dst.ReAllocate(sh);

    // Allocate a row buffer for coordinates
    int n = sh.width;
    std::vector<float> rowBuf;
    rowBuf.resize(n*2);

    // Precompute the cubic interpolant
    if (interp == eWarpInterpCubic)
        InitializeCubicLUT(cubicA);

    // Process each row
    for (int y = 0; y < sh.height; y++)
    {
        float *uvP  = &uv .Pixel(0, y, 0);
        float *xyP  = (relativeCoords) ? &rowBuf[0] : uvP;
        T *dstP     = &dst.Pixel(0, y, 0);

        // Convert to absolute coordinates if necessary
        if (relativeCoords)
        {
            for (int x = 0; x < n; x++)
            {
                xyP[2*x+0] = x + uvP[2*x+0];
                xyP[2*x+1] = y + uvP[2*x+1];
            }
        }

        // Resample the line
        WarpLine(src, dstP, xyP, n, sh.nBands, interp, src.MinVal(), src.MaxVal());
    }
}

//
//  Resample a complete image, given dst->src pixel transformation
//

template <class T>
void WarpGlobal(CImageOf<T> src, CImageOf<T>& dst,
                CTransform3x3 M,
                EWarpInterpolationMode interp, float cubicA)

{
    // Not implemented yet, since haven't decided on semantics of M yet...

    // Check that dst is of a valid shape
    if (dst.Shape().width == 0)
        dst.ReAllocate(src.Shape());
    CShape sh = dst.Shape();

    // Allocate a row buffer for coordinates
    int n = sh.width;
    std::vector<float> rowBuf;
    rowBuf.resize(n*2);

    // Precompute the cubic interpolant
    if (interp == eWarpInterpCubic)
        InitializeCubicLUT(cubicA);

    // Process each row
    for (int y = 0; y < sh.height; y++)
    {
        float *xyP  = &rowBuf[0];
        T *dstP     = &dst.Pixel(0, y, 0);

        // Compute pixel coordinates
        float X0 = (float) (M[0][1]*y + M[0][2]);
        float dX = (float) (M[0][0]);
        float Y0 = (float) (M[1][1]*y + M[1][2]);
        float dY = (float) (M[1][0]);
        float Z0 = (float) (M[2][1]*y + M[2][2]);
        float dZ = (float) (M[2][0]);
        bool affine = (dZ == 0.0);
        float Zi = 1.0f / Z0;           // TODO:  doesn't guard against divide by 0
        if (affine)
        {
            X0 *= Zi, dX *= Zi, Y0 *= Zi, dY *= Zi;
        }
        for (int x = 0; x < n; x++)
        {
            xyP[2*x+0] = X0 * Zi;
            xyP[2*x+1] = Y0 * Zi;
            X0 += dX;
            Y0 += dY;
            if (! affine)
            {
                Z0 += dZ;
                Zi = 1.0f / Z0;
            }
        }

        // Resample the line
        WarpLine(src, dstP, xyP, n, sh.nBands, interp, src.MinVal(), src.MaxVal());
    }
}

// Instantiate the code
void WarpInstantiate(void)
{
    CByteImage i1;
    CFloatImage uv1;
    WarpLocal(i1, i1, uv1, false, eWarpInterpLinear, 1.0f);
    CTransform3x3 M;
    WarpGlobal(i1, i1, M, eWarpInterpLinear, 1.0f);
}

