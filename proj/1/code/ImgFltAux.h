#ifndef ImgFltAux
#define ImgFltAux

#include <math.h>
#include <stdlib.h>

// Fixed by Loren.
#ifndef __max
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

/* Memory Related functions */

inline void RectIntersection(int c[4], const int a[4], const int b[4])
{
    c[0] = __max(a[0], b[0]);
    c[1] = __min(a[1], b[1]);
    c[2] = __max(a[2], b[2]);
    c[3] = __min(a[3], b[3]);
}

inline int IsPtInRect(int x, int y, const int r[4])
{
    return r[0] <= x && x < r[1] && r[2] <= y && y < r[3];
}

template <class T>
void SwapBuffer(T* buf0, T* buf1, int size)
{
    while (size > 0) {
        T tmp;

        tmp = *buf0;
        *buf0 = *buf1;
        *buf1 = tmp;

        buf0++;
        buf1++;

        size--;
    }
}

template <class T>
void VerticalFlipBuffer(T* buf, int width, int height)
{
    T* top = buf + (height - 1) * width;
    T* bottom = buf;

    while (top > bottom) {
        SwapBuffer(top, bottom, width);
        top -= width;
        bottom += width;
    }
}

void double2byte(unsigned char* btBuf, const double* dbBuf, int size);

#endif
