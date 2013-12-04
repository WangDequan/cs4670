#include "GUIModel.h"
#include "GUIController.h"

GUIModel::GUIModel():
    _featExtractor(NULL),
    _imPyr(NULL),
    _detector(NULL)
{
}

GUIModel::~GUIModel()
{
    if(_featExtractor != NULL) {
        delete _featExtractor;
        _featExtractor = NULL;
    }

    if(_imPyr != NULL) {
        delete _imPyr;
        _imPyr = NULL;
    }

    if(_detector == NULL) {
        delete _detector;
        _detector = NULL;
    }
}

void
GUIModel::loadSVMModel(const std::string &fname)
{
    if(_featExtractor != NULL) {
        delete _featExtractor;
        _featExtractor = NULL;
    }
    loadFromFile(fname, _svm, &_featExtractor);
    _featureType = _featExtractor->getFeatureType();

    _ui->setSelectedFeatureType(_featureType);
    _ui->deactivateFeatureMenu();
    _ui->setFeatureParameters(_featExtractor->getParameters());
    _ui->deactivateFeatureParameterControls();
}

void
GUIModel::setController(GUIController *controller)
{
    _ui = controller;
}

void
GUIModel::loadImage(const std::string &fname)
{
    ReadFile(_inputImage, fname.c_str());
    _ui->setInputImage(_inputImage);
    _ui->refresh();
}

void
GUIModel::saveDetections(const std::string &fname)
{
    saveToFile(fname, _detections);
}

void
GUIModel::saveParameters(const std::string &fname)
{
    std::map<std::string, ParametersMap> allParams;
    if(_detector != NULL) allParams[OBJECT_DETECTOR_KEY] = _detector->getParameters();
    if(_imPyr != NULL) allParams[IMAGE_PYRAMID_KEY] = _imPyr->getParameters();
    if(_featExtractor != NULL) {
        allParams[FEATURE_EXTRACTOR_KEY] = _featExtractor->getParameters();
        allParams[FEATURE_EXTRACTOR_KEY][FEATURE_TYPE_KEY] = _featureType;
    }

    saveToFile(fname, allParams);
}

void
GUIModel::computeImagePyramid(const ParametersMap &params)
{
    if(_inputImage.Shape().width == 0) {
        throw CError("Load an image in order to compute the image pyramid");
    }

    if(_imPyr != NULL) {
        delete _imPyr;
        _imPyr = NULL;
    }

    CFloatImage tmp;
    TypeConvert(_inputImage, tmp);

    _imPyr = new SBFloatPyramid(tmp, params);

    _ui->activatePyrControls(_imPyr->getNLevels());
    _ui->setImagePyramidImages(*_imPyr);
}

void
GUIModel::selectFeatureType(const std::string &featType)
{
    if(featType != _featureType) {
        _featureType = featType;
        _ui->setFeatureParameters(FeatureExtractor::getDefaultParameters(featType));
        _featParams = ParametersMap();
    }
}

void
GUIModel::computeFeature(const ParametersMap &params)
{
    if(_imPyr == NULL || _imPyr->getNLevels() == 0)
        throw CError("You must compute the image pyramid before computing features");
    if(_featureType.size() == 0)
        throw CError("You must select a feature type before computing features");

    // TODO: check if user loaded an image
    if(_featParams != params || _featExtractor == NULL) {
        _featParams = params;
        if(_featExtractor != NULL) {
            delete _featExtractor;
            _featExtractor = NULL;
        }
        _featExtractor = FeatureExtractor::create(_featureType, params);
    }

    (*_featExtractor)(*_imPyr, _featPyr);

    //int level = _ui->getCurrentSelectedPyrLevel();
    std::vector<CByteImage> featViz = _featExtractor->render(_featPyr, true);
    _ui->setFeatureImages(featViz);
}

void
GUIModel::computeSVMResponse(const ParametersMap &params)
{
    if(!_svm.initialized())
        throw CError("No SVM model was loaded");

    _svm.predictSlidingWindow(_featPyr, _svmRespPyr);
    _ui->setSVMResponseImages(_svmRespPyr);
}

void
GUIModel::nonMaximaSuppression(const ParametersMap &params)
{
    if(!_svm.initialized())
        throw CError("No SVM model was loaded");

    if(_nmsParams != params || _detector == NULL) {
        _nmsParams = params;
        if(_detector != NULL) {
            delete _detector;
            _detector = NULL;
        }

        _detector = new ObjectDetector(_nmsParams);
    }

    (*_detector)(_svmRespPyr, _svm.getROISize(), _featExtractor->scaleFactor(), _detections);

    CByteImage img;
    CopyPixels(_inputImage, img);
    drawDetections(img, _detections);

    _ui->setDetectionsImage(img);
}

void
GUIModel::showSVMWeights()
{
    if(!_svm.initialized())
        throw CError("No SVM model was loaded");

    if(_featExtractor == NULL)
        throw CError("No feature extractor present");

    CByteImage svmViz = _svm.renderSVMWeights(_featExtractor);
    _ui->showSVMWeights(svmViz);
}

void
GUIModel::showSVMDotProdAtPosition(int level, double x, double y)
{
    if(!_svm.initialized())
        throw CError("No SVM model was loaded");

    if(_featExtractor == NULL)
        throw CError("No feature extractor present");

    CShape featSupport = _svm.getFeatureSupportShape();

    const CFloatImage &feat = _featPyr[level];

    Feature svmWeights = _svm.getWeights();

    Feature featRoi(CShape(featSupport.width, featSupport.height, feat.Shape().nBands));
    featRoi.ClearPixels();
    int nBands = feat.Shape().nBands;
    for (int i = 0; i < featSupport.height; i++) {

        const float *src = (const float *) feat.PixelAddress(x + (-featSupport.width / 2), i + y + (-featSupport.height / 2), 0);
        float *dst = (float *) featRoi.PixelAddress(0, i, 0);
        const float *svmW = (const float *) svmWeights.PixelAddress(0, i, 0);

        for (int j = 0; j < featSupport.width; j++, svmW += nBands, dst += nBands, src += nBands) {

            if(!feat.Shape().InBounds(j + x + (-featSupport.width / 2), i + y + (-featSupport.height / 2))) continue;

            for (int k = 0; k < nBands; k++) {
                dst[k] = src[k] * svmW[k];
            }
        }
    }

    CByteImage featRoiViz = _featExtractor->renderPosNegComponents(featRoi);
    _ui->showSVMDotProd(featRoiViz);
}
