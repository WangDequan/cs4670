///////////////////////////////////////////////////////////////////////////
//
// NAME
//  Transform.h -- coordinate transformation matrices
//
// DESCRIPTION
//  These classes are used to implement 3x3 and 4x4 coordinate transformation
//  matrices.  Basic operations supported are:
//   . multiplication
//   . inverses
//   . translation and rotation
//
// SEE ALSO
//  Transform.cpp       implementation
//
// Copyright © Richard Szeliski, 2001.  See Copyright.h for more details
//
///////////////////////////////////////////////////////////////////////////

#ifndef __transform_h__
#define __transform_h__

class CTransform3x3
{
public:
    CTransform3x3();                    // default constructor (identity)
    static CTransform3x3 Translation(float tx, float ty);
    static CTransform3x3 Rotation(float degrees);
    CTransform3x3 Inverse(void  );      // matrix inverse
    CTransform3x3 operator*(const CTransform3x3& m);
	double Determinant();
    double* operator[](int i);          // access the elements
    const double* operator[](int i) const;    // access the elements
private:
    double m_array[3][3];               // data array
};

inline double* CTransform3x3::operator[](int i)
{
    return m_array[i];
}

inline const double* CTransform3x3::operator[](int i) const
{
    return m_array[i];
}

#endif /* __transform_h__ */
