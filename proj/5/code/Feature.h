#ifndef FEATURE_H
#define FEATURE_H

#include "Common.h"
#include "Utils.h"
#include "CroppedImageDatabase.h"
#include "SubBandImagePyramid.h"

typedef CFloatImage Feature;
typedef SBFloatPyramid FeaturePyramid;
typedef std::vector<Feature> FeatureCollection;

// Abstract super class for all feature extractors.
class FeatureExtractor
{
public:
    // TODO: document this
    FeatureExtractor(const ParametersMap &params = ParametersMap()) {};
    virtual ~FeatureExtractor() {};
    virtual ParametersMap getParameters() const = 0;
    virtual std::string getFeatureType() const = 0;

    // Extract feature vector for image image. Decending classes must implement this method
    void operator()(const CByteImage &image, Feature &feat) const;

    // Extract feature vector for image image. Decending classes must implement this method
    virtual void operator()(const CFloatImage &image, Feature &feat) const = 0;

    // Extracts descriptor for each image in the database, stores result in FeatureCollection,
    // this is used for training the support vector machine.
    void operator()(const CroppedImageDatabase &db, FeatureCollection &FeatureCollection) const;

    // Extracts descriptor for each level of imPyr and stores the results in featPyr
    void operator()(const SBFloatPyramid &imPyr, FeaturePyramid &featPyr) const;

    // Generate a visualization for the feature f, for debuging and inspection purposes
    virtual CByteImage render(const Feature &f) const = 0;

    // Ratio of input image size to output response size (necessary when computing location of detections)
    virtual double scaleFactor() const = 0;

    // Same as render(f) but normalizes values to be in range (0,1) by dividing by max value
    CByteImage render(const Feature &f, bool normalizeFeat) const;
    std::vector<CByteImage> render(const FeaturePyramid &f, bool normalizeFeat) const;

    CByteImage renderPosNegComponents(const Feature &feat) const;

    // Factory method that allocates the correct feature vector extractor given
    // the name of the extractor (caller is responsible for deallocating
    // extractor). To extend the code with other feature extractors or
    // other configurations for feature extractors add appropriate constructor
    // calls in implementation of this function.
    static FeatureExtractor *create(ParametersMap params = ParametersMap());
    static FeatureExtractor *create(const std::string &featureType, const ParametersMap &params = ParametersMap());

    static void save(FILE *f, const FeatureExtractor *feat);
    static FeatureExtractor *load(FILE *f);

    // TODO document this
    static ParametersMap getDefaultParameters(const std::string &featureType);
};

// Tiny Image feature. Converts image to grayscale and downscales it. Used
// mostly as a baseline feature.
class TinyImageFeatureExtractor : public FeatureExtractor
{
private:
    double _scale; // by how much should the input image be scaled (should be < 1)

public:
    std::string getFeatureType() const { return "ti"; };

    static ParametersMap getDefaultParameters();

    //TinyImageFeatureExtractor(double scale = 1.0 / 5.0);
    TinyImageFeatureExtractor(const ParametersMap &params = ParametersMap());
    ParametersMap getParameters() const;

    void operator()(const CFloatImage &image, Feature &feat) const;

    CByteImage render(const Feature &f) const;

    double scaleFactor() const { return _scale; }
};

// Tiny Image Gradient feature.
class TinyImageGradFeatureExtractor : public FeatureExtractor
{
private:
    double _scale;                        // By how much should the input image be scaled (should be < 1)
    CFloatImage _kernelDx, _kernelDy;     // Derivative kernels in x and y directions

public:
    std::string getFeatureType() const { return "tig"; };

    static ParametersMap getDefaultParameters();
    ParametersMap getParameters() const;

    //TinyImageGradFeatureExtractor(double scale = 1.0 / 5.0);
    TinyImageGradFeatureExtractor(const ParametersMap &params = ParametersMap());
    void operator()(const CFloatImage &image, Feature &feat) const;

    CByteImage render(const Feature &f) const;

    double scaleFactor() const { return _scale; }
};

// Histogram of Oriented Gradients feature.
class HOGFeatureExtractor : public FeatureExtractor
{
private:
    int _nAngularBins;                    // Number of angular bins
    bool _unsignedGradients;              // If true then we only consider the orientation modulo 180 degrees (i.e., 190
    // degrees is considered the same as 10 degrees)
    int _cellSize;                        // Support size of a cell, in pixels
    CFloatImage _kernelDx, _kernelDy;     // Derivative kernels in x and y directions
    std::vector<CFloatImage> _oriMarkers; // Used for visualization

public:
    std::string getFeatureType() const { return "hog"; };

    static ParametersMap getDefaultParameters();
    ParametersMap getParameters() const;

    //HOGFeatureExtractor(int nAngularBins = 18, bool unsignedGradients = true, int cellSize = 6);
    HOGFeatureExtractor(const ParametersMap &params = ParametersMap());

    void operator()(const CFloatImage &image, Feature &feat) const;

    CByteImage render(const Feature &f) const;

    double scaleFactor() const { return 1.0 / double(_cellSize); }
};

#endif // FEATURE_H
