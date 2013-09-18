#include "imgflt.h"

int main(int argc, char** argv)
{
    Fl::visual(FL_DOUBLE | FL_RGB);

    ImgFilterUI imgFltUI;

    // Added by Loren
    if(argc > 1)
        imgFltUI.imgView->OpenImage(argv[1]);

    imgFltUI.show();
    return Fl::run();
}


void my_img_filter(char helpInfo[2048])
{
    char info[2048] = "\
Commands:\n\
\n\
File-->Save Contour\n\
Save image with contour marked.\n\
\n\
File-->Save Mask\n\
Save compositing mask for PhotoShop.\n\
\n\
Tool-->Scissor\n\
Open a panel to choose what to draw in the window.\n\
\n\
In this panel, choose your work mode: \n\
Image Only:\n\
Show original image without contour superimposed on it; \n\
\n\
Image with Contour: show original image with contours superimposed on it; \n\
\n\
Debug Mode: \n\
Pixel Node:\n\
Draw a cost graph with original image pixel colors at the center \n\
of each 3-by-3 window, and black everywhere else; \n\
\n\
Cost Graph:\n\
Draw a cost graph with both pixel colors and link costs, where you \n\
can see whether your cost computation is reasonable or not, e.g., low cost \n\
(dark intensity) for links along image edges. \n\
\n\
Path Tree:\n\
Show minimum path tree in the cost graph for the current seed; You\n\
can use the counter widget to simulate how the tree is computed by specifying \n\
the number of expanded nodes. The tree consists of links with yellow color. The \n\
back track direction (towards the seed) goes from light yellow to dark yellow. \n\
Min Path: show the minimum path between the current seed and the mouse position; \n\
\n\
\nOperations for images and contours:\n\
\n\
\"+\", zoom in; \n\
\"-\", zoom out; \n\
Left click, select a seed; \n\
Enter, finish and close the current contour; \n\
Backspace, when scissoring, delete the last seed; otherwise, delete selected contour. \n\
Select a contour by moving onto it. Selected contour is red, un-selected ones are green.\n\
Crtl + Left Click + Drag, use a brush \n\
Right Click + Drag, move the image\n\
\n\
				GOOD LUCK!\n\
";

    strcpy(helpInfo, info);
}


