#ifndef SUB_BAND_IMAGE_PYRAMID_H
#define SUB_BAND_IMAGE_PYRAMID_H

#include "Common.h"
#include "ParametersMap.h"

template<class T>
class SBPyramid : public std::vector<CImageOf<T> >
{
public:
    static ParametersMap getDefaultParameters();

    ParametersMap getParameters() const;

    SBPyramid(int nLevels = 0);
    SBPyramid(CImageOf<T> &image, int nOctaves, int nSubOctaves, int minTopSize);
    SBPyramid(CImageOf<T> &image, const ParametersMap &params = getDefaultParameters());

    int getLevelIdx(int oct, int sub) {
        return ((_nSubOctaves + 1) * oct) + sub;
    }

    int getNLevels() const {
        return this->size();
    }

    int getNOctaves() const {
        return _nOctaves;
    }

    int getNSubOctaves() const {
        return _nSubOctaves;
    }

    double levelScale(int level) const {
        return double((*this)[level].Shape().width) / double((*this)[0].Shape().width);
    }

private:
    int _nOctaves, _nSubOctaves, _minTopSize;
    void _init(CImageOf<T> &image, int nOctaves, int nSubOctaves, int minTopSize);
};

typedef SBPyramid<float> SBFloatPyramid;

#include "SubBandImagePyramid.inl"

#endif // SUB_BAND_IMAGE_PYRAMID_H