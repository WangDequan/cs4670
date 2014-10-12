///////////////////////////////////////////////////////////////////////////
//
// NAME
//  Convolve.h -- separable and non-separable linear convolution
//
// SPECIFICATION
//  void Convolve(CImageOf<T> src, CImageOf<T>& dst,
//                CFloatImage kernel);
//
//  void ConvolveSeparable(CImageOf<T> src, CImageOf<T>& dst,
//                         CFloatImage xKernel, CFloatImage yKernel,
//                         int subsample);
//
// PARAMETERS
//  src                 source image
//  dst                 destination image
//  kernel              2-D convolution kernel
//  xKernel, yKernel    1-D convolution kernels (1-row images)
//  subsample           subsampling/decimation factor (1 = none, 2 = half, ...)
//
// DESCRIPTION
//  Perform a 2D or separable 1D convolution.  The convolution kernels
//  are supplied as 2D floating point images  (1 row images for the separable
//  kernels).  The position of the "0" pixel in the kernel is determined
//  by the kernel.origin[] parameters, which specify the offset (coordinate,
//  usually negative) of the first (top-left) pixel in the kernel.
//
// SEE ALSO
//  Convolve.cpp        implementation
//  Image.h             image class definition
//
// Copyright © Richard Szeliski, 2001.  See Copyright.h for more details
//
///////////////////////////////////////////////////////////////////////////

template <class T>
void Convolve(CImageOf<T> src, CImageOf<T>& dst,
              CFloatImage kernel);

template <class T>
void ConvolveSeparable(CImageOf<T> src, CImageOf<T>& dst,
                       CFloatImage xKernel, CFloatImage yKernel,
                       int subsample);

template <class T>
void ConvolveRow(CImageOf<T> buffer, CFloatImage kernel, T* dst,
                 int n, T minVal, T maxVal)
{
    CShape kShape = kernel.Shape();
    int kX  = kShape.width;
    int kY  = kShape.height;
    CShape bShape = buffer.Shape();
    int nB  = bShape.nBands;

    for (int i = 0; i < n; i++)
    {
        for (int b = 0; b < nB; b++)
        {
            float sum = 0.0f;
            for (int k = 0; k < kY; k++)
            {
                float* kPtr = &kernel.Pixel(0, k, 0);
                T*     bPtr = &buffer.Pixel(i, k, b);
                for (int l = 0; l < kX; l++, bPtr += nB)
                    sum += kPtr[l] * bPtr[0];
            }
            *dst++ = (T) __max(minVal, __min(maxVal, sum));
        }
    }
}

template <class T>
void Convolve(CImageOf<T> src, CImageOf<T>& dst,
              CFloatImage kernel)
{
    // Determine the shape of the kernel and source image
    CShape kShape = kernel.Shape();
    CShape sShape = src.Shape();

    // Allocate the result, if necessary
    dst.ReAllocate(sShape, false);
    if (sShape.width * sShape.height * sShape.nBands == 0)
        return;

    // Do the convolution
    for (int y = 0; y < sShape.height; y++) {
        for (int x = 0; x < sShape.width; x++) {
            for (int c = 0; c < sShape.nBands; c++) {
                double sum = 0;
                for (int kx = 0; kx < kShape.width; kx++)
                    for (int ky = 0; ky < kShape.height; ky++)
                        if ((x-kernel.origin[0]+kx >= 0) && (x-kernel.origin[0]+kx < sShape.width) && (y-kernel.origin[1]+ky >= 0) && (y-kernel.origin[1]+ky < sShape.height))
                            sum += kernel.Pixel(kx,ky,0) * src.Pixel(x-kernel.origin[0]+kx,y-kernel.origin[1]+ky,c);
                dst.Pixel(x,y,c) = (T) __max(dst.MinVal(), __min(dst.MaxVal(), sum));
            }
        }
    }
}

template <class T>
void ConvolveSeparable(CImageOf<T> src, CImageOf<T>& dst,
                       CFloatImage x_kernel, CFloatImage y_kernel,
                       int subsample)
{
    // Allocate the result, if necessary
    CShape dShape = src.Shape();
    if (subsample > 1)
    {
        dShape.width  = (dShape.width  + subsample-1) / subsample;
        dShape.height = (dShape.height + subsample-1) / subsample;
    }
    dst.ReAllocate(dShape, false);

    // Allocate the intermediate images
    CImageOf<T> tmpImg1(src.Shape());
    CImageOf<T> tmpImg2(src.Shape());

    // Create a proper vertical convolution kernel
    CFloatImage v_kernel(1, y_kernel.Shape().width, 1);
    for (int k = 0; k < y_kernel.Shape().width; k++)
        v_kernel.Pixel(0, k, 0) = y_kernel.Pixel(k, 0, 0);
    v_kernel.origin[1] = y_kernel.origin[0];

    // Perform the two convolutions
    Convolve(src, tmpImg1, x_kernel);
    Convolve(tmpImg1, tmpImg2, v_kernel);
				
    // Downsample or copy
    for (int y = 0; y < dShape.height; y++)
    {
        T* sPtr = &tmpImg2.Pixel(0, y * subsample, 0);
        T* dPtr = &dst.Pixel(0, y, 0);
        int nB  = dShape.nBands;
        for (int x = 0; x < dShape.width; x++)
        {
            for (int b = 0; b < nB; b++)
                dPtr[b] = sPtr[b];
            sPtr += subsample * nB;
            dPtr += nB;
        }
    }
}

extern CFloatImage ConvolveKernel_121;
extern CFloatImage ConvolveKernel_14641;
extern CFloatImage ConvolveKernel_7x7;
extern CFloatImage ConvolveKernel_8tapLowPass;
extern CFloatImage ConvolveKernel_SobelX;
extern CFloatImage ConvolveKernel_SobelY;
