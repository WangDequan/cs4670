#define N_OCTAVES_KEY      "n_octaves"
#define N_SUB_OCTAVES_KEY  "n_sub_octaves"
#define MIN_TOP_SIZE_KEY   "min_top_size"

template<class T>
SBPyramid<T>::SBPyramid(int nLevels)
{
    std::vector<CImageOf<T> >::resize(nLevels);
}

template<class T>
void
downSampleByHalf(const CImageOf<T> &src, CImageOf<T> &dst)
{
    int nBands = src.Shape().nBands;
    for (int i = 0; i < dst.Shape().height; i++) {
        const T *psrc = (const T *) src.PixelAddress(0, 2 * i, 0);
        T *pdst = (T *) dst.PixelAddress(0, i, 0);
        for (int j = 0; j < dst.Shape().width; j++, psrc += (2 * nBands), pdst += nBands) {
            for (int k = 0; k < nBands; k++) {
                pdst[k] = psrc[k];
            }
        }
    }
}

template<class T>
ParametersMap
SBPyramid<T>::getDefaultParameters()
{
    ParametersMap params;

    params.set(N_OCTAVES_KEY    , 1);
    params.set(N_SUB_OCTAVES_KEY, 0);
    params.set(MIN_TOP_SIZE_KEY , 128);

    return params;
}

template<class T>
ParametersMap
SBPyramid<T>::getParameters() const
{
    ParametersMap params;

    params.set(N_OCTAVES_KEY    , _nOctaves);
    params.set(N_SUB_OCTAVES_KEY, _nSubOctaves);
    params.set(MIN_TOP_SIZE_KEY , _minTopSize);

    return params;
}

template<class T>
SBPyramid<T>::SBPyramid(CImageOf<T> &image, const ParametersMap &params)
{
    int nOctaves = params.getInt(N_OCTAVES_KEY);
    int nSubOctaves = params.getInt(N_SUB_OCTAVES_KEY);
    int minTopSize = params.getInt(MIN_TOP_SIZE_KEY);
    _init(image, nOctaves, nSubOctaves, minTopSize);
}

template<class T>
SBPyramid<T>::SBPyramid(CImageOf<T> &image, int nOctaves, int nSubOctaves, int minTopSize)
{
    _init(image, nOctaves, nSubOctaves, minTopSize);
}

template<class T>
void
SBPyramid<T>::_init(CImageOf<T> &image, int nOctaves, int nSubOctaves, int minTopSize)
{
    using namespace std;
    assert(nOctaves > 0);
    assert(nSubOctaves >= 0);

    _nOctaves = nOctaves;
    _nSubOctaves = nSubOctaves;
    _minTopSize = minTopSize;

    this->resize((nOctaves - 1) * (nSubOctaves + 1) + 1);

    CShape octShape = image.Shape();

    // Allocate memory for each of the levels
    bool stop = false;
    int level = 0;
    for(int oct = 0; (oct < nOctaves) && (!stop); oct++) {
        (*this)[level].ReAllocate(octShape);
        level++;

        if(oct < nOctaves - 1) {
            for (int suboct = 0; suboct < nSubOctaves; suboct++, level++) {
                double scale = 1.0 / (1.0 + ((suboct + 1.0) / (nSubOctaves + 1.0)));

                int w = int(octShape.width * scale);
                int h = int(octShape.height * scale);

                if( std::min(w, h) < minTopSize) {
                    stop = true;
                    break;
                }

                CShape subOctShape(w, h, octShape.nBands);
                (*this)[level].ReAllocate(subOctShape);
            }
        }

        octShape.width = (octShape.width + 1) / 2;
        octShape.height = (octShape.height + 1) / 2;
    }
    this->resize(level);

    // Fill in first level
    CopyPixels(image, (*this)[0]);
    for (int i = 0; i < nSubOctaves && (i + 1) < this->size(); i++) {
        double scale = (*this)[i + 1].Shape().width / double((*this)[0].Shape().width);
        WarpGlobal((*this)[0], (*this)[i + 1], CTransform3x3::Scale(1.0 / scale, 1.0 / scale), eWarpInterpLinear);
    }

    // Fill in the remaining levels by downsampling the corresponding suboctaves
    // in the prev level by 2
    for (int i = 1; i < nOctaves && i < this->size(); i++) {
        int idx1 = getLevelIdx(i - 1, 0);
        int idx2 = getLevelIdx(i, 0);
        if(idx2 >= getNLevels()) break;
        downSampleByHalf((*this)[idx1], (*this)[idx2]);

        if (i < nOctaves - 1) {
            int idx1 = getLevelIdx(i - 1, 1);
            int idx2 = getLevelIdx(i, 1);
            for (int j = 1; j <= nSubOctaves && idx1 < getNLevels() && idx2 < getNLevels(); j++, idx1++, idx2++) {
                downSampleByHalf((*this)[idx1], (*this)[idx2]);
            }
        }
    }
}

