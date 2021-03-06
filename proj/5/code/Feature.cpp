#include "Feature.h"

static
double
_round(double d)
{
    return floor(d + 0.5);
}

// static int tinyCount = 0;
// static int tinyGradCount = 0;
// static int hogCount = 0;

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
    //printf("Done extracting from database\n");
}

void
FeatureExtractor::operator()(const SBFloatPyramid &imPyr, FeaturePyramid &featPyr) const
{
    featPyr.resize(imPyr.getNLevels());

    CByteImage x;
    for (int i = 0; i < imPyr.getNLevels(); i++) {
        this->operator()(imPyr[i], featPyr[i]);
    }
    //printf("Done extracting from pyramid\n");
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
    // printf("%d Extracting TinyImage features...\n", ++tinyCount);

    int targetW = _round(imgRGB.Shape().width * _scale);
    int targetH = _round(imgRGB.Shape().height * _scale);

    CFloatImage tinyImg(targetW, targetH, 1);

    CFloatImage imgG;
    convertRGB2GrayImage(imgRGB, imgG);

    CTransform3x3 s = CTransform3x3::Scale( 1. / _scale, 1. / _scale );

    WarpGlobal(imgG, tinyImg, s, eWarpInterpLinear);

    feat = tinyImg;

    // printf("%d Done extracting TinyImage features\n", tinyCount);
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

    /******** BEGIN TODO ********/
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

    // printf("%d Extracting TinyImageGrad features...\n", ++tinyGradCount);

    // Initialize output
    CFloatImage tinyImg(targetW, targetH, 1);

    // Generate grayscale image from RGB input
    CFloatImage imgG;
    convertRGB2GrayImage(imgRGB_, imgG);

    // Resize grayscale image
    CTransform3x3 s = CTransform3x3::Scale( 1. / _scale, 1. / _scale);
    WarpGlobal(imgG, tinyImg, s, eWarpInterpLinear);

    // Generate gradient images
    CFloatImage xGrad, yGrad;
    Convolve(tinyImg, xGrad, _kernelDx);
    Convolve(tinyImg, yGrad, _kernelDy);

    // Compute gradient magnitudes
    float dx, dy;
    for (int x = 0; x < targetW; x++) {
        for (int y = 0; y < targetH; y++) {
            dx = xGrad.Pixel(x, y, 0);
            dy = yGrad.Pixel(x, y, 0);
            tinyImg.Pixel(x, y, 0) = sqrt(dx * dx + dy * dy);
        }
    }

    // Convert output features to desired type
    TypeConvert(tinyImg, feat);

    // printf("%d Done extracting TinyImageGrad features\n", tinyGradCount);

    /******** END TODO ********/
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
	/******** BEGIN TODO ********/
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

    //printf("%d Extracting HOG features...\n", ++hogCount);

	CFloatImage imgG;
	convertRGB2GrayImage(img, imgG);

	CFloatImage xGrad, yGrad;
	Convolve(imgG, xGrad, _kernelDx);
	Convolve(imgG, yGrad, _kernelDy);

	int width = imgG.Shape().width;
	int height = imgG.Shape().height;

	// Overlap cells by half
	int nCellsX = 2 * width / _cellSize - 1;
	int nCellsY = 2 * height / _cellSize - 1;

	CFloatImage featImg(nCellsX, nCellsY, _nAngularBins);
	featImg.ClearPixels();

	CFloatImage gradMagImg(width, height, 1);
	CFloatImage gradOrImg(width, height, 1);

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			float dx = xGrad.Pixel(x, y, 0);
			float dy = yGrad.Pixel(x, y, 0);

			// Compute gradient magnitude
			float magnitude = sqrt(dx*dx + dy*dy);
			gradMagImg.Pixel(x, y, 0) = magnitude;

			// Compute gradient angle
			float angle = atan2(dy, dx);

			// If angle negative, set to positive equivalent
			angle = angle < 0 ? 2 * M_PI + angle : angle;

			// If unsigned, take mod wrt pi
			angle = _unsignedGradients ? fmod(angle, M_PI) : angle;

			gradOrImg.Pixel(x, y, 0) = angle;
		}
	}

	// Spread bins from 0 to Pi if unsigned, 0 to 2*Pi if signed
	float angleMax = _unsignedGradients ? M_PI : 2 * M_PI;
	float angBinWidth = angleMax / (float)_nAngularBins;

	// Compute histogram for each cell
	for (int cellX = 0; cellX < nCellsX; cellX++) {
		for (int cellY = 0; cellY < nCellsY; cellY++) {
			// Cell boundaries
			int minX = cellX * _cellSize / 2;
			int minY = cellY * _cellSize / 2;
			int maxX = minX + _cellSize;
			int maxY = minY + _cellSize;

			// Cell center
			float centerX = minX + _cellSize / 2.f;
			float centerY = minY + _cellSize / 2.f;

			// Add contribution for each pixel in the cell
			for (int x = minX; x <= maxX && x < width; x++) {
				for (int y = minY; y <= maxY && y < height; y++) {
					float magnitude = gradMagImg.Pixel(x, y, 0);
					float angle = gradOrImg.Pixel(x, y, 0);

					int angBin = (int) floor(angle / angBinWidth);

					float distX = abs(x - centerX);
					float distY = abs(y - centerY);

					float sigmaSq = _cellSize * _cellSize;
					float distance = sqrt(distX * distX + distY * distY);
					float gaussian = exp(-distance / (2 * sigmaSq)) / (2 * M_PI * sigmaSq);

					featImg.Pixel(cellX, cellY, angBin) += gaussian * magnitude;
				}
			}
		}
	}

    // Smoothing constant to prevent division by zero
	float eps = 0.1f;

    // Threshold for bin values as suggested on Wikipedia
	float thresh = 0.2f;

	// Normalize histogram bins
	for (int cellX = 0; cellX < nCellsX; cellX++) {
		for (int cellY = 0; cellY < nCellsY; cellY++) {
			// Use L2-norm with smoothing constant to normalize bins
			float binSum = eps * eps;
			for (int bin = 0; bin < _nAngularBins; bin++) {
				binSum += pow(featImg.Pixel(cellX, cellY, bin), 2);
			}

			// Threshold bins
			float threshSum = eps * eps;
			for (int bin = 0; bin < _nAngularBins; bin++) {
				float threshBinVal = min(thresh, featImg.Pixel(cellX, cellY, bin) / sqrt(binSum));
				featImg.Pixel(cellX, cellY, bin) = threshBinVal;
				threshSum += pow(threshBinVal, 2);
			}

			// Renormalize thresholded bins
			for (int bin = 0; bin < _nAngularBins; bin++) {
				featImg.Pixel(cellX, cellY, bin) /= sqrt(threshSum);
			}
		}
	}

	TypeConvert(featImg, feat);

    //printf("%d Done extracting HOG features\n", hogCount);

	/******** END TODO ********/
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


