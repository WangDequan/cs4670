///////////////////////////////////////////////////////////////////////////
//
// NAME
//  WarpImage.h -- warp an image either through a global parametric
//      transform or a local (per-pixel) transform
//
// SPECIFICATION
//  void WarpLocal(CImageOf<T> src, CImageOf<T>& dst,
//                 CFloatImage uv, bool relativeCoords,
//                 WarpInterpolationMode interp);
//
//  void WarpGlobal(CImageOf<T> src, CImageOf<T>& dst,
//                  CTransform3x3 M,
//                  WarpInterpolationMode interp);
//
// PARAMETERS
//  src                 source image
//  dst                 destination image
//  uv                  source pixel coordinates array/image
//  relativeCoords      source coordinates are relative (offsets = "flow")
//  interp              interpolation mode (nearest neighbor, bilinear, bicubic)
//  cubicA              parameter controlling cubic interpolation
//  M                   global 3x3 transformation matrix
//
// DESCRIPTION
//  WarpLocal preforms an inverse sampling of the source image into the
//  destination image.  In other words, for every pixel in dst, the pixel
//  in src at address uv is sampled (and interpolated, if required).
//
//  If any of the pixels involved in the interpolation are outside the
//  addressable region of src, the corresponding pixel in dst is set to all 0s.
//  Note that for cubic interpololation, this may result in significant loss
//  of pixels near the edges.  (Even for linear sampling with an integer
//  shift, the rightmost column will be lost.)
//
//  WarpGlobal performs a similar resampling, except that the transformation
//  is specified by a simple matrix that can be used to represent rigid,
//  affine, or perspective transforms.
//
//
// SEE ALSO
//  WarpImage.cpp       implementation
//  Image.h             image class definition
//
// Copyright © Richard Szeliski, 2001.  See Copyright.h for more details
//
///////////////////////////////////////////////////////////////////////////

enum EWarpInterpolationMode
{
    eWarpInterpNearest   = 0,    // nearest neighbor
    eWarpInterpLinear    = 1,    // bi-linear interpolation
    eWarpInterpCubic     = 3     // bi-cubic interpolation
};

template <class T>
void WarpLocal(CImageOf<T> src, CImageOf<T>& dst,
               CFloatImage uv, bool relativeCoords,
               EWarpInterpolationMode interp, float cubicA = 1.0);

template <class T>
void WarpGlobal(CImageOf<T> src, CImageOf<T>& dst,
                CTransform3x3 M,
                EWarpInterpolationMode interp, float cubicA = 1.0);



