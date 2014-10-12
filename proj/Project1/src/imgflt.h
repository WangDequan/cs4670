#ifndef IMGFLT_H
#define IMGFLT_H

#include <math.h>
#include <stdio.h>
#include <string.h>

//#include <strstrea.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/fl_message.H>

#include "ImageLib/Image.h"
#include "ImageLib/FileIO.h"

class ImgFilterUI;
class FltDesignUI;
class BrushConfigUI;
class HelpPageUI;
class ScissorPanelUI;

class ImgView;

struct Node;
struct Seed;

#include "PriorityQueue.h"

#include "ImgFilterUI.h"
#include "FltDesignUI.h"
#include "BrushConfigUI.h"
#include "HelpPageUI.h"
#include "ScissorPanelUI.h"

#include "ImgView.h"

#include "correlation.h"

#include "ImgFltAux.h"

#include "iScissor.h"

void my_img_filter(char helpInfo[2048]);

#endif
