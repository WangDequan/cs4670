#ifndef GUICONTROLLER_H
#define GUICONTROLLER_H

#include "Common.h"
#include "SubBandImagePyramid.h"

// Gui related
#include "ImageView.h"
#include "ImageViewGL.h"
#include "ControlsBox.h"

// FLTK
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Simple_Counter.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Help_Dialog.H>

// Reference: http://www.fltk.org/doc-1.3/index.html

class GUIModel;

class GUIController
{
public:
    GUIController();

    void show();
    void refresh();
    void resizeWindows(int width, int height);

    void setModel(GUIModel *model);
    GUIModel *getModel();

    // Setters for the different views
    void setInputImage(const CByteImage &img);
    void setImagePyramidImages(const std::vector<CFloatImage> &imgs);
    void setFeatureImages(const std::vector<CByteImage> &imgs);
    void setSVMResponseImages(const std::vector<CFloatImage> &imgs);
    void setNMSImage(const CByteImage &img);
    void setDetectionsImage(const CByteImage &img);
    void showSVMWeights(const CByteImage &img);
    void showSVMDotProd(const CByteImage &img);

    void setSelectedFeatureType(const std::string &featType);
    void setFeatureParameters(const ParametersMap &params);
    void deactivateFeatureParameterControls();
    void deactivateFeatureMenu();

    // Methods used by the model object
    void activatePyrControls(int nLevels);
    int getCurrentSelectedPyrLevel() const;

private:
    // Return the UI, given a menu item
    static GUIController *_whoAmI(Fl_Widget *menu);

    // Callback functions
    static void _cbOpenImage(Fl_Menu_ *menu, void *v);
    static void _cbOpenSVMModel(Fl_Menu_ *menu, void *v);
    static void _cbSelectFeatureType(Fl_Menu_ *menu, void *v);
    static void _cbSaveDetections(Fl_Menu_ *menu, void *v);
    static void _cbSaveParameters(Fl_Menu_ *menu, void *v);
    static void _cbQuit(Fl_Menu_ *menu, void *v);
    static void _cbShowHelp(Fl_Menu_ *menu, void *v);
    static void _cbComputePyramid(Fl_Widget *, void *data);
    static void _cbComputeFeature(Fl_Widget *, void *data);
    static void _cbComputeSVMResponse(Fl_Widget *w, void *data);
    static void _cbSelectedPyramidLevelUpdated(Fl_Widget *, void *data);
    static void _cbNonMaximaSuppression(Fl_Widget *w, void *data);
    static void _cbShowSVMWeights(Fl_Widget *w, void *data);
    static void _cbSVMRespCallbback(ImageViewGL *view, int level, double x, double y, void *data);

    // Private members
    Fl_Window *_mainWindow;
    Fl_Menu_Bar *_menuBar;
    Fl_Tabs *_tabs;

    // Mapping between feature type string and menu index
    std::map<std::string, int> _featMenuIndex;

    ImageViewGL *_inputImageView, *_imPyrView, *_featView, *_svmRespView, *_nmsView, *_svmDotProdView, *_svmWeightsView;
    ControlsBox *_pyrParams, *_featureParams, *_svmParams, *_nmsParams;

    Fl_Counter *_pyrLevelCounter;

    GUIModel *_model;

    Fl_Help_Dialog *_helpMessage;

};

#endif // GUICONTROLLER_H
