///////////////////////////////////////////////////////////////////////////
//
// NAME
//  Image.inl -- a simple reference-counted image structure
//
// DESCRIPTION
//  The incline function definitions to go with Image.h
//
// SEE ALSO
//  Image.h             more complete description
//
// Copyright © Richard Szeliski, 2001.  See Copyright.h for more details
//
///////////////////////////////////////////////////////////////////////////

//
// struct CShape: shape of image (width x height x nbands)
//

inline bool CShape::InBounds(int x, int y)
{
    // Is given pixel address valid?
    return (0 <= x && x < width &&
            0 <= y && y < height);
}

inline bool CShape::InBounds(int x, int y, int b)
{
    // Is given pixel address valid?
    return (0 <= x && x < width &&
            0 <= y && y < height &&
            0 <= b && b < nBands);
}

//
// class CImage : generic (weakly typed) image
//

inline 
void* 
CImage::PixelAddress(int x, int y, int band)
{
    // This could also go into the implementation file (CImage.cpp):
    return (void *) &m_memStart[y * m_rowSize + x * m_pixSize + band * m_bandSize];
}

inline 
const void* 
CImage::PixelAddress(int x, int y, int band) const
{
    // This could also go into the implementation file (CImage.cpp):
    return (void *) &m_memStart[y * m_rowSize + x * m_pixSize + band * m_bandSize];
}

//  Strongly typed image;
//    these are defined inline so user-defined image types can be supported:

template <class T>
inline CImageOf<T>::CImageOf(void) :
CImage(CShape(), typeid(T), sizeof(T)) {}

template <class T>
inline CImageOf<T>::CImageOf(CShape s) :
CImage(s, typeid(T), sizeof(T)) {}

template <class T>
inline CImageOf<T>::CImageOf(int width, int height, int nBands) :
CImage(CShape(width, height, nBands), typeid(T), sizeof(T)) {}

template <class T>
inline void CImageOf<T>::ReAllocate(CShape s, bool evenIfShapeDiffers)
{
    CImage::ReAllocate(s, typeid(T), sizeof(T), evenIfShapeDiffers);
}

template <class T>
inline void CImageOf<T>::ReAllocate(CShape s, T *memory,
                                    bool deleteWhenDone, int rowSize)
{
    CImage::ReAllocate(s, typeid(T), sizeof(T), memory, deleteWhenDone, rowSize);
}
    
template <class T>
inline 
T& 
CImageOf<T>::Pixel(int x, int y, int band)
{
    return *(T *) PixelAddress(x, y, band);
}

template <class T>
inline 
const T& 
CImageOf<T>::Pixel(int x, int y, int band) const 
{
    return *(T *) PixelAddress(x, y, band);
}

template <class T>
inline 
CImageOf<T> 
CImageOf<T>::SubImage(int x, int y, int width, int height)
{
    // sub-image sharing memory
    CImageOf<T> retval = *this;
    retval.SetSubImage(x, y, width, height);
    return retval;
}

template <class T>
inline
void 
CImageOf<T>::getRangeOfValues(T& minVal, T& maxVal) const
{
    minVal = MaxVal();
    maxVal = MinVal();

    CShape shape = this->Shape();
    for(int y = 0; y < shape.height; y++) {
        T* thisIt = (T*)this->PixelAddress(0, y, 0);
        for(int x = 0; x < shape.width * shape.nBands; x++, thisIt++) {
            minVal = min(minVal, *thisIt);
            maxVal = max(maxVal, *thisIt);
        }
    }    
}

template <class T>
inline
CImageOf<T>& 
CImageOf<T>::operator-=(const T& scalar)
{
    *this += -scalar;
    return *this;
}

template <class T>
inline
CImageOf<T>& 
CImageOf<T>::operator+=(const T& scalar)
{
    CShape shape = this->Shape();

    for(int y = 0; y < shape.height; y++) {
        T* thisIt = (T*)this->PixelAddress(0, y, 0);
        for(int x = 0; x < shape.width * shape.nBands; x++, thisIt++) {
            *thisIt += scalar;
        }
    }
    return *this;
}

template <class T>
inline
CImageOf<T>& 
CImageOf<T>::operator+=(const CImageOf<T>& other)
{
    if(this->Shape() != other.Shape()) throw CError("Images must have the same shape for operator+=");

    CShape shape = this->Shape();

    for(int y = 0; y < shape.height; y++) {
        const T* otherIt = (const T*)other.PixelAddress(0, y, 0);
        T* thisIt = (T*)this->PixelAddress(0, y, 0);
        for(int x = 0; x < shape.width * shape.nBands; x++, thisIt++, otherIt++) {
            *thisIt += *otherIt;
        }
    }
    return *this;
}

template <class T>
inline
CImageOf<T>& 
CImageOf<T>::operator/=(const CImageOf<T>& other)
{
    if(this->Shape() != other.Shape()) throw CError("Images must have the same shape for operator/=");

    CShape shape = this->Shape();

    for(int y = 0; y < shape.height; y++) {
        const T* otherIt = (const T*)other.PixelAddress(0, y, 0);
        T* thisIt = (T*)this->PixelAddress(0, y, 0);
        for(int x = 0; x < shape.width * shape.nBands; x++, thisIt++, otherIt++) {
            *thisIt /= *otherIt;
        }
    }
    return *this;
}

template <class T>
inline
CImageOf<T>& 
CImageOf<T>::operator/=(const T& scalar)
{
    CShape shape = this->Shape();

    for(int y = 0; y < shape.height; y++) {
        T* thisIt = (T*)this->PixelAddress(0, y, 0);
        for(int x = 0; x < shape.width * shape.nBands; x++, thisIt++) {
            *thisIt /= scalar;
        }
    }
    return *this;
}
