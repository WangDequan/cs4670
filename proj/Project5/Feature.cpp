#include "Feature.h"

#define PI 3.14159265

static
double
_round(double d)
{
    return floor(d + 0.5);
}

CByteImage
FeatureExtractor::renderPosNegComponents(const Feature &feat) const
{
    // Create two images, one for the positive weights and another
    // one for the negative weights
    Feature pos(feat.Shape()), neg(feat.Shape());
    pos.ClearPixels();
    neg.ClearPixels();

    for(int y = 0; y < pos.Shape().height; y++) {
        float *svmIt = (float *) feat.PixelAddress(0, y, 0);
        float *p = (float *) pos.PixelAddress(0, y, 0);
        float *n = (float *) neg.PixelAddress(0, y, 0);

        for(int x = 0; x < pos.Shape().width * pos.Shape().nBands; x++, p++, n++, svmIt++) {
            if(*svmIt < 0) *n = fabs(*svmIt);
            else if(*svmIt > 0) *p = *svmIt;
        }
    }

    CByteImage negViz, posViz;
    posViz = this->render(pos, true);
    negViz = this->render(neg, true);

    // Put positive and negative weights images side by side in a color image.
    // Negative weights show up as red and positive weights show up as green.
    CByteImage negposViz(CShape(posViz.Shape().width * 2, posViz.Shape().height, 3));
    negposViz.ClearPixels();

    for(int y = 0; y < negposViz.Shape().height; y++) {
        uchar *n = (uchar *) negViz.PixelAddress(0, y, 0);
        uchar *np = (uchar *) negposViz.PixelAddress(0, y, 2);
        for(int x = 0; x < negViz.Shape().width; x++, n++, np += 3) {
            *np = *n;
        }

        uchar *p = (uchar *) posViz.PixelAddress(0, y, 0);
        np = (uchar *) negposViz.PixelAddress(posViz.Shape().width, y, 1);
        for(int x = 0; x < negViz.Shape().width; x++, p++, np += 3) {
            *np = *p;
        }
    }

    return negposViz;
}

void
FeatureExtractor::operator()(const CByteImage &img, Feature &feat) const
{
    CFloatImage img_(img.Shape());
    TypeConvert(img, img_);
    this->operator()(img_, feat);
}

void
FeatureExtractor::operator()(const CroppedImageDatabase &db, FeatureCollection &feats) const
{
    int n = db.getSize();

    feats.resize(n);
    for(int i = 0; i < n; i++) {
        CByteImage img;
        ReadFile(img, db.getFilename(i).c_str());

        (*this)(img, feats[i]);
    }
}

void
FeatureExtractor::operator()(const SBFloatPyramid &imPyr, FeaturePyramid &featPyr) const
{
    featPyr.resize(imPyr.getNLevels());

    CByteImage x;
    for (int i = 0; i < imPyr.getNLevels(); i++) {
        this->operator()(imPyr[i], featPyr[i]);
    }
}

CByteImage
FeatureExtractor::render(const Feature &f, bool normalizeFeat) const
{
    if(normalizeFeat) {
        CShape shape = f.Shape();
        Feature fAux(shape);

        float fMin, fMax;
        f.getRangeOfValues(fMin, fMax);

        for(int y = 0; y < shape.height; y++) {
            float *fIt = (float *) f.PixelAddress(0, y, 0);
            float *fAuxIt = (float *) fAux.PixelAddress(0, y, 0);

            for(int x = 0; x < shape.width * shape.nBands; x++, fAuxIt++, fIt++) {
                *fAuxIt = (*fIt) / fMax;
            }
        }

        return this->render(fAux);
    } else {
        return this->render(f);
    }
}

std::vector<CByteImage>
FeatureExtractor::render(const FeaturePyramid &f, bool normalizeFeat) const
{
    std::vector<CByteImage> res(f.getNLevels());
    for(int i = 0; i < res.size(); i++) {
        res[i] = render(f[i], normalizeFeat);
    }
    return res;
}

FeatureExtractor *
FeatureExtractor::create(const std::string &featureType, const ParametersMap &params)
{
    ParametersMap tmp = params;
    tmp[FEATURE_TYPE_KEY] = featureType;
    return FeatureExtractor::create(tmp);
}

FeatureExtractor *
FeatureExtractor::create(ParametersMap params)
{
    std::string featureType = params.getStr(FEATURE_TYPE_KEY);
    params.erase(FEATURE_TYPE_KEY);

    if(strcasecmp(featureType.c_str(), "ti") == 0) return new TinyImageFeatureExtractor(params);
    else if(strcasecmp(featureType.c_str(), "tig"    ) == 0) return new TinyImageGradFeatureExtractor(params);
    else if(strcasecmp(featureType.c_str(), "hog"    ) == 0) return new HOGFeatureExtractor(params);

    // Implement other features or call a feature extractor with a different set
    // of parameters by adding more calls here.
    else if(strcasecmp(featureType.c_str(), "custom1") == 0) throw CError("not implemented");
    else if(strcasecmp(featureType.c_str(), "custom2") == 0) throw CError("not implemented");
    else if(strcasecmp(featureType.c_str(), "custom3") == 0) throw CError("not implemented");
    else {
        throw CError("Unknown feature type: %s", featureType.c_str());
    }
}

ParametersMap
FeatureExtractor::getDefaultParameters(const std::string &featureType)
{
    ParametersMap params;
    if(strcasecmp(featureType.c_str(), "ti"     ) == 0) params = TinyImageFeatureExtractor::getDefaultParameters();
    else if(strcasecmp(featureType.c_str(), "tig"    ) == 0) params = TinyImageGradFeatureExtractor::getDefaultParameters();
    else if(strcasecmp(featureType.c_str(), "hog"    ) == 0) params = HOGFeatureExtractor::getDefaultParameters();

    // Implement other features or call a feature extractor with a different set
    // of parameters by adding more calls here.
    else if(strcasecmp(featureType.c_str(), "custom1") == 0) throw CError("not implemented");
    else if(strcasecmp(featureType.c_str(), "custom2") == 0) throw CError("not implemented");
    else if(strcasecmp(featureType.c_str(), "custom3") == 0) throw CError("not implemented");
    else {
        throw CError("Unknown feature type: %s", featureType.c_str());
    }

    params[FEATURE_TYPE_KEY] = featureType;

    return params;
}

void
FeatureExtractor::save(FILE *f, const FeatureExtractor *feat)
{
    ParametersMap params = feat->getParameters();
    params[FEATURE_TYPE_KEY] = feat->getFeatureType();
    params.save(f);
}

FeatureExtractor *
FeatureExtractor::load(FILE *f)
{
    ParametersMap params;
    params.load(f);
    return FeatureExtractor::create(params);
}

// ============================================================================
// TinyImage
// ============================================================================

static const char *SCALE_KEY = "scale";

ParametersMap
TinyImageFeatureExtractor::getDefaultParameters()
{
    ParametersMap params;
    params.set(SCALE_KEY, 0.2);
    return params;
}

ParametersMap
TinyImageFeatureExtractor::getParameters() const
{
    ParametersMap params;
    params.set(SCALE_KEY, _scale);
    return params;
}

TinyImageFeatureExtractor::TinyImageFeatureExtractor(const ParametersMap &params)
{
    _scale = params.getFloat(SCALE_KEY);
}

void
TinyImageFeatureExtractor::operator()(const CFloatImage &imgRGB, Feature &feat) const
{
    int targetW = _round(imgRGB.Shape().width * _scale);
    int targetH = _round(imgRGB.Shape().height * _scale);

    CFloatImage tinyImg(targetW, targetH, 1);

    CFloatImage imgG;
    convertRGB2GrayImage(imgRGB, imgG);

    CTransform3x3 s = CTransform3x3::Scale( 1. / _scale, 1. / _scale );

    WarpGlobal(imgG, tinyImg, s, eWarpInterpLinear);

    feat = tinyImg;
}

CByteImage
TinyImageFeatureExtractor::render(const Feature &f) const
{
    CByteImage viz;
    TypeConvert(f, viz);
    return viz;
}

// ============================================================================
// TinyImage Gradient
// ============================================================================

ParametersMap
TinyImageGradFeatureExtractor::getDefaultParameters()
{
    return TinyImageFeatureExtractor::getDefaultParameters();
}

ParametersMap
TinyImageGradFeatureExtractor::getParameters() const
{
    ParametersMap params;
    params.set(SCALE_KEY, _scale);
    return params;
}

TinyImageGradFeatureExtractor::TinyImageGradFeatureExtractor(const ParametersMap &params)
{
    _scale = params.getFloat(SCALE_KEY);

    static float derivKvals[3] = { -1, 0, 1};

    _kernelDx.ReAllocate(CShape(3, 1, 1), derivKvals, false, 1);
    _kernelDx.origin[0] = 1;

    _kernelDy.ReAllocate(CShape(1, 3, 1), derivKvals, false, 1);
    _kernelDy.origin[0] = 1;
}

void
TinyImageGradFeatureExtractor::operator()(const CFloatImage &imgRGB_, Feature &feat) const
{
    int targetW = _round(imgRGB_.Shape().width * _scale);
    int targetH = _round(imgRGB_.Shape().height * _scale);

    // Compute tiny image gradient feature, output should be a _targetW by _targetH
    // grayscale image, similar to tiny image. The difference here is that you will
    // compute the gradients in the x and y directions, followed by the gradient
    // magnitude.
    //
    // Steps are:
    // 1) Convert image to grayscale (see convertRGB2GrayImage in Utils.h)
    // 2) Resize image to be _targetW by _targetH
    // 3) Compute gradients in x and y directions
    // 4) Compute gradient magnitude
    //
    // Useful functions:
    // convertRGB2GrayImage, TypeConvert, WarpGlobal, Convolve
    
    // Convert image to grayscale
    CFloatImage imgG;
    convertRGB2GrayImage(imgRGB_, imgG);

    // Resize image to be targetW by targetH
    CFloatImage tinyImg(targetW, targetH, 1);
    CTransform3x3 s = CTransform3x3::Scale( 1. / _scale, 1. / _scale );
    WarpGlobal(imgG, tinyImg, s, eWarpInterpLinear);
    
    // Compute gradients in x and y directions
    CFloatImage dX, dY;
    Convolve(tinyImg, dX, _kernelDx);   // gradient in x direction 
    Convolve(tinyImg, dY, _kernelDy);   // gradient in y direction

    // Compute gradient magnitude
    CFloatImage magImg(targetW, targetH, 1);

    for (int j=0; j<targetH; j++) {
        for (int i=0; i<targetW; i++) {
            // gradient magnitude = sqrt(dx^2 + dy^2)
            magImg.Pixel(i, j, 0) = sqrt(pow(dX.Pixel(i, j, 0), 2) + pow(dY.Pixel(i, j, 0), 2));
        }
    }

    // Return feature
    feat = magImg;
}

CByteImage
TinyImageGradFeatureExtractor::render(const Feature &f) const
{
    CByteImage viz;
    TypeConvert(f, viz);
    return viz;
}

// ============================================================================
// HOG
// ============================================================================

const char *N_ANGULAR_BINS_KEY     = "n_angular_bins";
const char *UNSIGNED_GRADIENTS_KEY = "unsigned_gradients";
const char *CELL_SIZE_KEY          = "cell_size";

ParametersMap
HOGFeatureExtractor::getDefaultParameters()
{
    ParametersMap params;
    params.set(N_ANGULAR_BINS_KEY    , 18);
    params.set(UNSIGNED_GRADIENTS_KEY, 1);
    params.set(CELL_SIZE_KEY         , 6);
    return params;
}

ParametersMap
HOGFeatureExtractor::getParameters() const
{
    ParametersMap params;
    params.set(N_ANGULAR_BINS_KEY    , _nAngularBins);
    params.set(UNSIGNED_GRADIENTS_KEY, _unsignedGradients);
    params.set(CELL_SIZE_KEY         , _cellSize);
    return params;
}

HOGFeatureExtractor::HOGFeatureExtractor(const ParametersMap &params)
{
    _nAngularBins = params.getInt(N_ANGULAR_BINS_KEY);
    _unsignedGradients = params.getInt(UNSIGNED_GRADIENTS_KEY);
    _cellSize = params.getInt(CELL_SIZE_KEY);

    static float derivKvals[3] = { -1, 0, 1};

    _kernelDx.ReAllocate(CShape(3, 1, 1), derivKvals, false, 1);
    _kernelDx.origin[0] = 1;

    _kernelDy.ReAllocate(CShape(1, 3, 1), derivKvals, false, 1);
    _kernelDy.origin[0] = 1;

    // Visualization Stuff
    // A set of patches representing the bin orientations. When drawing a hog cell
    // we multiply each patch by the hog bin value and add all contributions up to
    // form the visual representation of one cell. Full HOG is achieved by stacking
    // the viz for individual cells horizontally and vertically.
    _oriMarkers.resize(_nAngularBins);
    const int ms = 11;
    CShape markerShape(ms, ms, 1);

    // FIXME: add patches for contrast sensitive dimensions (half filled circle)

    // First patch is a horizontal line
    _oriMarkers[0].ReAllocate(markerShape, true);
    _oriMarkers[0].ClearPixels();
    for(int i = 1; i < ms - 1; i++) _oriMarkers[0].Pixel(/*floor(*/ ms / 2 /*)*/, i, 0) = 1;

    // The other patches are obtained by rotating the first one
    CTransform3x3 T = CTransform3x3::Translation((ms - 1) / 2.0, (ms - 1) / 2.0);
    for(int angBin = 1; angBin < _nAngularBins; angBin++) {
        double theta;
        if(_unsignedGradients) theta = 180.0 * (double(angBin) / _nAngularBins);
        else theta = 360.0 * (double(angBin) / _nAngularBins);
        CTransform3x3 R  = T * CTransform3x3::Rotation(theta) * T.Inverse();

        _oriMarkers[angBin].ReAllocate(markerShape, true);
        _oriMarkers[angBin].ClearPixels();

        WarpGlobal(_oriMarkers[0], _oriMarkers[angBin], R, eWarpInterpLinear);
    }
}

void
HOGFeatureExtractor::operator()(const CFloatImage &img, Feature &feat) const
{
    // Compute the Histogram of Oriented Gradients feature
    //
    // Steps are:
    // 1) Compute gradients in x and y directions. We provide the
    //    derivative kernel proposed in the paper in _kernelDx and
    //    _kernelDy.
    // 2) Compute gradient magnitude and orientation
    // 3) Add contribution each pixel to HOG cells whose
    //    support overlaps with pixel. The contribution should
    //    be weighted by a gaussian centered at the corresponding
    //    HOG cell. Each cell has a support of size
    //    _cellSize and each histogram has _nAngularBins. Note that
    //    pixels away from the borders of the image should contribute to
    //    at least four HOG cells.
    // 4) Normalize HOG for each cell. One simple strategy that is
    //    is also used in the SIFT descriptor is to first threshold
    //    the bin values so that no bin value is larger than some
    //    threshold (we leave it up to you do find this value) and
    //    then re-normalize the histogram so that it has norm 1. A more
    //    elaborate normalization scheme is proposed in Dalal & Triggs
    //    paper but we leave that as extra credit.
    //
    // Useful functions:
    // convertRGB2GrayImage, TypeConvert, WarpGlobal, Convolve

    // Get image dimensions
    int imgWidth = img.Shape().width; 
    int imgHeight = img.Shape().height;

    // Compute gradients in x and y directions
    CFloatImage dxImg, dyImg;
    Convolve(img, dxImg, _kernelDx);   // gradient in x direction 
    Convolve(img, dyImg, _kernelDy);   // gradient in y direction

    // Initialize feature image
    int targetW = ceil((double)imgWidth / _cellSize);
    int targetH = ceil((double)imgHeight / _cellSize);
    CFloatImage hogImg(targetW, targetH, _nAngularBins);
    hogImg.ClearPixels();

    // Initialize variables
    double dx, dy;                  // gradient values in x and y directions
    int cx, cy, bin;                // variables for indexing into HOG image
    double magnitude, orientation;  // gradient magnitude and orientation variables
    bool up, down, left, right;     // booleans for determining overlapping cells

    // Calculate gradient magnitude and orientation, adding the weighted values to the appropriate bins of the appropriate cells
    for (int j=0; j<imgHeight; j++) {
        for (int i=0; i<imgWidth; i++) {
            dx = dxImg.Pixel(i, j, 0);
            dy = dyImg.Pixel(i, j, 0);
            
            // Compute gradient magnitude
            magnitude = sqrt(pow(dx, 2) + pow(dy, 2));

            // Compute gradient orientation
            if(dx == 0.0 && dy == 0.0)
                orientation = 0.0;
	        else
                orientation = atan2(dy, dx); 
            
            // Find which cell the pixel is currently in
            cx = i/_cellSize;
            cy = j/_cellSize;

            // Find which bin the gradient orientation corresponds too (we add an offset so that -PI corresponds to bin 0)
            bin = (floor((double)(orientation+PI)/(2*PI/_nAngularBins)) == _nAngularBins) ? _nAngularBins-1 : floor((double)(orientation+PI)/(2*PI/_nAngularBins));

            // Add the weighted gradient magnitude to the appropriate angular bin of the current cell
            hogImg.Pixel(cx, cy, bin) += gaussWeight(i, j, (cx+0.5)*_cellSize, (cy+0.5)*_cellSize) * magnitude;

            //=========================================================
            // Determine which other cells the pixel contributes to
            //=========================================================
            up = down = left = right = false;
            
            // If the pixel is on the left half of the current cell and a cell exists to the left of the current cell, 
            // add the pixel's weighted contribution the appropriate bin in the left cell, and set the appropriate boolean
            if ((i < (cx+0.5)*_cellSize) && (cx-1 > 0)) {
                hogImg.Pixel(cx-1, cy, bin) += gaussWeight(i, j, (cx-0.5)*_cellSize, (cy+0.5)*_cellSize) * magnitude;
                left = true;
            } 
            // If the pixel is on the right half of the current cell and a cell exists to the right of the current cell, 
            // add the pixel's weighted contribution the appropriate bin in the right cell, and set the appropriate boolean
            else if ((i >= (cx+0.5)*_cellSize) && ((cx+1)*_cellSize < imgWidth)) {
                hogImg.Pixel(cx+1, cy, bin) += gaussWeight(i, j, (cx+1.5)*_cellSize, (cy+0.5)*_cellSize) * magnitude;
                right = true;
            }

            // If the pixel is on the top half of the current cell and a cell exists above the current cell, add the pixel's
            // weighted contribution the appropriate bin in the top cell, and set the appropriate boolean
            if ((j < (cy+0.5)*_cellSize) && (cy-1 > 0)) {
                hogImg.Pixel(cx, cy-1, bin) += gaussWeight(i, j, (cx+0.5)*_cellSize, (cy-0.5)*_cellSize) * magnitude;
                up = true;
            } 
            // If the pixel is on the lower half of the current cell and a cell exists below the current cell, add the pixel's
            // weighted contribution the appropriate bin in the bottom cell, and set the appropriate boolean
            else if ((j >= (cy+0.5)*_cellSize) && ((cy+1)*_cellSize < imgHeight)) {
                hogImg.Pixel(cx, cy+1, bin) += gaussWeight(i, j, (cx+0.5)*_cellSize, (cy+1.5)*_cellSize) * magnitude;
                down = true;
            }

            if (up) {
                if (left) {
                    // If the pixel contributes to a left and top cell, it also contributes to a diagonal top-left cell
                    hogImg.Pixel(cx-1, cy-1, bin) += gaussWeight(i, j, (cx-0.5)*_cellSize, (cy-0.5)*_cellSize) * magnitude;
                } 
                else if (right) {
                    // If the pixel contributes to a right and top cell, it also contributes to a diagonal top-right cell
                    hogImg.Pixel(cx+1, cy-1, bin) += gaussWeight(i, j, (cx+1.5)*_cellSize, (cy-0.5)*_cellSize) * magnitude;
                }
            }
            else if (down) {
                if (left) {
                    // If the pixel contributes to a left and bottom cell, it also contributes to a diagonal bottom-left cell
                    hogImg.Pixel(cx-1, cy+1, bin) += gaussWeight(i, j, (cx-0.5)*_cellSize, (cy+1.5)*_cellSize) * magnitude;
                } 
                else if (right) {
                    // If the pixel contributes to a right and bottom cell, it also contributes to a diagonal bottom-right cell
                    hogImg.Pixel(cx+1, cy+1, bin) += gaussWeight(i, j, (cx+1.5)*_cellSize, (cy+1.5)*_cellSize) * magnitude;
                }
            }
        }
    }

    // Normalize the histograms in each cell to have a total area of 1
    double area = 0;
    for (int j=0; j<targetH; j++) {
        for (int i=0; i<targetW; i++) {
            area = 0;
            for (int b=0; b<_nAngularBins; b++) {
                area += (2*PI/_nAngularBins) * hogImg.Pixel(i, j, b);
            }

            for (int b=0; b<_nAngularBins; b++) {
                hogImg.Pixel(i, j, b) = (area != 0) ? hogImg.Pixel(i, j, b)/area : 0;
            }
        }
    }

    // Return feature
    feat = hogImg;
}

/**
* Helper function to return a gaussian weight based on the distance between a pixel and the 
* center of a cell, where the pixel coordinates are (px, py), and the cell center coordinates 
* are (cx, cy)
*/
double
HOGFeatureExtractor::gaussWeight(double px, double py, double cx, double cy) const 
{
    double distance = sqrt(pow(px-cx,2) + pow(py-cy, 2));

    return (1/(_cellSize*sqrt(2*PI)))*exp(-0.5*(pow(distance/_cellSize,2)));
}

CByteImage
HOGFeatureExtractor::render(const Feature &f) const
{
    CShape cellShape = _oriMarkers[0].Shape();
    CFloatImage hogImgF(CShape(cellShape.width * f.Shape().width, cellShape.height * f.Shape().height, 1));
    hogImgF.ClearPixels();

    float minBinValue, maxBinValue;
    f.getRangeOfValues(minBinValue, maxBinValue);

    // For every cell in the HOG
    for(int hi = 0; hi < f.Shape().height; hi++) {
        for(int hj = 0; hj < f.Shape().width; hj++) {

            // Now _oriMarkers, multiplying contribution by bin level
            for(int hc = 0; hc < _nAngularBins; hc++) {
                float v = f.Pixel(hj, hi, hc) / maxBinValue;
                for(int ci = 0; ci < cellShape.height; ci++) {
                    float *cellIt = (float *) _oriMarkers[hc].PixelAddress(0, ci, 0);
                    float *hogIt = (float *) hogImgF.PixelAddress(hj * cellShape.height, hi * cellShape.height + ci, 0);

                    for(int cj = 0; cj < cellShape.width; cj++, hogIt++, cellIt++) {
                        (*hogIt) += v * (*cellIt);
                    }
                }
            }

        }
    }

    CByteImage hogImg;
    TypeConvert(hogImgF, hogImg);
    return hogImg;
}


