#include "SupportVectorMachine.h"

SupportVectorMachine::SupportVectorMachine():
    _model(NULL),
    _data(NULL)
{
}

SupportVectorMachine::SupportVectorMachine(const std::string &modelFName):
    _model(NULL),
    _data(NULL)
{
    load(modelFName);
}

void
SupportVectorMachine::_deinit()
{
    if(_model != NULL) svm_free_and_destroy_model(&_model);
    if(_data != NULL) delete [] _data;
    _model = NULL;
    _data = NULL;
}

SupportVectorMachine::~SupportVectorMachine()
{
    _deinit();
}

void
SupportVectorMachine::train(const std::vector<float> &labels, const FeatureCollection &fset, const Size &roiSize, double C)
{
    if(labels.size() != fset.size()) throw CError("Database size is different from feature set size!");

    _fVecShape = fset[0].Shape();
    if(roiSize.width != 0 && roiSize.height != 0) {
        _roiSize = roiSize;
    } else {
        _roiSize.width = _fVecShape.width;
        _roiSize.height = _fVecShape.height;
    }

    // Figure out size and number of feature vectors
    int nVecs = labels.size();
    CShape shape = fset[0].Shape();
    int dim = shape.width * shape.height * shape.nBands;

    // Parameters for SVM
    svm_parameter parameter;
    parameter.svm_type = C_SVC;
    parameter.kernel_type = LINEAR;
    parameter.degree = 0;
    parameter.gamma = 0;
    parameter.coef0 = 0;
    parameter.nu = 0.5;
    parameter.cache_size = 100;  // In MB
    parameter.C = C;
    parameter.eps = 1e-3;
    parameter.p = 0.1;
    parameter.shrinking = 1;
    parameter.probability = 0;
    parameter.nr_weight = 0; // ?
    parameter.weight_label = NULL;
    parameter.weight = NULL;
    //cross_validation = 0;

    // Allocate memory
    svm_problem problem;
    problem.l = nVecs;
    problem.y = new double[nVecs];
    problem.x = new svm_node*[nVecs];
    if(_data) delete [] _data;

    /******** BEGIN TODO ********/
    // Copy the data used for training the SVM into the libsvm data structures "problem".
    // Put the feature vectors in _data and labels in problem.y. Also, problem.x[k]
    // should point to the address in _data where the k-th feature vector starts (i.e.,
    // problem.x[k] = &_data[starting index of k-th feature])
    //
    // Hint:
    // * Don't forget to set _data[].index to the corresponding dimension in
    //   the original feature vector. You also need to set _data[].index to -1
    //   right after the last element of each feature vector

    // Vector containing all feature vectors. svm_node is a struct with
    // two fields, index and value. Index entry indicates position
    // in feature vector while value is the value in the original feature vector,
    // each feature vector of size k takes up k+1 svm_node's in _data
    // the last one being simply to indicate that the feature has ended by setting the index
    // entry to -1
    _data = new svm_node[nVecs * (dim + 1)];

printf("TODO: %s:%d\n", __FILE__, __LINE__); 

    /******** END TODO ********/

    // Train the model
    if(_model != NULL) svm_free_and_destroy_model(&_model);
    _model = svm_train(&problem, &parameter);

    // Cleanup
    delete [] problem.y;
    delete [] problem.x;
}

float
SupportVectorMachine::predict(const Feature &feature) const
{
    CShape shape = feature.Shape();
    int dim = shape.width * shape.height * shape.nBands;

    svm_node *svmNode = new svm_node[dim + 1];

    svm_node *svmNodeIter = svmNode;

    for(int y = 0, k = 0; y < shape.height; y++) {
        float *data = (float *) feature.PixelAddress(0, y, 0);
        for (int x = 0; x < shape.width * shape.nBands; x++, data++, k++, svmNodeIter++) {
            svmNodeIter->index = k;
            svmNodeIter->value = *data;
        }
    }
    svmNodeIter->index = -1;

    double decisionValue;
    float label = svm_predict_values(_model, svmNode, &decisionValue);

    delete [] svmNode;

    return decisionValue;
}

std::vector<float>
SupportVectorMachine::predict(const FeatureCollection &fset) const
{
    std::vector<float> preds(fset.size());
    for(int i = 0; i < fset.size(); i++) {
        preds[i] = predict(fset[i]);
    }

    return preds;
}

double
SupportVectorMachine::getBiasTerm() const
{
    if(_model == NULL)
        throw CError("Asking for SVM bias term but there is no "
                     "model. Either load one from file or train one before.");
    return _model->rho[0];
}

Feature
SupportVectorMachine::getWeights() const
{
    if(_model == NULL)
        throw CError("Asking for SVM weights but there is no model. Either "
                     "load one from file or train one before.");

    Feature weightVec(_fVecShape);
    weightVec.ClearPixels();

    weightVec.origin[0] = _fVecShape.width / 2;
    weightVec.origin[1] = _fVecShape.height / 2;

    int nSVs = _model->l; // number of support vectors

    for(int s = 0; s < nSVs; s++) {
        double coeff = _model->sv_coef[0][s];
        svm_node *sv = _model->SV[s];

        for(int y = 0, d = 0; y < _fVecShape.height; y++) {
            float *w = (float *) weightVec.PixelAddress(0, y, 0);
            for(int x = 0; x < _fVecShape.width * _fVecShape.nBands; x++, d++, w++, sv++) {
                assert(sv->index == d);
                *w += sv->value * coeff;
            }
        }
    }

    return weightVec;
}

void
SupportVectorMachine::load(const std::string &filename)
{
    FILE *f = fopen(filename.c_str(), "rb");
    if(f == NULL) throw CError("Failed to open file %s for reading", filename.c_str());
    this->load(f);
}

void
SupportVectorMachine::load(FILE *fp)
{
    _deinit();
    fscanf(fp, "%d %d %d", &_fVecShape.width, &_fVecShape.height, &_fVecShape.nBands);
    fscanf(fp, "%lf %lf", &_roiSize.width, &_roiSize.height);
    _model = svm_load_model_fp(fp);
    if(_model == NULL) {
        throw CError("Failed to load SVM model");
    }
}

void
SupportVectorMachine::save(FILE *fp) const
{
    if(_model == NULL) throw CError("No model to be saved");

    fprintf(fp, "%d %d %d\n", _fVecShape.width, _fVecShape.height, _fVecShape.nBands);
    fprintf(fp, "%lf %lf\n", _roiSize.width, _roiSize.height);

    if(svm_save_model_fp(fp, _model) != 0) {
        throw CError("Error while trying to write model to file");
    }
}

void
SupportVectorMachine::save(const std::string &filename) const
{
    FILE *fp = fopen(filename.c_str(), "wb");
    if(fp == NULL) {
        throw CError("Could not open file %s for writing.", filename.c_str());
    }

    save(fp);
    if (ferror(fp) != 0 || fclose(fp) != 0) {
        throw CError("Error while closing file %s", filename.c_str());
    }
}

void
SupportVectorMachine::predictSlidingWindow(const Feature &feat, CFloatImage &response) const
{
    response.ReAllocate(CShape(feat.Shape().width, feat.Shape().height, 1));
    response.ClearPixels();

    /******** BEGIN TODO ********/
    // Sliding window prediction.
    //
    // In this project we are using a linear SVM. This means that
    // it's classification function is very simple, consisting of a
    // dot product of the feature vector with a set of weights learned
    // during training, followed by a subtraction of a bias term
    //
    //          pred <- dot(feat, weights) - bias term
    //
    // Now this is very simple to compute when we are dealing with
    // cropped images, our computed features have the same dimensions
    // as the SVM weights. Things get a little more tricky when you
    // want to evaluate this function over all possible subwindows of
    // a larger feature, one that we would get by running our feature
    // extraction on an entire image.
    //
    // Here you will evaluate the above expression by breaking
    // the dot product into a series of convolutions (remember that
    // a convolution can be though of as a point wise dot product with
    // the convolution kernel), each one with a different band.
    //
    // Convolve each band of the SVM weights with the corresponding
    // band in feat, and add the resulting score image. The final
    // step is to subtract the SVM bias term given by this->getBiasTerm().
    //
    // Hint: you might need to set the origin for the convolution kernel
    // in order to get the result from convoltion to be correctly centered
    //
    // Useful functions:
    // Convolve, BandSelect, this->getWeights(), this->getBiasTerm()

printf("TODO: %s:%d\n", __FILE__, __LINE__); 

    /******** END TODO ********/
}

void
SupportVectorMachine::predictSlidingWindow(const FeaturePyramid &featPyr, SBFloatPyramid &responsePyr) const
{
    responsePyr.resize(featPyr.getNLevels());
    for (int i = 0; i < featPyr.getNLevels(); i++) {
        this->predictSlidingWindow(featPyr[i], responsePyr[i]);
    }
}

CByteImage
SupportVectorMachine::renderSVMWeights(const FeatureExtractor *featExtractor)
{
    Feature svmW = this->getWeights();
    svmW -= this->getBiasTerm() / (svmW.Shape().width * svmW.Shape().height * svmW.Shape().nBands);

    return featExtractor->renderPosNegComponents(svmW);
}

