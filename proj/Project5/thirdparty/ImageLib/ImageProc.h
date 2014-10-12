#pragma once
#ifndef IMAGE_PROC_H
#define IMAGE_PROC_H

#include "Image.h"

//
// Type and band conversion routines
//

// When converting from float -> byte, multiplies by 255
//                      byte -> float, divides by 255
template <class T1, class T2>
void TypeConvert(const CImageOf<T1>& src, CImageOf<T2>& dst);

//extern template void TypeConvert<>(const CImageOf<unsigned char>& src, CImageOf<float>& dst);
//extern template void TypeConvert<>(const CImageOf<float>& src, CImageOf<unsigned char>& dst);
//extern template void TypeConvert<>(const CImageOf<unsigned char>& src, CImageOf<unsigned char>& dst);
//extern template void TypeConvert<>(const CImageOf<float>& src, CImageOf<float>& dst);

//
// Helper functions for point and neighborhood processing
//

void PointProcess1(CImage& img1, void* dataPtr,
                   bool (*fn)(int n, CImage **iptrs, void* dataPtr,
                              void* p1, int b1));

void PointProcess2(CImage& img1, CImage& img2, void* dataPtr,
                   bool (*fn)(int n, CImage **iptrs, void* dataPtr,
                              void* p1, int b1,
                              void* p2, int b2));

void NeighborhoodProcess(CImage& src, CImage& dst, void* dataPtr,
                         int halfWidth, int halfHeight,
                         bool reverseRaster,
                         bool (*fn)(int n, CImage **iptrs, void* dataPtr,
                                    void* p1, int b1,
                                    void* p2, int b2));

void NeighborhoodProcessSeparable(CImage& src, CImage& dst, void* dataPtr,
                               int halfWidth, int halfHeight,
                               bool reverseRaster,
                               bool (*f1)(int n, CImage **iptrs, void* dataPtr,
                                          void* p1, int b1,
                                          void* p2, int b2),
                               bool (*f2)(int n, CImage **iptrs, void* dataPtr,
                                          void* p1, int b1,
                                          void* p2, int b2));

//
// Miscellaneous utility routines
//

CImage Rotate90(CImage img, int nTimesCCW);


#endif


