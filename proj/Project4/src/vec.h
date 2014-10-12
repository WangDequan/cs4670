#ifndef __VECTOR_HEADER__
#define __VECTOR_HEADER__

// Stupid FLTK includes istd::ostream.h, so I can't include the official 
// STL version of istd::ostream.    -- ehsu
#include <math.h>
#include "mat.h"

#include <iostream>

#pragma warning(push)
#pragma warning(disable : 4244)

//==========[ Forward References ]=========================

template <class T> class Vec;
template <class T> class Vec3;
template <class T> class Vec4;
//template <class T> class Mat3;
//template <class T> class Mat4;

//==========[ Exception Classes ]==========================

class VectorSizeMismatch {};

//==========[ class Vec ]==================================

template <class T>
class Vec {
    // array of elements
    T*		n;
    // vector size
    int		numElements;

public:

    //---[ Constructors/Destructor ]-------------

    Vec() { n = NULL; numElements = 0; }
    // creates a new vector with size elements
    // if zeroElements is true the vector is initialized to zero
    Vec( int size, bool zeroElements = false );
    // copy constructor
    Vec( const Vec<T>& v );
    // destructor, simply deletes the array
    virtual ~Vec();

    //---[ Size Methods ]------------------------

    int size() const { return numElements; }
    void resize( int size, bool zeroElements = false );

    //---[ Equal Operators ]---------------------

    Vec<T>& operator =  ( const Vec<T>& v );
    Vec<T>& operator += ( const Vec<T>& v );
    Vec<T>& operator -= ( const Vec<T>& v );
    Vec<T>& operator *= ( const T d );
    Vec<T>& operator /= ( const T d );

    //---[ Access Operators ]--------------------

    T& operator []( int i )
    { return n[i]; }
    T operator []( int i ) const 
    { return n[i]; }

    //---[ Arithmetic Operators ]----------------

    Vec<T> operator-( const Vec<T>& a );
    Vec<T> operator+( const Vec<T>& a );

    //---[ Conversion Operators ]----------------

    T* getPointer() const { return n; }

    //---[ Length Methods ]----------------------

    double length2() const;
    double length() const;

    //---[ Normalization ]-----------------------

    void normalize();

    //---[ Zero Test ]---------------------------

    bool iszero();
    void zeroElements();

    //---[ Friend Methods ]----------------------

    friend T operator *( const Vec<T>& a, const Vec<T>& b );
    friend Vec<T> operator -( const Vec<T>& v );
    friend Vec<T> operator *( const Vec<T>& a, const double d );
    friend Vec<T> operator *( const double d, const Vec<T>& a );
    friend Vec<T> operator /( const Vec<T>& a, const double d );
    friend Vec<T> operator ^( const Vec<T>& a, const Vec<T>& b );
    friend bool operator ==( const Vec<T>& a, const Vec<T>& b );
    friend bool operator !=( const Vec<T>& a, const Vec<T>& b );
    friend Vec<T> minimum( const Vec<T>& a, const Vec<T>& b );
    friend Vec<T> maximum( const Vec<T>& a, const Vec<T>& b );
    friend Vec<T> prod( const Vec<T>& a, const Vec<T>& b );
};

typedef Vec<int>	Veci;
typedef Vec<float>	Vecf;
typedef Vec<double>	Vecd;

//==========[ class Vec3 ]=================================

template <class T>
class Vec3 {

    //---[ Private Variable Declarations ]-------

    // x, y, z
    T		n[3];

public:

    //---[ Constructors ]------------------------

    Vec3() { n[0] = 0.0; n[1] = 0.0; n[2] = 0.0; }
    Vec3( const T x, const T y, const T z )
    { n[0] = x; n[1] = y; n[2] = z; }
    Vec3( const Vec3<T>& v )
    { n[0] = v.n[0]; n[1] = v.n[1]; n[2] = v.n[2]; }

    //---[ Equal Operators ]---------------------

    Vec3<T>& operator=( const Vec3<T>& v )
    { n[0] = v.n[0]; n[1] = v.n[1]; n[2] = v.n[2]; return *this; }
    Vec3<T>& operator +=( const Vec3<T>& v )
    { n[0] += v.n[0]; n[1] += v.n[1]; n[2] += v.n[2]; return *this; }
    Vec3<T>& operator -= ( const Vec3<T>& v )
    { n[0] -= v.n[0]; n[1] -= v.n[1]; n[2] -= v.n[2]; return *this; }
    Vec3<T>& operator *= ( const T d )
    { n[0] *= d; n[1] *= d; n[2] *= d; return *this; }
    Vec3<T>& operator /= ( const T d )
    { n[0] /= d; n[1] /= d; n[2] /= d; return *this; }

    //---[ Access Operators ]--------------------

    T& operator []( int i )
    { return n[i]; }
    T operator []( int i ) const 
    { return n[i]; }

    //---[ Arithmetic Operators ]----------------

    Vec3<T> operator-( const Vec3<T>& a ) { return Vec3<T>(n[0]-a.n[0],n[1]-a.n[1],n[2]-a.n[2]); }
    Vec3<T> operator+( const Vec3<T>& a ) { return Vec3<T>(a.n[0]+n[0],a.n[1]+n[1],a.n[2]+n[2]); }

    //---[ Conversion Operators ]----------------

    const T* getPointer() const { return n; }

    //---[ Length Methods ]----------------------

    double length2() const
    { return n[0]*n[0] + n[1]*n[1] + n[2]*n[2]; }
    double length() const
    { return sqrt( length2() ); }

    //---[ Normalization ]-----------------------

    void normalize() { 
        double len = length();
        n[0] /= len; n[1] /= len; n[2] /= len;
    }

    //---[ Zero Test ]---------------------------

    bool iszero() { return ( (n[0]==0 && n[1]==0 && n[2]==0) ? true : false); };
    void zeroElements() { memset(n,0,sizeof(T)*3); }

    //---[ Friend Methods ]----------------------

    friend T operator *( const Vec3<T>& a, const Vec4<T>& b );
    friend T operator *( const Vec4<T>& b, const Vec3<T>& a );
    friend Vec3<T> operator -( const Vec3<T>& v );
    friend Vec3<T> operator *( const Vec3<T>& a, const double d );
    friend Vec3<T> operator *( const double d, const Vec3<T>& a );
    friend Vec3<T> operator *( const Vec3<T>& v, Mat4<T>& a );
    friend T operator *( const Vec3<T>& a, const Vec3<T>& b );
    friend Vec3<T> operator *( const Mat3<T>& a, const Vec3<T>& v );
    friend Vec3<T> operator *( const Vec3<T>& v, const Mat3<T>& a );
    friend Vec3<T> operator *( const Mat4<T>& a, const Vec3<T>& v );
    friend Vec3<T> operator /( const Vec3<T>& a, const double d );
    friend Vec3<T> operator ^( const Vec3<T>& a, const Vec3<T>& b );
    friend bool operator ==( const Vec3<T>& a, const Vec3<T>& b );
    friend bool operator !=( const Vec3<T>& a, const Vec3<T>& b );
    friend Vec3<T> minimum( const Vec3<T>& a, const Vec3<T>& b );
    friend Vec3<T> maximum( const Vec3<T>& a, const Vec3<T>& b );
    friend Vec3<T> prod( const Vec3<T>& a, const Vec3<T>& b );
    friend Vec3<T> cross( const Vec3<T>& a, const Vec3<T>& b );
};

typedef Vec3<int> Vec3i;
typedef Vec3<float> Vec3f;
typedef Vec3<double> Vec3d;

//==========[ class Vec4 ]=================================

template <class T>
class Vec4 {

    //---[ Private Variable Declarations ]-------

    // x, y, z, w
    T		n[4];

public:
	
    //---[ Constructors ]------------------------

    Vec4() { n[0] = 0.0; n[1] = 0.0; n[2] = 0.0; n[3] = 0.0; }
    Vec4( const T x, const T y, const T z, const T w )
    { n[0] = x; n[1] = y; n[2] = z; n[3] = w; }
    Vec4( const Vec4& v )
    { n[0] = v.n[0]; n[1] = v.n[1]; n[2] = v.n[2]; n[3] = v.n[3]; }

    //---[ Equal Operators ]---------------------

    Vec4<T>& operator =( const Vec4<T>& v )
    { n[0] = v.n[0]; n[1] = v.n[1]; n[2] = v.n[2]; n[3] = v.n[3];
        return *this; }
    Vec4<T>& operator +=( const Vec4<T>& v )
    { n[0] += v.n[0]; n[1] += v.n[1]; n[2] += v.n[2]; n[3] += v.n[3];
        return *this; }
    Vec4<T>& operator -= ( const Vec4<T>& v )
    { n[0] -= v.n[0]; n[1] -= v.n[1]; n[2] -= v.n[2]; n[3] -= v.n[3];
        return *this; }
    Vec4<T>& operator *= ( const T d )
    { n[0] *= d; n[1] *= d; n[2] *= d; n[3] *= d; return *this; }
    Vec4<T>& operator /= ( const T d )
    { n[0] /= d; n[1] /= d; n[2] /= d; n[3] /= d; return *this; }

    //---[ Access Operators ]--------------------

    T& operator []( int i )
    { return n[i]; }
    T operator []( int i ) const 
    { return n[i]; }

    //---[ Arithmetic Operators ]----------------

    Vec4<T> operator-( const Vec4<T>& a ) { return Vec4<T>(n[0]-a.n[0],n[1]-a.n[1],n[2]-a.n[2],n[3]-a.n[3]); }
    Vec4<T> operator+( const Vec4<T>& a ) { return Vec4<T>(a.n[0]+n[0],a.n[1]+n[1],a.n[2]+n[2],a.n[3]-n[3]); }

    //---[ Length Methods ]----------------------

    double length2() const
    { return n[0]*n[0] + n[1]*n[1] + n[2]*n[2] + n[3]*n[3]; }
    double length() const
    { return sqrt( length2() ); }

    //---[ Zero Test ]---------------------------

    bool isZero() const { return n[0]==0&&n[1]==0&&n[2]==0&&n[3]==0; }
    void zeroElements() { memset(n,0,4*sizeof(T)); }

    //---[ Normalization ]-----------------------

    void normalize() {
        double len = length();
        n[0] /= len; n[1] /= len; n[2] /= len; n[3] /= len;
    }
	
    //---[ Friend Methods ]----------------------

    friend T operator *( const Vec3<T>& a, const Vec4<T>& b );
    friend T operator *( const Vec4<T>& b, const Vec3<T>& a );
    friend Vec4<T> operator -( const Vec4<T>& v );
    friend Vec4<T> operator *( const Vec4<T>& a, const double d );
    friend Vec4<T> operator *( const double d, const Vec4<T>& a );
    friend T operator *( const Vec4<T>& a, const Vec4<T>& b );
    friend Vec4<T> operator *( const Mat4<T>& a, const Vec4<T>& v );
    friend Vec4<T> operator *( const Vec4<T>& v, const Mat4<T>& a );
    friend Vec4<T> operator /( const Vec4<T>& a, const double d );
    friend Vec4<T> operator ^( const Vec4<T>& a, const Vec4<T>& b );
    friend bool operator ==( const Vec4<T>& a, const Vec4<T>& b );
    friend bool operator !=( const Vec4<T>& a, const Vec4<T>& b );
    friend Vec4<T> minimum( const Vec4<T>& a, const Vec4<T>& b );
    friend Vec4<T> maximum( const Vec4<T>& a, const Vec4<T>& b );
    friend Vec4<T> prod( const Vec4<T>& a, const Vec4<T>& b );
};

typedef Vec4<int> Vec4i;
typedef Vec4<float> Vec4f;
typedef Vec4<double> Vec4d;

//==========[ Vec Methods ]================================

template <class T>
Vec<T>::Vec( int size, bool zeroElements ) {
    numElements = size;
    n = new T[size];

    if( !zeroElements )
        return;

    for( int i=0;i<size;i++ )
        n[i] = 0.0;
}

template <class T>
Vec<T>::Vec( const Vec<T>& v ) {
    numElements = v.numElements;
    n = new T[numElements];

    memcpy( n, v.n, numElements * sizeof(T) );
}

template <class T>
Vec<T>::~Vec() {
    delete [] n;
}

template <class T>
void Vec<T>::resize( int size, bool zeroElements ) {
    if( numElements != size ) {
        numElements = size;
        if( n != NULL )
            delete [] n;

        n = new T[size];
    }

    if( zeroElements )
        memset( n, 0, numElements * sizeof(T) );
}

template <class T>
void Vec<T>::zeroElements() {
    memset( n, 0, numElements * sizeof(T) );
}

template <class T>
Vec<T>& Vec<T>::operator=( const Vec<T>& v ) {
#ifdef _DEBUG
    if( v.numElements != numElements )
        throw VectorSizeMismatch();
#endif

    for( int i=0;i<numElements;i++ )
        n[i] = v.n[i];

    return *this;
}

template <class T>
Vec<T>& Vec<T>::operator+=( const Vec<T>& v ) {
#ifdef _DEBUG
    if( v.numElements != numElements )
        throw VectorSizeMismatch();
#endif

    for( int i=0;i<numElements;i++ )
        n[i] += v.n[i];

    return *this;
}

template <class T>
Vec<T>& Vec<T>::operator-=( const Vec<T>& v ) {
#ifdef _DEBUG
    if( v.numElements != numElements )
        throw VectorSizeMismatch();
#endif

    for( int i=0;i<numElements;i++ )
        n[i] -= v.n[i];

    return *this;
}

template <class T>
Vec<T>& Vec<T>::operator*=( const T d ) {
    for( int i=0;i<numElements;i++ )
        n[i] *= d;

    return *this;
}

template <class T>
Vec<T>& Vec<T>::operator/=( const T d ) {
    for( int i=0;i<numElements;i++ )
        n[i] /= d;

    return *this;
}

template <class T>
Vec<T> Vec<T>::operator-( const Vec<T>& v ) {
#ifdef _DEBUG
    if( v.numElements != numElements )
        throw VectorSizeMismatch();
#endif

    Vec<T>	result( numElements, false );

    for( int i=0;i<numElements;i++ )
        result.n[i] = n[i] - v.n[i];

    return result;
}

template <class T>
Vec<T> Vec<T>::operator+( const Vec<T>& v ) {
#ifdef _DEBUG
    if( v.numElements != numElements )
        throw VectorSizeMismatch();
#endif

    Vec<T>	result( numElements, false );

    for( int i=0;i<numElements;i++ )
        result.n[i] = v.n[i] + n[i];

    return result;
}

template <class T>
double Vec<T>::length2() const {
    double	result = 0.0;

    for( int i=0;i<numElements;i++ )
        result += n[i] * n[i];

    return result;
}

template <class T>
double Vec<T>::length() const {
    return sqrt( length2() );
}

template <class T>
void Vec<T>::normalize() {
    double len = length();

    for( int i=0;i<numElements;i++ )
        n[i] /= len;
}

template <class T>
bool Vec<T>::iszero() {
    for( int i=0;i<numElements;i++ )
        if( n[i] != 0 ) return false;

    return true;
}

template <class T>
T operator*( const Vec<T>& a, const Vec<T>& b ) {
#ifdef _DEBUG
    if( a.numElements != b.numElements )
        throw VectorSizeMismatch();
#endif

    double	result = 0.0;

    for( int i=0;i<a.numElements;i++ )
        result += a.n[i] * b.n[i];

    return result;
}

template <class T>
Vec<T> operator-( const Vec<T>& v ) {
    Vec<T>	result( v.numElements, false );

    for( int i=0;i<v.numElements;i++ )
        result.n[i] = -v.n[i];

    return result;
}

template <class T>
Vec<T> operator*( const Vec<T>& a, const double d ) {
    Vec<T>	result( a.numElements, false );

    for( int i=0;i<a.numElements;i++ )
        result.n[i] = a.n[i] * d;

    return result;
}

template <class T>
Vec<T> operator*( const double d, const Vec<T>& a ) {
    Vec<T>	result( a.numElements, false );

    for( int i=0;i<a.numElements;i++ )
        result.n[i] = a.n[i] * d;

    return result;
}

template <class T>
Vec<T> operator/( const Vec<T>& a, const double d ) {
    Vec<T>	result( a.numElements, false );

    for( int i=0;i<a.numElements;i++ )
        result.n[i] = a.n[i] / d;

    return result;
}

// template <class T>
// Vec<T> operator^( const Vec<T>& a, const Vec<T>& b ) {
// #ifdef _DEBUG
// 	if( a.numElements != b.numElements )
// 		throw VectorSizeMismatch();
// #endif

// 	return *this;
// }

template <class T>
bool operator==( const Vec<T>& a, const Vec<T>& b ) {
#ifdef _DEBUG
    if( a.numElements != b.numElements )
        throw VectorSizeMismatch();
#endif

    for( int i=0;i<a.numElements;i++ )
        if( a.n[i] != b.n[i] )
            return false;

    return true;
}

template <class T>
bool operator!=( const Vec<T>& a, const Vec<T>& b ) {
#ifdef _DEBUG
    if( a.numElements != b.numElements )
        throw VectorSizeMismatch();
#endif

    for( int i=0;i<a.numElements;i++ )
        if( a.n[i] == b.n[i] )
            return false;

    return true;
}

template <class T>
std::ostream& 
operator<<( std::ostream& os, const Vec<T>& a ) {
    os << a.numElements;

    for( int i = 0; i < a.numElements; i++)
        os << " " << a.n[i];

    return os;
}

template <class T>
std::istream& operator>>( std::istream& is, const Vec<T>& a ) {
    return is;
}

template <class T>
Vec<T> minimum( const Vec<T>& a, const Vec<T>& b ) {
#ifdef _DEBUG
    if( a.numElements != b.numElements )
        throw VectorSizeMismatch();
#endif

    Vec<T>	result( a.numElements, false );

    for( int i=0;i<a.numElements;i++ )
        result.n[i] = minimum( a.n[i], b.n[i] );

    return result;
}

template <class T>
Vec<T> maximum( const Vec<T>& a, const Vec<T>& b ) {
#ifdef _DEBUG
    if( a.numElements != b.numElements )
        throw VectorSizeMismatch();
#endif

    Vec<T>	result( a.numElements, false );

    for( int i=0;i<a.numElements;i++ )
        result.n[i] = maximum( a.n[i], b.n[i] );

    return result;
}

template <class T>
Vec<T> prod( const Vec<T>& a, const Vec<T>& b ) {
#ifdef _DEBUG
    if( a.numElements != b.numElements )
        throw VectorSizeMismatch();
#endif

    Vec<T>	result( a.numElements, false );

    for( int i=0;i<a.numElements;i++ )
        result.n[i] = a.n[i] *b.n[i];

    return result;
}


//==========[ Inline Method Definitions (Vectors) ]========

template <class T>
inline T operator *( const Vec3<T>& a, const Vec4<T>& b ) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + b[3];
}

template <class T>
inline T operator *( const Vec4<T>& b, const Vec3<T>& a ) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + b[3];
}

template <class T>
inline Vec3<T> operator -(const Vec3<T>& v) {
    return Vec3<T>( -v.n[0], -v.n[1], -v.n[2] );
}

// template <class T>
inline Vec3<double> operator *(const Vec3<double>& a, const double d ) {
    return Vec3<double>( a.n[0] * d, a.n[1] * d, a.n[2] * d );
}

// template <class T>
inline Vec3<double> operator *(const double d, const Vec3<double>& a) {
    return a * d;
}

inline Vec3<double> operator *(const Mat4<double>& a, const Vec3<double>& v){
    return Vec3<double>( a.n[0]*v.n[0]+a.n[1]*v.n[1]+a.n[2]*v.n[2]+a.n[3],
                         a.n[4]*v.n[0]+a.n[5]*v.n[1]+a.n[6]*v.n[2]+a.n[7],
                         a.n[8]*v.n[0]+a.n[9]*v.n[1]+a.n[10]*v.n[2]+a.n[11] );
}

template <class T>
inline Vec3<T> operator *(const Vec3<T>& v, Mat4<T>& a) {
    return a.transpose() * v;
}

// template <class T>
inline double operator *(const Vec3<double>& a, const Vec3<double>& b){
    return a.n[0]*b.n[0] + a.n[1]*b.n[1] + a.n[2]*b.n[2];
}

//template <double>
inline Vec3<double> operator *( const Mat3<double>& a, const Vec3<double>& v ) {
    return Vec3<double>( a.n[0]*v.n[0]+a.n[1]*v.n[1]+a.n[2]*v.n[2],
                         a.n[3]*v.n[0]+a.n[4]*v.n[1]+a.n[5]*v.n[2],
                         a.n[6]*v.n[0]+a.n[7]*v.n[1]+a.n[8]*v.n[2] );
}

template <class T>
inline Vec3<T> operator *( const Vec3<T>& v, const Mat3<T>& a ) {
    return a.transpose() * v;
}

template <class T>
inline Vec3<T> operator /(const Vec3<T>& a, const double d){
    return Vec3<T>( a.n[0] / d, a.n[1] / d, a.n[2] / d );
}

// template <class T>
inline Vec3<double> operator ^(const Vec3<double>& a, const Vec3<double>& b) {
    return Vec3<double>( a.n[1]*b.n[2] - a.n[2]*b.n[1],
                         a.n[2]*b.n[0] - a.n[0]*b.n[2],
                         a.n[0]*b.n[1] - a.n[1]*b.n[0] );
}

template <class T>
inline bool operator ==(const Vec3<T>& a, const Vec3<T>& b) {
    return a.n[0]==b.n[0] && a.n[1] == b.n[1] && a.n[2] == b.n[2];
}

template <class T>
inline bool operator !=(const Vec3<T>& a, const Vec3<T>& b) {
    return !( a == b );
}


template <class T>
inline void swap( Vec3<T>& a, Vec3<T>& b ) {
    Vec3<T> t( a );
    a = b;
    b = t;
}

template <class T>
inline Vec3<T> minimum( const Vec3<T>& a, const Vec3<T>& b ) {
    return Vec3<T>( minimum(a.n[0],b.n[0]), minimum(a.n[1],b.n[1]), minimum(a.n[2],b.n[2]) );
}

template <class T>
inline Vec3<T> maximum(const Vec3<T>& a, const Vec3<T>& b) {
    return Vec3<T>( maximum(a.n[0],b.n[0]), maximum(a.n[1],b.n[1]), maximum(a.n[2],b.n[2]) );
}

template <class T>
inline Vec3<T> prod(const Vec3<T>& a, const Vec3<T>& b ) {
    return Vec3<T>( a.n[0]*b.n[0], a.n[1]*b.n[1], a.n[2]*b.n[2] );
}


// template <class T>
inline Vec3<double> cross(const Vec3<double>& a, const Vec3<double>& b ) {
    return Vec3<double>( a.n[1]*b.n[2] - a.n[2]*b.n[1], 
                         a.n[2]*b.n[0] - a.n[0]*b.n[2], 
                         a.n[0]*b.n[1] - a.n[1]*b.n[0] );
}

template <class T>
inline Vec4<T> operator -( const Vec4<T>& v ) {
    return Vec4<T>( -v.n[0], -v.n[1], -v.n[2], -v.n[3] );
}

template <class T>
inline Vec4<T> operator *(const Vec4<T>& a, const double d ) {
    return Vec4<T>( a.n[0] * d, a.n[1] * d, a.n[2] * d, a.n[3] * d );
}

template <class T>
inline Vec4<T> operator *(const double d, const Vec4<T>& a) {
    return a * d;
}

template <class T>
inline T operator *(const Vec4<T>& a, const Vec4<T>& b) {
    return a.n[0]*b.n[0] + a.n[1]*b.n[1] + a.n[2]*b.n[2] + a.n[3]*b.n[3];
}

// template <class double>
inline Vec4<double> operator *(const Mat4<double>& a, const Vec4<double>& v) {
    return Vec4<double>( a.n[0]*v.n[0]+a.n[1]*v.n[1]+a.n[2]*v.n[2]+a.n[3]*v.n[3],
                         a.n[4]*v.n[0]+a.n[5]*v.n[1]+a.n[6]*v.n[2]+a.n[7]*v.n[3],
                         a.n[8]*v.n[0]+a.n[9]*v.n[1]+a.n[10]*v.n[2]+a.n[11]*v.n[3],
                         a.n[12]*v.n[0]+a.n[13]*v.n[1]+a.n[14]*v.n[2]+a.n[15]*v.n[3]);
}

template <class T>
inline Vec4<T> operator *( const Vec4<T>& v, Mat4<T>& a ){
    return a.transpose() * v;
}

template <class T>
inline Vec4<T> operator /(const Vec4<T>& a, const double d) {
    return Vec4<T>( a.n[0] / d, a.n[1] / d, a.n[2] / d, a.n[3] / d );
}

template <class T>
inline bool operator ==(const Vec4<T>& a, const Vec4<T>& b) {
    return a.n[0] == b.n[0] && a.n[1] == b.n[1] && a.n[2] == b.n[2] 
        && a.n[3] == b.n[3];
}

template <class T>
inline bool operator !=(const Vec4<T>& a, const Vec4<T>& b) {
    return !( a == b );
}


template <class T>
inline void swap( Vec4<T>& a, Vec4<T>& b ) {
    Vec4<T> t( a );
    a = b;
    b = t;
}

template <class T>
inline Vec4<T> minimum( const Vec4<T>& a, const Vec4<T>& b ) {
    return Vec4<T>( minimum(a.n[0],b.n[0]), minimum(a.n[1],b.n[1]), minimum(a.n[2],b.n[2]),
                    minimum(a.n[3],b.n[3]) );
}

template <class T>
inline Vec4<T> maximum(const Vec4<T>& a, const Vec4<T>& b) {
    return Vec4<T>( maximum(a.n[0],b.n[0]), maximum(a.n[1],b.n[1]), maximum(a.n[2],b.n[2]),
                    maximum(a.n[3],b.n[3]) );
}

template <class T>
inline Vec4<T> prod(const Vec4<T>& a, const Vec4<T>& b ) {
    return Vec4<T>( a.n[0]*b.n[0], a.n[1]*b.n[1], a.n[2]*b.n[2], a.n[3]*b.n[3] );
}

#pragma warning(pop)

#endif
