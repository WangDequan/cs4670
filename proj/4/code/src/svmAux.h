#ifndef svmAux
#define svmAux

#include <math.h>

/* Memory Related functions */

template <class T>
void SwapBuffer(T *buf0, T *buf1, int size)
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
void VerticalFlipBuffer(T *buf, int width, int height)
{
    T *top = buf + (height - 1) * width;
    T *bottom = buf;

    while (top > bottom) {
        SwapBuffer(top, bottom, width);
        top -= width;
        bottom += width;
    }
}

inline void ApplyHomography(double &u, double &v, const CTransform3x3 &H, double x, double y, double z)
{
    u = H[0][0] * x + H[0][1] * y + H[0][2] * z;
    v = H[1][0] * x + H[1][1] * y + H[1][2] * z;
    double w = H[2][0] * x + H[2][1] * y + H[2][2] * z;
    u /= w;
    v /= w;
}

inline void Matrix3by3Inv(double invM[3][3], const double m[3][3])
{
    int i, j, k;
    double temp;
    double bigm[6][3];
    /*   Declare identity matrix   */

    invM[0][0] = 1;
    invM[0][1] = 0;
    invM[0][2] = 0;
    invM[1][0] = 0;
    invM[1][1] = 1;
    invM[1][2] = 0;
    invM[2][0] = 0;
    invM[2][1] = 0;
    invM[2][2] = 1;

    for (i = 0; i < 3; i++) {
        for (j = 0;  j < 3;  j++) {
            bigm[i][j] = m[i][j];
            bigm[i + 3][j] = invM[i][j];
        }
    }

    /*   Work across by columns   */
    for (i = 0;  i < 3;  i++) {
        for (j = i;  (bigm[i][j] == 0.0) && (j < 3);  j++)
            ;
        if (j == 3) {
            fprintf (stderr, "error:  cannot do inverse matrix\n");
            //printf ("error:  cannot do inverse matrix\n");
            exit (2);
        } else if (i != j) {
            for (k = 0;  k < 6;  k++) {
                temp = bigm[k][i];
                bigm[k][i] = bigm[k][j];
                bigm[k][j] = temp;
            }
        }

        /*   Divide original row   */
        for (j = 5;  j >= i;  j--)
            bigm[j][i] /= bigm[i][i];

        /*   Subtract other rows   */

        for (j = 0;  j < 3;  j++)
            if (i != j)
                for (k = 5;  k >= i;  k--)
                    bigm[k][j] -= bigm[k][i] * bigm[i][j];
    }

    for (i = 0;  i < 3;  i++)
        for (j = 0;  j < 3;  j++)
            invM[i][j] = bigm[i + 3][j];
}

#endif
