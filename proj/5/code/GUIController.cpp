#include "GUIController.h"
#include "GUIModel.h"

// ============================================================================
// Utils
// ============================================================================

std::string
chooseFile(const std::string &title, const std::string &pattern,
           int dialogType)
{
    // TODO: make dialog ask for filename
    // const char *fname = fl_file_chooser("Open SVM Model", "*.{dets}", NULL);

    Fl_Native_File_Chooser fchooser;
    fchooser.title(title.c_str());
    fchooser.type(dialogType);
    fchooser.filter(pattern.c_str());
    switch ( fchooser.show() ) {
    case -1:
        return "";
        break;  // ERROR
    case  1:
        return "";
        break;  // CANCEL
    default:
        return fchooser.filename();
        break;  // FILE CHOSEN
    }
}

// ============================================================================
// GUIController
// ============================================================================

GUIController::GUIController():
    _mainWindow(NULL),
    _menuBar(NULL),
    _model(NULL),
    _inputImageView(NULL),
    _imPyrView(NULL),
    _featView(NULL),
    _svmRespView(NULL),
    _nmsView(NULL),
    _pyrLevelCounter(NULL),
    _featureParams(NULL),
    _svmParams(NULL),
    _nmsParams(NULL),
    _svmDotProdView(NULL),
    _svmWeightsView(NULL),
    _helpMessage(NULL)
{
    //Fl::visual(FL_DOUBLE | FL_INDEX);

    // Create the main window.
    _mainWindow = new Fl_Window(1400, 800, "OBDet");
    {
        _mainWindow->user_data((void *) this);

        // Menu Bar
        int menuHeight = 25;
        _menuBar = new Fl_Menu_Bar(0, 0, _mainWindow->w(), menuHeight);
        {
            // FILE
            _menuBar->add("File/Open &Image..."    , FL_CTRL + 'o', (Fl_Callback *)GUIController::_cbOpenImage);
            _menuBar->add("File/Open &SVM Model...", FL_CTRL + 'v', (Fl_Callback *)GUIController::_cbOpenSVMModel);
            _menuBar->add("File/Save Parameters...", FL_CTRL + 'p', (Fl_Callback *)GUIController::_cbSaveParameters);
            _menuBar->add("File/Save Detections...", FL_CTRL + 'd', (Fl_Callback *)GUIController::_cbSaveDetections);
            _menuBar->add("File/&Quit"             , FL_CTRL + 'q', (Fl_Callback *)GUIController::_cbQuit);

            // FEATURE
            _featMenuIndex["ti"] = _menuBar->add("Feature/Tiny Image", "", (Fl_Callback *)GUIController::_cbSelectFeatureType, (void *)"ti", FL_MENU_RADIO | FL_MENU_VALUE);
            _featMenuIndex["tig"] = _menuBar->add("Feature/Tiny Image Gradient Magnitude", "", (Fl_Callback *)GUIController::_cbSelectFeatureType, (void *)"tig", FL_MENU_RADIO);
            _featMenuIndex["hog"] = _menuBar->add("Feature/HOG", "", (Fl_Callback *)GUIController::_cbSelectFeatureType, (void *)"hog", FL_MENU_RADIO);
            _featMenuIndex["custom1"] = _menuBar->add("Feature/Custom 1", "", (Fl_Callback *)GUIController::_cbSelectFeatureType, (void *)"custom1", FL_MENU_RADIO);
            _featMenuIndex["custom2"] = _menuBar->add("Feature/Custom 2", "", (Fl_Callback *)GUIController::_cbSelectFeatureType, (void *)"custom2", FL_MENU_RADIO);
            _featMenuIndex["custom3"] = _menuBar->add("Feature/Custom 3", "", (Fl_Callback *)GUIController::_cbSelectFeatureType, (void *)"custom3", FL_MENU_RADIO);

            // SVM
            _menuBar->add("SVM/Show SVM weights", "", (Fl_Callback *)GUIController::_cbShowSVMWeights);

            // HELP
            _menuBar->add("Help/Commands", "", (Fl_Callback *)GUIController::_cbShowHelp);
        }

        // border size
        const int b = DEFAULT_BORDER;
        const int controlsWidth = 300;
        const int toolbarHeight = 40;


        // Toolbox
        _pyrLevelCounter = new Fl_Simple_Counter(b, _menuBar->h() + b, 100, toolbarHeight - 2 * b);

        _pyrLevelCounter->tooltip("Selects which pyramid level to display");
        _pyrLevelCounter->step(1);
        _pyrLevelCounter->minimum(0);
        _pyrLevelCounter->deactivate();
        _pyrLevelCounter->callback(GUIController::_cbSelectedPyramidLevelUpdated, this);

        // Tabs
        _tabs = new Fl_Tabs(b, _pyrLevelCounter->h() + _pyrLevelCounter->y() + b,
                            _mainWindow->w() - 2 * b, _mainWindow->h() - _pyrLevelCounter->h() - _pyrLevelCounter->y() - 2 * b, "");
        int tabX, tabY, tabW, tabH;
        int viewX, viewY, viewW, viewH;
        _tabs->client_area(tabX, tabY, tabW, tabH, 0);
        {
            // Input image
            Fl_Group *inputImgGrp = new Fl_Group(tabX, tabY, tabW, tabH, "Input Image");
            viewX = inputImgGrp->x() + b + controlsWidth;
            viewY = inputImgGrp->y() + b;
            viewW = inputImgGrp->w() - 2 * b - controlsWidth;
            viewH = inputImgGrp->h() - 2 * b;
            {
                _inputImageView = new ImageViewGL(viewX, viewY, viewW, viewH);

            }
            inputImgGrp->end();

            // Image Pyramid
            Fl_Group *imPyrGrp = new Fl_Group(inputImgGrp->x(), inputImgGrp->y(), inputImgGrp->w(), inputImgGrp->h(), "Image Pyramid");
            {
                _imPyrView = new ImageViewGL(viewX, viewY, viewW, viewH);
                _pyrParams = new ControlsBox(inputImgGrp->x() + b, inputImgGrp->y() + b,
                                             controlsWidth - b, inputImgGrp->h() - 2 * b, "",
                                             "Build Pyramid", GUIController::_cbComputePyramid, this);

                _pyrParams->setFields(SBPyramid<float>::getDefaultParameters());
            }
            imPyrGrp->end();

            // Computed feature
            Fl_Group *featGrp = new Fl_Group(inputImgGrp->x(), inputImgGrp->y(), inputImgGrp->w(), inputImgGrp->h(), "Computed Feature");
            {
                _featView = new ImageViewGL(viewX, viewY, viewW, viewH);
                _featureParams = new ControlsBox(inputImgGrp->x() + b, inputImgGrp->y() + b,
                                                 controlsWidth - b, inputImgGrp->h() - 2 * b, "",
                                                 "Compute Feature", GUIController::_cbComputeFeature, this);

            }
            featGrp->end();

            // SVM Response
            Fl_Group *svmRespGrp = new Fl_Group(inputImgGrp->x(), inputImgGrp->y(), inputImgGrp->w(), inputImgGrp->h(), "SVM Response");
            {
                _svmRespView = new ImageViewGL(viewX, viewY, viewW, viewH);
                _svmRespView->setClickCallback(GUIController::_cbSVMRespCallbback, this);
                _svmParams = new ControlsBox(inputImgGrp->x() + b, inputImgGrp->y() + b,
                                             controlsWidth - b, inputImgGrp->h() - 2 * b, "",
                                             "Compute SVM Response", GUIController::_cbComputeSVMResponse, this);

            }
            svmRespGrp->end();

            // Final Detections
            Fl_Group *detsGrp = new Fl_Group(inputImgGrp->x(), inputImgGrp->y(), inputImgGrp->w(), inputImgGrp->h(), "Detections");
            {
                _nmsView = new ImageViewGL(viewX, viewY, viewW, viewH);
                _nmsParams = new ControlsBox(inputImgGrp->x() + b, inputImgGrp->y() + b,
                                             controlsWidth - b, inputImgGrp->h() - 2 * b, "",
                                             "Non Maxima Suppression", GUIController::_cbNonMaximaSuppression, this);

                _nmsParams->setFields(ObjectDetector::getDefaultParameters());

            }
            detsGrp->end();

        }
        _tabs->end();
        Fl_Group::current()->resizable(_tabs);
    }
    _mainWindow->end();
}

// http://goo.gl/fCyqrQ

GUIController *
GUIController::_whoAmI(Fl_Widget *o)
{
    return (GUIController *)(o->parent()->user_data());
}

void
GUIController::show()
{
    _mainWindow->show();
}

void
GUIController::refresh()
{
    _mainWindow->redraw();
}

void
GUIController::setModel(GUIModel *model)
{
    assert(_model == NULL);
    _model = model;
}

GUIModel *
GUIController::getModel()
{
    assert(_model != NULL);
    return _model;
}

void
GUIController::setInputImage(const CByteImage &img)
{
    _inputImageView->setImage(img);
}

void
GUIController::setImagePyramidImages(const std::vector<CFloatImage> &imgs)
{
    _imPyrView->setImages(imgs);
}

void
GUIController::setFeatureImages(const std::vector<CByteImage> &imgs)
{
    _featView->setImages(imgs);
}

void
GUIController::setSVMResponseImages(const std::vector<CFloatImage> &imgs)
{
    _svmRespView->setImages(imgs);
}

void
GUIController::setNMSImage(const CByteImage &img)
{
    _nmsView->setImage(img);
}

void
GUIController::setDetectionsImage(const CByteImage &img)
{
    _nmsView->setImage(img);
}

void
GUIController::resizeWindows(int width, int height)
{
    // _mainWindow->resize(_mainWindow->x(), _mainWindow->y(), width, height + 25);
    // _menuBar->resize(_menuBar->x(), _menuBar->y(), width, 25);
    // _inputImageView->resize(_inputImageView->x(), _inputImageView->y(), width, height);
}

void
GUIController::activatePyrControls(int nLevels)
{
    _pyrLevelCounter->maximum(nLevels - 1);
    _pyrLevelCounter->activate();
}

int
GUIController::getCurrentSelectedPyrLevel() const
{
    return _pyrLevelCounter->value();
}

void
GUIController::setFeatureParameters(const ParametersMap &params)
{
    _featureParams->setFields(params);
    _featureParams->redraw();
}

void
GUIController::deactivateFeatureParameterControls()
{
    _featureParams->deactivateFieldTable();
}

void
GUIController::setSelectedFeatureType(const std::string &featType)
{
    Fl_Menu_Item *m = (Fl_Menu_Item *) _menuBar->menu();
    assert(m);
    m[_featMenuIndex[featType]].setonly();
}

void
GUIController::deactivateFeatureMenu()
{
    Fl_Menu_Item *m = (Fl_Menu_Item *) _menuBar->menu();
    assert(m);
    for(std::map<std::string, int>::iterator i = _featMenuIndex.begin(); i != _featMenuIndex.end(); i++) {
        m[i->second].deactivate();
    }
}

void
GUIController::showSVMWeights(const CByteImage &img)
{
    if(_svmWeightsView == NULL) {
        _svmWeightsView = new ImageViewGL(0, 0, img.Shape().width, img.Shape().height, "SVM Weights");
    }

    _svmWeightsView->show();
    _svmWeightsView->setImage(img);
}

void
GUIController::showSVMDotProd(const CByteImage &img)
{
    if(_svmDotProdView == NULL) {
        _svmDotProdView = new ImageViewGL(0, 0, img.Shape().width, img.Shape().height, "SVM Dot product");
    }

    _svmDotProdView->show();
    _svmDotProdView->setImage(img);
}

// ============================================================================
// Callbacks
// ============================================================================

void
GUIController::_cbOpenImage(Fl_Menu_ * menu, void *v)
{
    assert(_whoAmI(menu)->_model != NULL);
    try {
        std::string fname = chooseFile("Open Image",
                                       "*.{jpg,jpeg,tga}",
                                       Fl_Native_File_Chooser::BROWSE_FILE);

        if(fname.size()) _whoAmI(menu)->getModel()->loadImage(fname);
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbSelectFeatureType(Fl_Menu_ * menu, void *data)
{
    try {
        const char *ftype = (const char *) data;
        _whoAmI(menu)->_model->selectFeatureType(ftype);
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbOpenSVMModel(Fl_Menu_ * menu, void *v)
{
    try {
        std::string fname = chooseFile("Load SVM Model",
                                       "*.{svm}",
                                       Fl_Native_File_Chooser::BROWSE_FILE);

        if(fname.size()) _whoAmI(menu)->getModel()->loadSVMModel(fname);
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbSaveDetections(Fl_Menu_ *menu, void *v)
{
    try {
        std::string fname = chooseFile("Save Detections",
                                       "Detections *.{dets}",
                                       Fl_Native_File_Chooser::BROWSE_SAVE_FILE);

        if(fname.size()) _whoAmI(menu)->_model->saveDetections(fname);
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbSaveParameters(Fl_Menu_ *menu, void *v)
{
    try {
        std::string fname = chooseFile("Save Parameters",
                                       "Params file\t*.params",
                                       Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
        if(fname.size()) _whoAmI(menu)->_model->saveParameters(fname);
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbQuit(Fl_Menu_ *menu, void *v)
{
    try {
        _whoAmI(menu)->_mainWindow->hide();
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbShowSVMWeights(Fl_Widget *menu, void *data)
{
    try {
        _whoAmI(menu)->getModel()->showSVMWeights();
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbShowHelp(Fl_Menu_ *menu, void *v)
{
    const char *helpMessage =
        "<H1>Image Views</H1>"
        "<UL>"
        "   <LI>Click and drag to translate</LI>"
        "   <LI>Shift + Click and drag to zoom</LI>"
        "   <LI>Hit the \"m\" key to reset the view</LI>"
        "</UL>"
        "<H1>SVM Response</H1>"
        "<UL>"
        "   <LI>Click on the response image while holding the OPTION key to "
        "       visualize the element wise multiplication of the SVM weights "
        "       and the feature centered at the point you clicked on</LI>"
        "</UL>"
        ;

    try {
        GUIController *me = _whoAmI(menu);

        if(me->_helpMessage == NULL) {
            me->_helpMessage = new Fl_Help_Dialog();
            me->_helpMessage->value(helpMessage);
        }

        me->_helpMessage->show();
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbComputePyramid(Fl_Widget *w, void *data)
{
    try {
        GUIController *me = (GUIController *) data;
        me->getModel()->computeImagePyramid(me->_pyrParams->getFieldValues());
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbComputeFeature(Fl_Widget *w, void *data)
{
    try {
        GUIController *me = (GUIController *) data;
        me->getModel()->computeFeature(me->_featureParams->getFieldValues());
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbComputeSVMResponse(Fl_Widget *w, void *data)
{
    try {
        GUIController *me = (GUIController *) data;
        me->getModel()->computeSVMResponse(me->_svmParams->getFieldValues());
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbSelectedPyramidLevelUpdated(Fl_Widget *w, void *data)
{
    try {
        GUIController *me = (GUIController *) data;
        int idx = ((Fl_Counter *)w)->value();
        me->_imPyrView->setCurrentImageIndex(idx);
        me->_featView->setCurrentImageIndex(idx);
        me->_svmRespView->setCurrentImageIndex(idx);
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbNonMaximaSuppression(Fl_Widget *w, void *data)
{
    try {
        GUIController *me = (GUIController *) data;
        me->getModel()->nonMaximaSuppression(me->_nmsParams->getFieldValues());
    } catch(CError &err) {
        fl_alert(err.message);
    }
}

void
GUIController::_cbSVMRespCallbback(ImageViewGL *view, int level, double x, double y, void *data)
{
    try {
        GUIController *me = (GUIController *) data;
        me->getModel()->showSVMDotProdAtPosition(level, x, y);
    } catch(CError &err) {
        fl_alert(err.message);
    }
}
