#ifndef __GUIMODEL_H__
#define __GUIMODEL_H__

#include "Common.h"
#include "SupportVectorMachine.h"
#include "SubBandImagePyramid.h"
#include "ObjectDetector.h"
#include "Detection.h"
#include "FileIO.h"

#include <Fl/Fl_Shared_Image.H>

class GUIController;

class GUIModel
{
public:
    GUIModel();
    ~GUIModel();

    void setController(GUIController *controller);

    // IO
    void loadSVMModel(const std::string &fname);
    void loadImage(const std::string &fname);
    void saveDetections(const std::string &fname);
    void saveParameters(const std::string &fname);

    // Recomputes the image pyramid
    void computeImagePyramid(const ParametersMap &params);

    // Updates the level that is being displayed
    // void updateSelectedPyrarmidLevel(int level);

    void selectFeatureType(const std::string &featType);
    void computeFeature(const ParametersMap &params);
    void computeSVMResponse(const ParametersMap &params);
    void nonMaximaSuppression(const ParametersMap &params);
    void showSVMWeights();
    void showSVMDotProdAtPosition(int level, double x, double y);

private:
    SupportVectorMachine _svm;

    // Input image
    CByteImage _inputImage;

    // Image Pyramid
    SBFloatPyramid *_imPyr;

    ObjectDetector *_detector;
    std::vector<Detection> _detections;
    ParametersMap _nmsParams;

    // Feature extraction
    FeatureExtractor *_featExtractor;
    std::string _featureType;
    ParametersMap _featParams;
    FeaturePyramid _featPyr;
    SBFloatPyramid _svmRespPyr;

    GUIController *_ui;
};

#endif // __GUIMODEL_H__