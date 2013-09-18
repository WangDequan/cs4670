#include "ImgFltAux.h"

void double2byte(unsigned char* btBuf, const double* dbBuf, int size)
{
    for(int i = 0; i < size; i++) {
        double tmp = dbBuf[i];
        tmp = __min(255.0, tmp);
        tmp = __max(0.0, tmp);
        btBuf[i] = (unsigned char)(int)floor(tmp);
    }
}
