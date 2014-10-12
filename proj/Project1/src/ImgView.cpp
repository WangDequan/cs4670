#include "ImgView.h"
#include "iScissor.h"

ImgView::ImgView(int x, int y, int w, int h, const char* label) : Fl_Double_Window(x, y, w, h, label)
{
    imgBuf = NULL;
    origImg = NULL;
    curImg = NULL;
    curImgChar = NULL;

    imgBuf = NULL;
    imgLeft = 0;
    imgTop = 0;

    fltDesignUI = new FltDesignUI;
    fltDesignUI->imgView = this;

    int i;
    for (i = 0; i < FLT_HEIGHT * FLT_WIDTH; i++) {
        fltKernel[i] = 0;
    }
    fltKernel[(FLT_HEIGHT / 2)*FLT_WIDTH + (FLT_HEIGHT / 2)] = 1;
    scale = 1;
    offset = 0;

    InitFltDesignUI();

    fltDesignUI->image->value(1);
    fltDesignUI->selection->value(0);

    scissorPanelUI = new ScissorPanelUI();
    scissorPanelUI->imgView = this;
    scissorPanelUI->expanded->range(0, 0);
    scissorPanelUI->expanded->value(0);
    scissorPanelUI->expanded->deactivate();

    brushSelection = NULL;
    brushType = ROUND_BRUSH;
    brushSize = 20;
    brushRadius2 = brushSize * brushSize;

    brushOpacity = 0.5;

    brushConfigUI = new BrushConfigUI;
    brushConfigUI->imgView = this;

    //brushConfigUI->round->value(1);
    //brushConfigUI->square->value(0);
    brushConfigUI->brushSize->range(1, 2 * brushSize);
    brushConfigUI->brushSize->value(brushSize);
    brushConfigUI->brushOpacity->value(brushOpacity);

    helpPageUI = new HelpPageUI();
    char helpInfo[2048];
    my_img_filter(helpInfo);
    helpPageUI->helpTextBuffer->text(helpInfo);
    helpPageUI->helpText->buffer(helpPageUI->helpTextBuffer);
    helpPageUI->mainWindow->resizable(helpPageUI->helpText);
}

void ImgView::FreeBuffer(void)
{
    if (imgBuf) {
        delete[] origImg;
        delete[] imgBuf;

        delete[] brushSelection;
        delete[] costGraph;
        delete[] pixelNodes;

        delete[] viewBuf;

        delete[] nodeBuf;

        origImg = NULL;
        imgBuf = NULL;

        brushSelection = NULL;
        costGraph = NULL;
        pixelNodes = NULL;

        viewBuf = NULL;

        nodeBuf = NULL;

        int i;
        for (i = 0; i < contours.GetSize(); i++) {
            contours[i]->FreePtrs();
        }
        contours.FreePtrs();
        // empty the list of null pointers, in case we load another image
        contours = CTypedPtrArray <CTypedPtrDblList <Seed> > ();

        currentCntr.FreePtrs();
    }
}

void ImgView::HideAll(void)
{
    fltDesignUI->hide();
    brushConfigUI->hide();
    scissorPanelUI->hide();
}

ImgView::~ImgView()
{
    FreeBuffer();

//	delete fltDesignUI;
    delete brushConfigUI;
    delete helpPageUI;
    delete scissorPanelUI;
}

void ImgView::OpenImage(void)
{
    char* filename = fl_file_chooser("choose a targa file", "*.tga", 0);

    if (filename) {
        OpenImage(filename);
    }
}

void ImgView::OpenImage(const char* filename)
{
    int i, j;

    FreeBuffer();

    CByteImage rick;

    // Fixed by Loren.
    try {
        ReadFile(rick, filename);
    } catch(CError err) {
        printf("%s\n", err.message);
    }

    imgWidth = rick.Shape().width;
    imgHeight = rick.Shape().height;

    graphWidth = imgWidth * 3;
    graphHeight = imgHeight * 3;

    int numNodes = imgWidth * imgHeight;
    int imgSize = imgWidth * imgHeight * 3;
    int graphSize = graphWidth * graphHeight * 3;

    brushSelection = new unsigned char[numNodes];
    memset(brushSelection, 0, numNodes * sizeof(unsigned char));

    origImg = new unsigned char [imgSize];
    curImg = new double [imgSize];
    curImgChar = new unsigned char [imgSize];



    imgBuf = new unsigned char [imgSize];
    costGraph = new unsigned char [graphSize];
    pixelNodes = new unsigned char [graphSize];

    for (j = 0; j < imgHeight; j++) {
        for (i = 0; i < imgWidth; i++) {
            int imgIndex = 3 * (j * imgWidth + i);
            int graphIndex = 3 * ((3 * j + 1) * graphWidth + (3 * i + 1));
            int x = i;
            int y = imgHeight - 1 - j;

            for (int c = 0; c < 3; c++) {
                //costGraph[graphIndex+c] =
                pixelNodes[graphIndex + c] =
                    imgBuf[imgIndex + c] = origImg[imgIndex + c] = rick.Pixel(x, y, 2 - c);

                pixelNodes[graphIndex + 3 * (+1)			+ c] =
                    pixelNodes[graphIndex + 3 * (-graphWidth + 1) + c] =
                        pixelNodes[graphIndex + 3 * (-graphWidth)	+ c] =
                            pixelNodes[graphIndex + 3 * (-graphWidth - 1) + c] =
                                pixelNodes[graphIndex + 3 * (-1)			+ c] =
                                    pixelNodes[graphIndex + 3 * (+graphWidth - 1) + c] =
                                        pixelNodes[graphIndex + 3 * (+graphWidth)	+ c] =
                                            pixelNodes[graphIndex + 3 * (+graphWidth + 1) + c] = 0;
            }
        }
    }

    for (int i = 0; i < imgSize; i++) {
        curImgChar[i] = origImg[i];
        curImg[i] = double(origImg[i]);
    }


    nodeBuf = new Node[numNodes]();

    InitNodeBuf(nodeBuf, imgBuf, imgWidth, imgHeight);
    MakeCostGraph(costGraph, nodeBuf, imgBuf, imgWidth, imgHeight);

    imgLeft = -imgWidth / 2;
    imgTop = -imgHeight / 2;

    zoomFactor = 1;

    AllocateViewBuffer(this->w(), this->h());

    drawMode = IMAGE_WITH_CONTOUR;
    scissorPanelUI->contour->value(1);
    scissorPanelUI->expanded->range(0, imgWidth * imgHeight);
    scissorPanelUI->expanded->lstep(100);
    scissorPanelUI->expanded->value(1000);
    scissorPanelUI->expanded->deactivate();

    brushSelPtr = NULL;
    scissorPanelUI->whole->value(1);

    UpdateViewBuffer();

    redraw();
}

void ImgView::SaveContour(void)
{
    char* filename = fl_file_chooser("choose a targa file", "*.tga", 0);

    if (filename) {
        SaveContour(filename);
    }
}

void ImgView::SaveContour(const char* filename)
{
    CByteImage rick(imgWidth, imgHeight, 3);

    int i, j;

    //our buffer is in R, G, B order from top to bottom and from left to right;
    //Rick's .tga file format is in B,G,R order from bottom to top and from left to right;
    for (j = 0; j < imgHeight; j++) {
        for (i = 0; i < imgWidth; i++) {
            int index = 3 * (j * imgWidth + i);
            int x = i;
            int y = imgHeight - 1 - j;
            rick.Pixel(x, y, 0) = origImg[index + 2];
            rick.Pixel(x, y, 1) = origImg[index + 1];
            rick.Pixel(x, y, 2) = origImg[index + 0];
        }
    }

    int k;

    for (k = 0; k < contours.GetSize(); k++) {
        CTypedPtrDblElement <Seed>* seedEle = contours[k]->GetHeadPtr();

        while (!contours[k]->IsSentinel(seedEle)) {
            Seed* seed = seedEle->Data();
            int x = seed->x;
            int y = imgHeight - 1 - seed->y;
            rick.Pixel(x, y, 0) = PATH_COLOR[2];
            rick.Pixel(x, y, 1) = PATH_COLOR[1];
            rick.Pixel(x, y, 2) = PATH_COLOR[0];

            seedEle = seedEle->Next();
        }
    }

    // Fixed by Loren.
    try {
        WriteFile(rick, filename);
    } catch(CError err) {
        printf("%s\n", err.message);
    }
}

void ImgView::SaveMask(void)
{
    char* filename = fl_file_chooser("choose a targa file", "*.tga", 0);

    if (filename) {
        SaveMask(filename);
    }

}

void ImgView::SaveMask(const char* filename)
{
    CByteImage rick(imgWidth, imgHeight, 3);

    unsigned char* maskBuf = new unsigned char [3 * imgWidth * imgHeight];
    memset(maskBuf, 0, 3 * imgWidth * imgHeight * sizeof(unsigned char));

    int i, j, k;

    unsigned char* cntrBuf = new unsigned char [3 * imgWidth * imgHeight];

    for (k = 0; k < contours.GetSize(); k++) {
        if (contours[k]->IsCircular()) {
            memset(cntrBuf, 0, 3 * imgWidth * imgHeight * sizeof(unsigned char));

            CTypedPtrDblElement <Seed>* seedEle = contours[k]->GetHeadPtr();

            Seed* seed = seedEle->Data();
            int lastRow = seed->y;
            int lastCol = seed->x;
            int bufIndex;

            seedEle = seedEle->Next();

            while (!contours[k]->IsSentinel(seedEle)) {
                seed = seedEle->Data();
                if (lastRow < seed->y) {
                    bufIndex = 3 * (lastRow * imgWidth + lastCol);
                    cntrBuf[bufIndex + 0]++;
                    cntrBuf[bufIndex + 1]++;
                    cntrBuf[bufIndex + 2]++;

                    lastRow = seed->y;
                    lastCol = seed->x;
                } else if (lastRow > seed->y) {
                    bufIndex = 3 * (seed->y * imgWidth + seed->x);
                    cntrBuf[bufIndex + 0]++;
                    cntrBuf[bufIndex + 1]++;
                    cntrBuf[bufIndex + 2]++;

                    lastRow = seed->y;
                    lastCol = seed->x;
                }

                seedEle = seedEle->Next();
            }

            for (j = 0; j < imgHeight; j++) {
                int count[3] = {0, 0, 0};

                for (i = 0; i < imgWidth; i++) {
                    int bufIndex = 3 * (j * imgWidth + i);

                    for (int c = 0; c < 3; c++) {
                        if (cntrBuf[bufIndex + c] == 0) {
                            if (count[c] % 2) {
                                maskBuf[bufIndex + c] = 255;
                            }
                        } else {
                            count[c] += cntrBuf[bufIndex + c];
                        }
                    }
                }
            }

            seedEle = contours[k]->GetHeadPtr();
            while (!contours[k]->IsSentinel(seedEle)) {
                seed = seedEle->Data();
                bufIndex = 3 * (seed->y * imgWidth + seed->x);
                maskBuf[bufIndex + 0] = maskBuf[bufIndex + 1] = maskBuf[bufIndex + 2] = 255;

                seedEle = seedEle->Next();
            }
        }
    }

    delete[] cntrBuf;

    //our buffer is in R, G, B order from top to bottom and from left to right;
    //Rick's .tga file format is in B,G,R order from bottom to top and from left to right;
    for (j = 0; j < imgHeight; j++) {
        for (i = 0; i < imgWidth; i++) {
            int index = 3 * (j * imgWidth + i);
            int x = i;
            int y = imgHeight - 1 - j;
            rick.Pixel(x, y, 0) = maskBuf[index + 2];
            rick.Pixel(x, y, 1) = maskBuf[index + 1];
            rick.Pixel(x, y, 2) = maskBuf[index + 0];
        }
    }

    delete[] maskBuf;

    // Fixed by Loren.
    try {
        WriteFile(rick, filename);
    } catch(CError err) {
        printf("%s\n", err.message);
    }
}

void ImgView::TryFilter(void)
{
    fltDesignUI->show();
    //cout<<"try filter!"<<endl;
}

void ImgView::LoadFilter(void)
{
    char* filename = fl_file_chooser("choose a filter file", "*.txt", 0);

    if (filename) {
        LoadFilter(filename);
    }
}

void ImgView::LoadFilter(const char* filename)
{
    FILE* fp = fopen(filename, "r");

    int i;
    for (i = 0; i < FLT_WIDTH * FLT_HEIGHT; i++) {
        fscanf(fp, "%lf", fltKernel + i);
    }

    fscanf(fp, "%lf", &scale);
    fscanf(fp, "%lf", &offset);

    fclose(fp);

    InitFltDesignUI();
}

void ImgView::SaveFilter(void)
{
    char* filename = fl_file_chooser("choose a filter file", "*.txt", 0);

    if (filename) {
        SaveFilter(filename);
    }
}

void ImgView::SaveFilter(const char* filename)
{
    FILE* fp = fopen(filename, "w");

    int i, j;
    for (j = 0; j < FLT_HEIGHT; j++) {
        for (i = 0; i < FLT_WIDTH; i++) {
            fprintf(fp, "%lf ", fltKernel[j * FLT_WIDTH + i]);
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "%lf\n", scale);
    fprintf(fp, "%lf", offset);

    fclose(fp);
}

void ImgView::PreviewFilter(void)
{
    if (imgBuf) {
        Filter();

        for (int y = 0; y < imgHeight; y++) {
            for (int x = 0; x < imgWidth; x++) {
                for (int c = 0; c < 3; c++) {
                    curImgChar[y * imgWidth * 3 + x * 3 + c] = (unsigned char)__min(255.0, __max(0.0, floor(curImg[y * imgWidth * 3 + x * 3 + c])));
                }
            }
        }

        UpdateViewBuffer();

        redraw();
    }

    //cout<<"preview"<<endl;
}

void ImgView::CancelFilter(void)
{
    if (imgBuf) {
        for (int y = 0; y < imgHeight; y++) {
            for (int x = 0; x < imgWidth; x++) {
                for (int c = 0; c < 3; c++) {
                    curImg[y * imgWidth * 3 + x * 3 + c] = double(origImg[y * imgWidth * 3 + x * 3 + c]);
                    curImgChar[y * imgWidth * 3 + x * 3 + c] = origImg[y * imgWidth * 3 + x * 3 + c];

                }
            }
        }

        UpdateViewBuffer();

        redraw();
    }

    //cout<<"cancel"<<endl;
}

void ImgView::AcceptFilter(void)
{
    if (imgBuf) {
        for (int y = 0; y < imgHeight; y++) {
            for (int x = 0; x < imgWidth; x++) {
                for (int c = 0; c < 3; c++) {
                    origImg[y * imgWidth * 3 + x * 3 + c] = imgBuf[y * imgWidth * 3 + x * 3 + c] = curImgChar[y * imgWidth * 3 + x * 3 + c];
                }
            }
        }

        for (int j = 0; j < imgHeight; j++) {
            for (int i = 0; i < imgWidth; i++) {
                int imgIndex = 3 * (j * imgWidth + i);
                int graphIndex = 3 * ((3 * j + 1) * graphWidth + (3 * i + 1));
                int x = i;
                int y = imgHeight - 1 - j;

                for (int c = 0; c < 3; c++) {
                    //costGraph[graphIndex+c] =
                    pixelNodes[graphIndex + c] =
                        imgBuf[imgIndex + c];

                    pixelNodes[graphIndex + 3 * (+1)			+ c] =
                        pixelNodes[graphIndex + 3 * (-graphWidth + 1) + c] =
                            pixelNodes[graphIndex + 3 * (-graphWidth)	+ c] =
                                pixelNodes[graphIndex + 3 * (-graphWidth - 1) + c] =
                                    pixelNodes[graphIndex + 3 * (-1)			+ c] =
                                        pixelNodes[graphIndex + 3 * (+graphWidth - 1) + c] =
                                            pixelNodes[graphIndex + 3 * (+graphWidth)	+ c] =
                                                pixelNodes[graphIndex + 3 * (+graphWidth + 1) + c] = 0;
                }
            }
        }

        InitNodeBuf(nodeBuf, imgBuf, imgWidth, imgHeight);
        MakeCostGraph(costGraph, nodeBuf, imgBuf, imgWidth, imgHeight);

    }



    UpdateViewBuffer();

    redraw();

    //cout<<"accept"<<endl;
}

void ImgView::StopFilter(void)
{
    if (imgBuf) {
        for (int y = 0; y < imgHeight; y++) {
            for (int x = 0; x < imgWidth; x++) {
                for (int c = 0; c < 3; c++) {
                    curImg[y * imgWidth * 3 + x * 3 + c] = double(origImg[y * imgWidth * 3 + x * 3 + c]);
                    curImgChar[y * imgWidth * 3 + x * 3 + c] = origImg[y * imgWidth * 3 + x * 3 + c];

                }
            }
        }

        UpdateViewBuffer();

        redraw();
    }

    fltDesignUI->hide();
    //cout<<"stop filter!"<<endl;
}

void ImgView::InitFltDesignUI(void)
{
    fltDesignUI->ele0->value(fltKernel[0]);
    fltDesignUI->ele1->value(fltKernel[1]);
    fltDesignUI->ele2->value(fltKernel[2]);
    fltDesignUI->ele3->value(fltKernel[3]);
    fltDesignUI->ele4->value(fltKernel[4]);
    fltDesignUI->ele5->value(fltKernel[5]);
    fltDesignUI->ele6->value(fltKernel[6]);
    fltDesignUI->ele7->value(fltKernel[7]);
    fltDesignUI->ele8->value(fltKernel[8]);
    fltDesignUI->ele9->value(fltKernel[9]);
    fltDesignUI->ele10->value(fltKernel[10]);
    fltDesignUI->ele11->value(fltKernel[11]);
    fltDesignUI->ele12->value(fltKernel[12]);
    fltDesignUI->ele13->value(fltKernel[13]);
    fltDesignUI->ele14->value(fltKernel[14]);
    fltDesignUI->ele15->value(fltKernel[15]);
    fltDesignUI->ele16->value(fltKernel[16]);
    fltDesignUI->ele17->value(fltKernel[17]);
    fltDesignUI->ele18->value(fltKernel[18]);
    fltDesignUI->ele19->value(fltKernel[19]);
    fltDesignUI->ele20->value(fltKernel[20]);
    fltDesignUI->ele21->value(fltKernel[21]);
    fltDesignUI->ele22->value(fltKernel[22]);
    fltDesignUI->ele23->value(fltKernel[23]);
    fltDesignUI->ele24->value(fltKernel[24]);

    fltDesignUI->scale->value(scale);
    fltDesignUI->offset->value(offset);
}

void ImgView::UpdateFilter()
{
    fltKernel[0] = fltDesignUI->ele0->value();
    fltKernel[1] = fltDesignUI->ele1->value();
    fltKernel[2] = fltDesignUI->ele2->value();
    fltKernel[3] = fltDesignUI->ele3->value();
    fltKernel[4] = fltDesignUI->ele4->value();
    fltKernel[5] = fltDesignUI->ele5->value();
    fltKernel[6] = fltDesignUI->ele6->value();
    fltKernel[7] = fltDesignUI->ele7->value();
    fltKernel[8] = fltDesignUI->ele8->value();
    fltKernel[9] = fltDesignUI->ele9->value();
    fltKernel[10] = fltDesignUI->ele10->value();
    fltKernel[11] = fltDesignUI->ele11->value();
    fltKernel[12] = fltDesignUI->ele12->value();
    fltKernel[13] = fltDesignUI->ele13->value();
    fltKernel[14] = fltDesignUI->ele14->value();
    fltKernel[15] = fltDesignUI->ele15->value();
    fltKernel[16] = fltDesignUI->ele16->value();
    fltKernel[17] = fltDesignUI->ele17->value();
    fltKernel[18] = fltDesignUI->ele18->value();
    fltKernel[19] = fltDesignUI->ele19->value();
    fltKernel[20] = fltDesignUI->ele20->value();
    fltKernel[21] = fltDesignUI->ele21->value();
    fltKernel[22] = fltDesignUI->ele22->value();
    fltKernel[23] = fltDesignUI->ele23->value();
    fltKernel[24] = fltDesignUI->ele24->value();

    scale = fltDesignUI->scale->value();
    offset = fltDesignUI->offset->value();

//	printf("update filter\n");
}

void ImgView::Filter(void)
{
    if (imgBuf) {
        unsigned char* range = NULL;

        if (fltDesignUI->selection->value()) {
            range = brushSelection;
        }

        image_filter(curImg, origImg, range, imgWidth, imgHeight,
                     fltKernel, FLT_WIDTH, FLT_HEIGHT,
                     scale, offset);
    }
}

void ImgView::TryBrush(void)
{
    brushConfigUI->show();
}

void ImgView::UpdateBrushConfig(void)
{
    /*if (brushConfigUI->round->value())
    {
    	brushType = ROUND_BRUSH;
    }
    else if (brushConfigUI->square->value())
    {
    	brushType = SQUARE_BRUSH;
    }*/

    brushSize = (int)brushConfigUI->brushSize->value();
    brushOpacity = brushConfigUI->brushOpacity->value();

    brushRadius2 = brushSize * brushSize;

    //printf("brush size %d\n",brushSize);

    if (imgBuf) {
        UpdateImgBufOpacity();
        if (drawMode == IMAGE_ONLY || drawMode == IMAGE_WITH_CONTOUR) {
            UpdateViewBuffer();
            redraw();
        }
    }
}

void ImgView::CleanBrushSelection(void)
{
    if (imgBuf) {
        memset(brushSelection, 0, imgWidth * imgHeight * sizeof(unsigned char));

        UpdateImgBufOpacity();
        if (drawMode == IMAGE_ONLY || drawMode == IMAGE_WITH_CONTOUR) {
            UpdateViewBuffer();
            redraw();
        }
    }
}

void ImgView::BrushSelection(int b)
{
    int invalid_tree = 0;
    if (!b && brushSelPtr == brushSelection) {
        brushSelPtr = NULL;
        invalid_tree = 1;
    } else if (b && brushSelPtr == NULL) {
        brushSelPtr = brushSelection;
        invalid_tree = 1;
    }
    if (invalid_tree && imgBuf) {
        if (currentCntr.GetCount()) {
            Seed* seed = currentCntr.GetTailPtr()->Data();
            int col = seed->x;
            int row = seed->y;
            int seedIndex = row * imgWidth + col;

            if (brushSelPtr == brushSelection && !brushSelection[seedIndex]) {
                printf("current seed ( %d , %d ) is out of brush selected range!\n", col, row);
            }

            int expanded = imgWidth * imgHeight;
            if (scissorPanelUI->expanded->active()) {
                expanded = (int)scissorPanelUI->expanded->value();
            }
            LiveWireDP(col, row, nodeBuf, imgWidth, imgHeight, brushSelPtr, expanded);

            printf("minimum path tree is finished\n");

            UpdateViewBuffer();
            redraw();
        }
    }
}

void ImgView::UpdateImgBufOpacity(void)
{
    int i, j;
    for (j = 0; j < imgHeight; j++) {
        for (i = 0; i < imgWidth; i++) {
            int selIndex = j * imgWidth + i;
            int imgIndex = 3 * selIndex;
            if (brushSelection[selIndex]) {
                for (int c = 0; c < 3; c++) {
                    double tmp = origImg[imgIndex + c];
                    tmp *= brushOpacity;
                    imgBuf[imgIndex + c] = (unsigned char)__min(255.0, __max(0.0, floor(tmp)));
                    curImgChar[imgIndex + c] = imgBuf[imgIndex + c];
                    curImg[imgIndex + c] = double(curImgChar[imgIndex + c]);
                }
            } else {
                for (int c = 0; c < 3; c++) {
                    imgBuf[imgIndex + c] = origImg[imgIndex + c];
                    curImgChar[imgIndex + c] = imgBuf[imgIndex + c];
                    curImg[imgIndex + c] = double(curImgChar[imgIndex + c]);
                }
            }
        }
    }
}

void ImgView::TryScissor(void)
{
    scissorPanelUI->show();
}

void ImgView::OrigImage(void)
{
    if (drawMode != IMAGE_ONLY) {
        if (drawMode == GRAPH_WITH_TREE) {
            UpdatePathTree();
        }

        drawMode = IMAGE_ONLY;
        if (imgBuf) {
            UpdateViewBuffer();
            redraw();
        }
    }
    scissorPanelUI->expanded->deactivate();
}

void ImgView::Contour(void)
{
    if (drawMode != IMAGE_WITH_CONTOUR) {
        if (drawMode == GRAPH_WITH_TREE) {
            UpdatePathTree();
        }

        drawMode = IMAGE_WITH_CONTOUR;
        if (imgBuf) {
            UpdateViewBuffer();
            redraw();
        }
    }
    scissorPanelUI->expanded->deactivate();
}

void ImgView::PixelColor(void)
{
    if (drawMode != GRAPH_WITH_COLOR) {
        if (drawMode == GRAPH_WITH_TREE) {
            UpdatePathTree();
        }

        drawMode = GRAPH_WITH_COLOR;
        if (imgBuf) {
            UpdateViewBuffer();
            redraw();
        }
    }
    scissorPanelUI->expanded->deactivate();
}

void ImgView::CostGraph(void)
{
    if (drawMode != GRAPH_WITH_COST) {
        if (drawMode == GRAPH_WITH_TREE) {
            UpdatePathTree();
        }

        drawMode = GRAPH_WITH_COST;
        if (imgBuf) {
            UpdateViewBuffer();
            redraw();
        }
    }
    scissorPanelUI->expanded->deactivate();

}

void ImgView::PathTree(void)
{
    if (drawMode != GRAPH_WITH_TREE) {
        drawMode = GRAPH_WITH_TREE;
        if (imgBuf) {
            PartialExpanding();
        }
        scissorPanelUI->expanded->activate();
    }
}

void ImgView::MinPath(void)
{
    if (drawMode != GRAPH_WITH_PATH) {
        if (drawMode == GRAPH_WITH_TREE) {
            UpdatePathTree();
        }

        drawMode = GRAPH_WITH_PATH;
        if (imgBuf) {
            UpdateViewBuffer();
            redraw();
        }
    }
    scissorPanelUI->expanded->deactivate();

}

void ImgView::PartialExpanding(void)
{
    int expanded = (int)scissorPanelUI->expanded->value();

    if (imgBuf) {
        if (currentCntr.GetCount()) {
            Seed* seed = currentCntr.GetTailPtr()->Data();
            int col = seed->x;
            int row = seed->y;
            int seedIndex = row * imgWidth + col;

            if (brushSelPtr == brushSelection && !brushSelection[seedIndex]) {
                printf("current seed ( %d , %d ) is out of brush selected range!\n", col, row);
            }

            LiveWireDP(col, row, nodeBuf, imgWidth, imgHeight, brushSelPtr, expanded);

            // Fixed by Loren. Was:
            // freePtX = (x-zoomPort[0])/zoomFactor+targetPort[0];
            freePtX = (col - zoomPort[0]) / zoomFactor + targetPort[0];
            freePtX /= 3;
            // freePtY = (y-zoomPort[2])/zoomFactor+targetPort[2];
            freePtY = (row - zoomPort[2]) / zoomFactor + targetPort[2];
            freePtY /= 3;

            printf("minimum path tree is finished\n");
        }

        UpdateViewBuffer();
        redraw();
    }
}

void ImgView::UpdatePathTree(void)
{
    if (imgBuf && currentCntr.GetCount()) {
        Seed* seed = currentCntr.GetTailPtr()->Data();
        int col = seed->x;
        int row = seed->y;
        int seedIndex = row * imgWidth + col;

        if (brushSelPtr == brushSelection && !brushSelection[seedIndex]) {
            printf("current seed ( %d , %d ) is out of brush selected range!\n", col, row);
        }

        LiveWireDP(col, row, nodeBuf, imgWidth, imgHeight, brushSelPtr, imgWidth * imgHeight);

        printf("minimum path tree is finished\n");
    }
}

void ImgView::MarkPath(int col, int row, const unsigned char clr[3])
{
    if (drawMode == IMAGE_WITH_CONTOUR) {

        int freePtIndex = row * imgWidth + col;
        Node* node = nodeBuf + freePtIndex;

        int imgX = col;
        int imgY = row;

        if (IsPtInRect(imgX, imgY, targetPort)) {
            int viewPixelTop = (imgY - targetPort[2]) * zoomFactor + zoomPort[2];
            int viewPixelLeft = (imgX - targetPort[0]) * zoomFactor + zoomPort[0];

            unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
            for (int n = 0; n < zoomFactor; n++) {
                unsigned char* viewCol = viewRow;
                for (int m = 0; m < zoomFactor; m++) {
                    viewCol[0] = clr[0];
                    viewCol[1] = clr[1];
                    viewCol[2] = clr[2];
                    viewCol += 3;
                }
                viewRow += 3 * viewWidth;
            }
        }

        while (node->prevNode) {
            node = node->prevNode;

            imgX = node->column;
            imgY = node->row;

            if (IsPtInRect(imgX, imgY, targetPort)) {
                int viewPixelTop = (imgY - targetPort[2]) * zoomFactor + zoomPort[2];
                int viewPixelLeft = (imgX - targetPort[0]) * zoomFactor + zoomPort[0];

                unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                for (int n = 0; n < zoomFactor; n++) {
                    unsigned char* viewCol = viewRow;
                    for (int m = 0; m < zoomFactor; m++) {
                        viewCol[0] = clr[0];
                        viewCol[1] = clr[1];
                        viewCol[2] = clr[2];
                        viewCol += 3;
                    }
                    viewRow += 3 * viewWidth;
                }
            }
        }
    } else if (drawMode == GRAPH_WITH_PATH) {
        int imgX = col / 3;
        int imgY = row / 3;

        int freePtIndex = imgY * imgWidth + imgX;
        Node* node = nodeBuf + freePtIndex;

        int graphX = 3 * imgX + 1;
        int graphY = 3 * imgY + 1;

        if (IsPtInRect(graphX, graphY, targetPort)) {
            int viewPixelTop = (graphY - targetPort[2]) * zoomFactor + zoomPort[2];
            int viewPixelLeft = (graphX - targetPort[0]) * zoomFactor + zoomPort[0];

            unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
            for (int n = 0; n < zoomFactor; n++) {
                unsigned char* viewCol = viewRow;
                for (int m = 0; m < zoomFactor; m++) {
                    viewCol[0] = clr[0];
                    viewCol[1] = clr[1];
                    viewCol[2] = clr[2];
                    viewCol += 3;
                }
                viewRow += 3 * viewWidth;
            }
        }

        while (node->prevNode) {
            node = node->prevNode;

            imgX = node->column;
            imgY = node->row;

            graphX = 3 * imgX + 1;
            graphY = 3 * imgY + 1;

            if (IsPtInRect(graphX, graphY, targetPort)) {
                int viewPixelTop = (graphY - targetPort[2]) * zoomFactor + zoomPort[2];
                int viewPixelLeft = (graphX - targetPort[0]) * zoomFactor + zoomPort[0];

                unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                for (int n = 0; n < zoomFactor; n++) {
                    unsigned char* viewCol = viewRow;
                    for (int m = 0; m < zoomFactor; m++) {
                        viewCol[0] = clr[0];
                        viewCol[1] = clr[1];
                        viewCol[2] = clr[2];
                        viewCol += 3;
                    }
                    viewRow += 3 * viewWidth;
                }
            }
        }
    }
}

void ImgView::UnMarkPath(int col, int row)
{
    if (drawMode == IMAGE_WITH_CONTOUR) {
        int freePtIndex = row * imgWidth + col;
        Node* node = nodeBuf + freePtIndex;

        int imgX = col;
        int imgY = row;
        const unsigned char* imgPixel = imgBuf + 3 * (imgY * imgWidth + imgX);

        if (IsPtInRect(imgX, imgY, targetPort)) {
            int viewPixelTop = (imgY - targetPort[2]) * zoomFactor + zoomPort[2];
            int viewPixelLeft = (imgX - targetPort[0]) * zoomFactor + zoomPort[0];

            unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
            for (int n = 0; n < zoomFactor; n++) {
                unsigned char* viewCol = viewRow;
                for (int m = 0; m < zoomFactor; m++) {
                    viewCol[0] = imgPixel[0];
                    viewCol[1] = imgPixel[1];
                    viewCol[2] = imgPixel[2];
                    viewCol += 3;
                }
                viewRow += 3 * viewWidth;
            }
        }

        while (node->prevNode) {
            node = node->prevNode;

            imgX = node->column;
            imgY = node->row;
            imgPixel = imgBuf + 3 * (imgY * imgWidth + imgX);

            if (IsPtInRect(imgX, imgY, targetPort)) {
                int viewPixelTop = (imgY - targetPort[2]) * zoomFactor + zoomPort[2];
                int viewPixelLeft = (imgX - targetPort[0]) * zoomFactor + zoomPort[0];

                unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                for (int n = 0; n < zoomFactor; n++) {
                    unsigned char* viewCol = viewRow;
                    for (int m = 0; m < zoomFactor; m++) {
                        viewCol[0] = imgPixel[0];
                        viewCol[1] = imgPixel[1];
                        viewCol[2] = imgPixel[2];
                        viewCol += 3;
                    }
                    viewRow += 3 * viewWidth;
                }
            }
        }
    } else if (drawMode == GRAPH_WITH_PATH) {
        int imgX = col / 3;
        int imgY = row / 3;
        const unsigned char* imgPixel = imgBuf + 3 * (imgY * imgWidth + imgX);

        int freePtIndex = imgY * imgWidth + imgX;
        Node* node = nodeBuf + freePtIndex;

        int graphX = 3 * imgX + 1;
        int graphY = 3 * imgY + 1;

        if (IsPtInRect(graphX, graphY, targetPort)) {
            int viewPixelTop = (graphY - targetPort[2]) * zoomFactor + zoomPort[2];
            int viewPixelLeft = (graphX - targetPort[0]) * zoomFactor + zoomPort[0];

            unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
            for (int n = 0; n < zoomFactor; n++) {
                unsigned char* viewCol = viewRow;
                for (int m = 0; m < zoomFactor; m++) {
                    viewCol[0] = imgPixel[0];
                    viewCol[1] = imgPixel[1];
                    viewCol[2] = imgPixel[2];
                    viewCol += 3;
                }
                viewRow += 3 * viewWidth;
            }
        }

        while (node->prevNode) {
            node = node->prevNode;

            imgX = node->column;
            imgY = node->row;
            imgPixel = imgBuf + 3 * (imgY * imgWidth + imgX);


            graphX = 3 * imgX + 1;
            graphY = 3 * imgY + 1;

            if (IsPtInRect(graphX, graphY, targetPort)) {
                int viewPixelTop = (graphY - targetPort[2]) * zoomFactor + zoomPort[2];
                int viewPixelLeft = (graphX - targetPort[0]) * zoomFactor + zoomPort[0];

                unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                for (int n = 0; n < zoomFactor; n++) {
                    unsigned char* viewCol = viewRow;
                    for (int m = 0; m < zoomFactor; m++) {
                        viewCol[0] = imgPixel[0];
                        viewCol[1] = imgPixel[1];
                        viewCol[2] = imgPixel[2];
                        viewCol += 3;
                    }
                    viewRow += 3 * viewWidth;
                }
            }
        }
    }
}

void ImgView:: MarkPath(const CTypedPtrDblList <Seed>* cntr, const unsigned char clr[3])
{
    if (drawMode == IMAGE_WITH_CONTOUR) {
        CTypedPtrDblElement <Seed>* seedEle = cntr->GetHeadPtr();
        while (!cntr->IsSentinel(seedEle)) {
            Seed* seed = seedEle->Data();
            int imgX = seed->x;
            int imgY = seed->y;
            if (IsPtInRect(imgX, imgY, targetPort)) {
                int viewPixelTop = (imgY - targetPort[2]) * zoomFactor + zoomPort[2];
                int viewPixelLeft = (imgX - targetPort[0]) * zoomFactor + zoomPort[0];

                unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                for (int n = 0; n < zoomFactor; n++) {
                    unsigned char* viewCol = viewRow;
                    for (int m = 0; m < zoomFactor; m++) {
                        viewCol[0] = clr[0];
                        viewCol[1] = clr[1];
                        viewCol[2] = clr[2];
                        viewCol += 3;
                    }
                    viewRow += 3 * viewWidth;
                }
            }

            seedEle = seedEle->Next();
        }
    } else if (drawMode == GRAPH_WITH_PATH) {
        CTypedPtrDblElement <Seed>* seedEle = cntr->GetHeadPtr();
        while (!cntr->IsSentinel(seedEle)) {
            Seed* seed = seedEle->Data();
            int imgX = seed->x;
            int imgY = seed->y;
            int graphX = 3 * imgX + 1;
            int graphY = 3 * imgY + 1;

            if (IsPtInRect(graphX, graphY, targetPort)) {
                int viewPixelTop = (graphY - targetPort[2]) * zoomFactor + zoomPort[2];
                int viewPixelLeft = (graphX - targetPort[0]) * zoomFactor + zoomPort[0];

                unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                for (int n = 0; n < zoomFactor; n++) {
                    unsigned char* viewCol = viewRow;
                    for (int m = 0; m < zoomFactor; m++) {
                        viewCol[0] = clr[0];
                        viewCol[1] = clr[1];
                        viewCol[2] = clr[2];
                        viewCol += 3;
                    }
                    viewRow += 3 * viewWidth;
                }
            }

            seedEle = seedEle->Next();
        }
    }
}

void ImgView::UnMarkPath(const CTypedPtrDblList <Seed>* cntr)
{
    if (drawMode == IMAGE_WITH_CONTOUR) {
        CTypedPtrDblElement <Seed>* seedEle = cntr->GetHeadPtr();
        while (!cntr->IsSentinel(seedEle)) {
            Seed* seed = seedEle->Data();
            int imgX = seed->x;
            int imgY = seed->y;
            const unsigned char* imgPixel = imgBuf + 3 * (imgY * imgWidth + imgX);

            if (IsPtInRect(imgX, imgY, targetPort)) {
                int viewPixelTop = (imgY - targetPort[2]) * zoomFactor + zoomPort[2];
                int viewPixelLeft = (imgX - targetPort[0]) * zoomFactor + zoomPort[0];

                unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                for (int n = 0; n < zoomFactor; n++) {
                    unsigned char* viewCol = viewRow;
                    for (int m = 0; m < zoomFactor; m++) {
                        viewCol[0] = imgPixel[0];
                        viewCol[1] = imgPixel[1];
                        viewCol[2] = imgPixel[2];
                        viewCol += 3;
                    }
                    viewRow += 3 * viewWidth;
                }
            }

            seedEle = seedEle->Next();
        }
    } else if (drawMode == GRAPH_WITH_PATH) {
        CTypedPtrDblElement <Seed>* seedEle = cntr->GetHeadPtr();
        while (!cntr->IsSentinel(seedEle)) {
            Seed* seed = seedEle->Data();
            int imgX = seed->x;
            int imgY = seed->y;
            const unsigned char* imgPixel = imgBuf + 3 * (imgY * imgWidth + imgX);

            int graphX = 3 * imgX + 1;
            int graphY = 3 * imgY + 1;

            if (IsPtInRect(graphX, graphY, targetPort)) {
                int viewPixelTop = (graphY - targetPort[2]) * zoomFactor + zoomPort[2];
                int viewPixelLeft = (graphX - targetPort[0]) * zoomFactor + zoomPort[0];

                unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                for (int n = 0; n < zoomFactor; n++) {
                    unsigned char* viewCol = viewRow;
                    for (int m = 0; m < zoomFactor; m++) {
                        viewCol[0] = imgBuf[0];
                        viewCol[1] = imgBuf[1];
                        viewCol[2] = imgBuf[2];
                        viewCol += 3;
                    }
                    viewRow += 3 * viewWidth;
                }
            }

            seedEle = seedEle->Next();
        }

    }
}

void ImgView:: MarkCurrentContour(void)
{
    MarkPath(&currentCntr, SELECTED_PATH_COLOR);
}

void ImgView:: MarkPreviousContour(void)
{
    for (int i = 0; i < contours.GetSize(); i++) {
        if (i == selectedCntr) {
            MarkPath(contours[i], SELECTED_PATH_COLOR);
        } else {
            MarkPath(contours[i], PATH_COLOR);
        }
    }
}

void ImgView:: MarkAllContour(void)
{
    MarkCurrentContour();
    MarkPreviousContour();
}

void ImgView::MarkAllContour(int col, int row)
{
    MarkAllContour();
    MarkPath(col, row, SELECTED_PATH_COLOR);
}

void ImgView:: UnMarkCurrentContour(void)
{
    UnMarkPath(&currentCntr);
}

void ImgView:: UnMarkPreviousContour(void)
{
    for (int i = 0; i < contours.GetSize(); i++) {
        UnMarkPath(contours[i]);
    }

}

void ImgView:: UnMarkAllContour(void)
{
    UnMarkCurrentContour();
    UnMarkPreviousContour();
}

void ImgView::MarkPathTree(void)
{
    int i, j;

    for (j = 0; j < imgHeight; j++) {
        for (i = 0; i < imgWidth; i++) {
            int nodeIndex = j * imgWidth + i;
            Node* node = nodeBuf + nodeIndex;

            if (node->state == EXPANDED || node->state == ACTIVE) {
                Node* prevNode = node->prevNode;

                if (prevNode) {
                    int prevNodeCol = prevNode->column;
                    int prevNodeRow = prevNode->row;

                    int graphCol[2], graphRow[2];

                    if (prevNodeCol < node->column) {
                        graphCol[0] = 3 * node->column;
                        graphCol[1] = 3 * node->column - 1;
                    } else if (prevNodeCol > node->column) {
                        graphCol[0] = 3 * node->column + 2;
                        graphCol[1] = 3 * node->column + 3;
                    } else {
                        graphCol[0] = 3 * node->column + 1;
                        graphCol[1] = 3 * node->column + 1;
                    }

                    if (prevNodeRow < node->row) {
                        graphRow[0] = 3 * node->row;
                        graphRow[1] = 3 * node->row - 1;
                    } else if (prevNodeRow > node->row) {
                        graphRow[0] = 3 * node->row + 2;
                        graphRow[1] = 3 * node->row + 3;
                    } else {
                        graphRow[0] = 3 * node->row + 1;
                        graphRow[1] = 3 * node->row + 1;
                    }

                    for (int k = 0; k < 2; k++) {
                        if (IsPtInRect(graphCol[k], graphRow[k], targetPort)) {
                            int viewPixelTop = (graphRow[k] - targetPort[2]) * zoomFactor + zoomPort[2];
                            int viewPixelLeft = (graphCol[k] - targetPort[0]) * zoomFactor + zoomPort[0];

                            unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                            for (int n = 0; n < zoomFactor; n++) {
                                unsigned char* viewCol = viewRow;
                                for (int m = 0; m < zoomFactor; m++) {
                                    viewCol[0] = (TREE_COLOR[0] >> k);
                                    viewCol[1] = (TREE_COLOR[1] >> k);
                                    viewCol[2] = (TREE_COLOR[2] >> k);
                                    viewCol += 3;
                                }
                                viewRow += 3 * viewWidth;
                            }
                        }
                    }
                }

                int gRow = 3 * node->row + 1;
                int gCol = 3 * node->column + 1;

                if (IsPtInRect(gCol, gRow, targetPort)) {
                    int viewPixelTop = (gRow - targetPort[2]) * zoomFactor + zoomPort[2];
                    int viewPixelLeft = (gCol - targetPort[0]) * zoomFactor + zoomPort[0];

                    unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                    for (int n = 0; n < zoomFactor; n++) {
                        unsigned char* viewCol = viewRow;
                        for (int m = 0; m < zoomFactor; m++) {
                            if (node->state == EXPANDED) {
                                viewCol[0] = EXPANDED_COLOR[0];
                                viewCol[1] = EXPANDED_COLOR[1];
                                viewCol[2] = EXPANDED_COLOR[2];
                            } else {
                                viewCol[0] = ACTIVE_COLOR[0];
                                viewCol[1] = ACTIVE_COLOR[1];
                                viewCol[2] = ACTIVE_COLOR[2];
                            }
                            viewCol += 3;
                        }
                        viewRow += 3 * viewWidth;
                    }
                }
            }
        }
    }
}

void ImgView::MarkPathOnTree(int col, int row)
{
    int nodeIndex = row * imgWidth + col;
    Node* node = nodeBuf + nodeIndex;

    while (node->prevNode) {
        Node* prevNode = node->prevNode;
        int prevNodeCol = prevNode->column;
        int prevNodeRow = prevNode->row;

        int graphCol[2], graphRow[2];

        if (prevNodeCol < node->column) {
            graphCol[0] = 3 * node->column;
            graphCol[1] = 3 * node->column - 1;
        } else if (prevNodeCol > node->column) {
            graphCol[0] = 3 * node->column + 2;
            graphCol[1] = 3 * node->column + 3;
        } else {
            graphCol[0] = 3 * node->column + 1;
            graphCol[1] = 3 * node->column + 1;
        }

        if (prevNodeRow < node->row) {
            graphRow[0] = 3 * node->row;
            graphRow[1] = 3 * node->row - 1;
        } else if (prevNodeRow > node->row) {
            graphRow[0] = 3 * node->row + 2;
            graphRow[1] = 3 * node->row + 3;
        } else {
            graphRow[0] = 3 * node->row + 1;
            graphRow[1] = 3 * node->row + 1;
        }

        for (int k = 0; k < 2; k++) {
            if (IsPtInRect(graphCol[k], graphRow[k], targetPort)) {
                int viewPixelTop = (graphRow[k] - targetPort[2]) * zoomFactor + zoomPort[2];
                int viewPixelLeft = (graphCol[k] - targetPort[0]) * zoomFactor + zoomPort[0];

                unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                for (int n = 0; n < zoomFactor; n++) {
                    unsigned char* viewCol = viewRow;
                    for (int m = 0; m < zoomFactor; m++) {
                        viewCol[0] = (SELECTED_PATH_COLOR[0] >> k);
                        viewCol[1] = (SELECTED_PATH_COLOR[1] >> k);
                        viewCol[2] = (SELECTED_PATH_COLOR[2] >> k);
                        viewCol += 3;
                    }
                    viewRow += 3 * viewWidth;
                }
            }
        }

        node = prevNode;
    }
}

void ImgView::UnMarkPathOnTree(int col, int row)
{
    int nodeIndex = row * imgWidth + col;
    Node* node = nodeBuf + nodeIndex;

    while (node->prevNode) {
        Node* prevNode = node->prevNode;
        int prevNodeCol = prevNode->column;
        int prevNodeRow = prevNode->row;

        int graphCol[2], graphRow[2];

        if (prevNodeCol < node->column) {
            graphCol[0] = 3 * node->column;
            graphCol[1] = 3 * node->column - 1;
        } else if (prevNodeCol > node->column) {
            graphCol[0] = 3 * node->column + 2;
            graphCol[1] = 3 * node->column + 3;
        } else {
            graphCol[0] = 3 * node->column + 1;
            graphCol[1] = 3 * node->column + 1;
        }

        if (prevNodeRow < node->row) {
            graphRow[0] = 3 * node->row;
            graphRow[1] = 3 * node->row - 1;
        } else if (prevNodeRow > node->row) {
            graphRow[0] = 3 * node->row + 2;
            graphRow[1] = 3 * node->row + 3;
        } else {
            graphRow[0] = 3 * node->row + 1;
            graphRow[1] = 3 * node->row + 1;
        }

        for (int k = 0; k < 2; k++) {
            if (IsPtInRect(graphCol[k], graphRow[k], targetPort)) {
                int viewPixelTop = (graphRow[k] - targetPort[2]) * zoomFactor + zoomPort[2];
                int viewPixelLeft = (graphCol[k] - targetPort[0]) * zoomFactor + zoomPort[0];

                unsigned char* viewRow = viewBuf + 3 * (viewPixelTop * viewWidth + viewPixelLeft);
                for (int n = 0; n < zoomFactor; n++) {
                    unsigned char* viewCol = viewRow;
                    for (int m = 0; m < zoomFactor; m++) {
                        viewCol[0] = (TREE_COLOR[0] >> k);
                        viewCol[1] = (TREE_COLOR[1] >> k);
                        viewCol[2] = (TREE_COLOR[2] >> k);
                        viewCol += 3;
                    }
                    viewRow += 3 * viewWidth;
                }
            }
        }

        node = prevNode;
    }
}

void ImgView::AppendCurrentContour(int col, int row)
{
    // unnecessary way, but to check students' codes.

    CTypedPtrDblList <Node> path;
    MinimumPath(&path, col, row, nodeBuf, imgWidth, imgHeight);
    if (path.GetCount() > 1) {
        Node* node = path.RemoveTail();
        int imgX = node->column;
        int imgY = node->row;

        CTypedPtrDblElement <Seed>* seedEle = currentCntr.AddTail(new Seed(imgX, imgY, 1));

        while(path.GetCount() > 1) {
            node = path.RemoveTail();
            int imgX = node->column;
            int imgY = node->row;

            seedEle = currentCntr.AddPrev(seedEle, new Seed(imgX, imgY, 0));
        }
    }
}

void ImgView::FinishCurrentContour(int col, int row)
{
    AppendCurrentContour(col, row);
    currentCntr.SetCircular(0);
}

void ImgView::FinishCurrentContour(void)
{
    Seed* seed = currentCntr.GetHeadPtr()->Data();
    AppendCurrentContour(seed->x, seed->y);
    //currentCntr.RemoveTail();
    currentCntr.SetCircular(1);
}

void ImgView::CommitCurrentContour()
{
    CTypedPtrDblList <Seed>* newCntr = new CTypedPtrDblList <Seed>;

    while(!currentCntr.IsEmpty()) {
        newCntr->AddTail(currentCntr.RemoveTail());
    }

    newCntr->SetCircular(currentCntr.IsCircular());

    contours.AddTail(newCntr);
}

void ImgView::ChopLastSeed(void)
{
    currentCntr.RemoveTail();

    CTypedPtrDblElement <Seed>* seedEle = currentCntr.GetTailPtr();

    while (!currentCntr.IsSentinel(seedEle) && seedEle->Data()->seed == 0) {
        delete seedEle->Data();
        seedEle = seedEle->Prev();
        currentCntr.RemoveTail();
    }
}

int ImgView::IsPtAroundContour(int x, int y, const CTypedPtrDblList <Seed>* cntr) const
{
    CTypedPtrDblElement <Seed>* seedEle = cntr->GetHeadPtr();

    while (!cntr->IsSentinel(seedEle)) {
        Seed* seed = seedEle->Data();
        if (abs(seed->x - x) + abs(seed->y - y) < 8) return 1;
        seedEle = seedEle->Next();
    }
    return 0;
}

void ImgView::AllocateViewBuffer(int w, int h)
{
    viewWidth = w;
    viewHeight = h;
    viewBuf = new unsigned char [w * h * 3];
}

void ImgView::UpdateViewPort(int bufLeft, int bufTop, int bufWidth, int bufHeight)
{
    int viewPort[4] = {
        -((viewWidth / 2) / zoomFactor),
        (viewWidth - viewWidth / 2) / zoomFactor,
        -((viewHeight / 2) / zoomFactor),
        (viewHeight - (viewHeight / 2)) / zoomFactor
    };

    int bufPort[4] = {
        bufLeft,
        bufLeft + bufWidth,
        bufTop,
        bufTop + bufHeight
    };

    int interPort[4];

    RectIntersection(interPort, bufPort, viewPort);

    targetPort[0] = interPort[0] - bufLeft;
    targetPort[1] = interPort[1] - bufLeft;
    targetPort[2] = interPort[2] - bufTop;
    targetPort[3] = interPort[3] - bufTop;

    zoomPort[0] = interPort[0] * zoomFactor + viewWidth / 2;
    zoomPort[1] = interPort[1] * zoomFactor + viewWidth / 2;
    zoomPort[2] = interPort[2] * zoomFactor + viewHeight / 2;
    zoomPort[3] = interPort[3] * zoomFactor + viewHeight / 2;
}

void ImgView::UpdateViewBuffer(const unsigned char* origBuf, int bufLeft, int bufTop, int bufWidth, int bufHeight)
{
    if (imgBuf == NULL) return;

    UpdateViewPort(bufLeft, bufTop, bufWidth, bufHeight);

    //copy origBuf, targetPort
    //to   viewBuf, zoomPort
    //each orig Pixel expands by zoomFactor*zoomFactor
    //other pixels are BK_COLOR;

    int i, j;

    unsigned char* zb = viewBuf;

    if (zoomPort[0] < zoomPort[1] && zoomPort[2] < zoomPort[3]) {
        for (j = 0; j < zoomPort[2]; j++) {
            for (i = 0; i < viewWidth; i++) {
                *zb = BK_COLOR[0];
                zb++;
                *zb = BK_COLOR[1];
                zb++;
                *zb = BK_COLOR[2];
                zb++;
            }
        }

        const unsigned char* bufRow = origBuf + 3 * (targetPort[2] * bufWidth + targetPort[0]);

        for (j = zoomPort[2]; j < zoomPort[3]; j += zoomFactor) {
            for (int n = 0; n < zoomFactor; n++) {
                for (i = 0; i < zoomPort[0]; i++) {
                    *zb = BK_COLOR[0];
                    zb++;
                    *zb = BK_COLOR[1];
                    zb++;
                    *zb = BK_COLOR[2];
                    zb++;

                }

                const unsigned char* bufCol = bufRow;

                for (i = zoomPort[0]; i < zoomPort[1]; i += zoomFactor) {
                    for (int m = 0; m < zoomFactor; m++) {
                        *zb = bufCol[0];
                        zb++;
                        *zb = bufCol[1];
                        zb++;
                        *zb = bufCol[2];
                        zb++;
                    }

                    bufCol += 3;
                }

                for (i = zoomPort[1]; i < viewWidth; i++) {
                    *zb = BK_COLOR[0];
                    zb++;
                    *zb = BK_COLOR[1];
                    zb++;
                    *zb = BK_COLOR[2];
                    zb++;
                }
            }

            bufRow += 3 * bufWidth;
        }

        for (j = zoomPort[3]; j < viewHeight; j++) {
            for (i = 0; i < viewWidth; i++) {
                *zb = BK_COLOR[0];
                zb++;
                *zb = BK_COLOR[1];
                zb++;
                *zb = BK_COLOR[2];
                zb++;
            }
        }
    } else {
        for (j = 0; j < viewHeight; j++) {
            for (i = 0; i < viewWidth; i++) {
                *zb = BK_COLOR[0];
                zb++;
                *zb = BK_COLOR[1];
                zb++;
                *zb = BK_COLOR[2];
                zb++;
            }
        }
    }
}

void ImgView::UpdateViewBuffer(void)
{
    if (drawMode == IMAGE_ONLY) {
        UpdateViewBuffer(curImgChar, imgLeft, imgTop, imgWidth, imgHeight);
    } else if (drawMode == IMAGE_WITH_CONTOUR ) {
        UpdateViewBuffer(curImgChar, imgLeft, imgTop, imgWidth, imgHeight);
        if (currentCntr.IsEmpty()) {
            MarkAllContour();
        } else {
            MarkAllContour(freePtX, freePtY);
        }
    } else if (drawMode == GRAPH_WITH_COLOR) {
        UpdateViewBuffer(pixelNodes, 3 * imgLeft, 3 * imgTop, graphWidth, graphHeight);
    } else if (drawMode == GRAPH_WITH_COST) {
        UpdateViewBuffer(costGraph, 3 * imgLeft, 3 * imgTop, graphWidth, graphHeight);
    } else if (drawMode == GRAPH_WITH_TREE) {
        UpdateViewBuffer(pixelNodes, 3 * imgLeft, 3 * imgTop, graphWidth, graphHeight);
        if (!currentCntr.IsEmpty()) {
            MarkPathTree();
        }
    } else if (drawMode == GRAPH_WITH_PATH) {
        UpdateViewBuffer(pixelNodes, 3 * imgLeft, 3 * imgTop, graphWidth, graphHeight);
        if (!currentCntr.IsEmpty()) {
            MarkPathTree();
            MarkPathOnTree(freePtX, freePtY);
        }
    }
}

void ImgView::resize(int x, int y, int w, int h)
{
    if (imgBuf) {
        AllocateViewBuffer(w, h);
        UpdateViewBuffer();
        redraw();
    }

    Fl_Double_Window::resize(x, y, w, h);
}

int ImgView::handle(int c)
{
    if (c == FL_PUSH) {
        int x = Fl::event_x();
        int y = Fl::event_y();

        if (imgBuf) {
            if (Fl::event_button() == FL_LEFT_MOUSE) {
                if ((Fl::get_key(FL_Control_L) || Fl::get_key(FL_Control_R)) && currentCntr.IsEmpty() && (drawMode == IMAGE_WITH_CONTOUR || drawMode == IMAGE_ONLY) ) {
                    // apply brush here.
                    int cntX = (x - zoomPort[0]) / zoomFactor + targetPort[0];
                    int cntY = (y - zoomPort[2]) / zoomFactor + targetPort[2];

                    //int i,j;

                    /********************TO DO********************
                     * apply brush here, by updating brushSelection.  You'll also need to update LiveWireDP to
                     * ensure that the live wire with brush selection works correctly
                     *
                     */

                    printf("selecting region by brush (1): to be implemented in ImgView.cpp\n");
                    /******************************************************/
                    UpdateImgBufOpacity();
                    UpdateViewBuffer();
                    redraw();
                }

                else if (currentCntr.IsEmpty() && drawMode == IMAGE_WITH_CONTOUR)
                    //if ((Fl::get_key(FL_Control_L)||Fl::get_key(FL_Control_R))
                    //	 && drawMode == IMAGE_WITH_CONTOUR)
                {
                    int col = (x - zoomPort[0]) / zoomFactor + targetPort[0];
                    int row = (y - zoomPort[2]) / zoomFactor + targetPort[2];

                    if (IsPtInRect(col, row, targetPort)) {
                        if (!currentCntr.IsEmpty()) {
                            UnMarkPath(freePtX, freePtY);
                            UnMarkCurrentContour();
                            currentCntr.RemoveAll();
                        }

                        currentCntr.SetCircular(0);
                        int oldCol = col;
                        int oldRow = row;
                        SeedSnap(col, row, imgBuf, imgWidth, imgHeight);
                        if (!IsPtInRect(col, row, targetPort)) {
                            printf("the result of SeedSnap is out of current view port, still use un-snapped seed!\n");
                            col = oldCol;
                            row = oldRow;
                        }
                        Seed* seed = new Seed(col, row, 1);
                        currentCntr.AddTail(seed);

                        selectedCntr = -1;

                        MarkAllContour();

                        printf("current seed position: ( %4d , %4d ).\n", col, row);

                        int seedIndex = row * imgWidth + col;

                        if (brushSelPtr == brushSelection && !brushSelection[seedIndex]) {
                            printf("current seed ( %d , %d ) is out of brush selected range!\n", col, row);
                        }

                        LiveWireDP(col, row, nodeBuf, imgWidth, imgHeight, brushSelPtr, imgWidth * imgHeight);

                        printf("minimum path tree is finished\n");

                        freePtX = col;
                        freePtY = row;

                        redraw();
                    }
                } else if (currentCntr.GetCount() && drawMode == IMAGE_WITH_CONTOUR) {
                    int col = (x - zoomPort[0]) / zoomFactor + targetPort[0];
                    int row = (y - zoomPort[2]) / zoomFactor + targetPort[2];

                    if (IsPtInRect(col, row, targetPort)) {

                        UnMarkPath(freePtX, freePtY);

                        AppendCurrentContour(col, row); //must be called before LiveWireDP;
                        MarkAllContour();

                        printf("current seed position: ( %4d , %4d ).\n", col, row);

                        int seedIndex = row * imgWidth + col;

                        if (brushSelPtr == brushSelection && !brushSelection[seedIndex]) {
                            printf("current seed ( %d , %d ) is out of brush selected range!\n", col, row);
                        }

                        LiveWireDP(col, row, nodeBuf, imgWidth, imgHeight, brushSelPtr, imgWidth * imgHeight);

                        freePtX = col;
                        freePtY = row;

                        printf("minimum path tree is finished\n");

                        redraw();
                    }
                }

            }
        }

        mouseX = x;
        mouseY = y;

        return 1;
    } else if (c == FL_MOVE) {
        int x = Fl::event_x();
        int y = Fl::event_y();

        unsigned char pixel[3] = {BK_COLOR[0], BK_COLOR[1], BK_COLOR[2]};
        if (imgBuf && 0 <= x && x < viewWidth
                && 0 <= y && y < viewHeight) {
            memcpy(pixel, viewBuf + 3 * (y * viewWidth + x), 3 * sizeof(unsigned char));
        }

        char info[256];
        sprintf(info, "x = %d y = %d r = %d g = %d b = %d",
                x, y, (int)pixel[0], (int)pixel[1], (int)pixel[2]);
        mouseInfo->value(info);

        if (imgBuf) {
            if (drawMode == IMAGE_WITH_CONTOUR) {
                int col = (x - zoomPort[0]) / zoomFactor + targetPort[0];
                int row = (y - zoomPort[2]) / zoomFactor + targetPort[2];

                if (IsPtInRect(col, row, targetPort)) {
                    if (currentCntr.GetCount()) {
                        UnMarkPath(freePtX, freePtY);
                        MarkAllContour(col, row);

                        freePtX = col;
                        freePtY = row;

                        redraw();
                    } else {
                        for (int k = 0; k < contours.GetSize(); k++) {
                            if (IsPtAroundContour(col, row, contours[k])) {
                                if (0 <= selectedCntr && selectedCntr < contours.GetSize()) {
                                    MarkPath(contours[selectedCntr], PATH_COLOR);
                                }
                                MarkPath(contours[k], SELECTED_PATH_COLOR);

                                selectedCntr = k;
                                redraw();

                                break;
                            }
                        }
                    }
                }
            } else if (drawMode == GRAPH_WITH_PATH) {
                int col = (x - zoomPort[0]) / zoomFactor + targetPort[0];
                int row = (y - zoomPort[2]) / zoomFactor + targetPort[2];
                col /= 3;
                row /= 3;

                if (currentCntr.GetCount()) {
                    UnMarkPathOnTree(freePtX, freePtY);
                    MarkPathOnTree(col, row);
                    freePtX = col;
                    freePtY = row;

                    redraw();
                }
            }
        }

        mouseX = x;
        mouseY = y;

        return 1;
    } else if (c == FL_DRAG) {
        int x = Fl::event_x();
        int y = Fl::event_y();

        if (Fl::event_button() == FL_RIGHT_MOUSE) {
            if (imgBuf) {
                imgLeft += (x - mouseX) / zoomFactor;
                imgTop += (y - mouseY) / zoomFactor;

                UpdateViewBuffer();
                //MarkAllContour(freePtX,freePtY);

                redraw();
            }
        } else if ((Fl::get_key(FL_Control_L) || Fl::get_key(FL_Control_R))) {
            if (drawMode == IMAGE_ONLY || drawMode == IMAGE_WITH_CONTOUR ) {
                if (currentCntr.IsEmpty()) {
                    int cntX = (x - zoomPort[0]) / zoomFactor + targetPort[0];
                    int cntY = (y - zoomPort[2]) / zoomFactor + targetPort[2];

                    //int i,j;

                    /********************TO DO********************
                     * apply brush here, by updating brushSelection.  You'll also need to update LiveWireDP to
                     * ensure that the live wire with brush selection works correctly
                     *
                     */

                    printf("selecting region by brush (2): to be implemented in ImgView.cpp\n");
                    /******************************************************/
                    UpdateImgBufOpacity();
                    UpdateViewBuffer();
                    redraw();
                }
            }
        }

        mouseX = x;
        mouseY = y;

        return 1;
    } else if (c == FL_RELEASE) {
        int x = Fl::event_x();
        int y = Fl::event_y();

        mouseX = x;
        mouseY = y;

        return 1;
    } else if (c == FL_SHORTCUT) {
        int x = Fl::event_x();
        int y = Fl::event_y();

        if (Fl::event_key(FL_BackSpace)) {
            if (drawMode == IMAGE_WITH_CONTOUR) {
                if (currentCntr.GetCount()) {
                    UnMarkPath(freePtX, freePtY);
                    UnMarkCurrentContour();

                    ChopLastSeed();

                    if (currentCntr.GetCount()) {
                        Seed* lastSeed = currentCntr.GetTailPtr()->Data();
                        int col = lastSeed->x;
                        int row = lastSeed->y;
                        int seedIndex = row * imgWidth + col;

                        if (brushSelPtr == brushSelection && !brushSelection[seedIndex]) {
                            printf("current seed ( %d , %d ) is out of brush selected range!\n", col, row);
                        }

                        LiveWireDP(col, row, nodeBuf, imgWidth, imgHeight, brushSelPtr, imgWidth * imgHeight);

                        freePtX = (x - zoomPort[0]) / zoomFactor + targetPort[0];
                        freePtY = (y - zoomPort[2]) / zoomFactor + targetPort[2];

                        printf("minimum path tree is finished\n");

                        MarkAllContour(freePtX, freePtY);
                    } else {
                        MarkAllContour();
                    }

                    redraw();
                } else {
                    if (0 <= selectedCntr && selectedCntr < contours.GetSize()) {
                        UnMarkPath(contours[selectedCntr]);
                        CTypedPtrDblList <Seed>* oldCntr = contours[selectedCntr];
                        contours[selectedCntr] = contours[contours.GetSize() - 1];
                        contours.RemoveTail();
                        oldCntr->FreePtrs();
                        delete oldCntr;
                        selectedCntr = -1;
                        MarkAllContour();
                        redraw();
                    }

                }
            }
            return 1;
        } else
            return Fl_Double_Window::handle(c);
    }
    //fixed by JHC
    else if (c == FL_FOCUS || c == FL_UNFOCUS)  {
        return 1;
    }
    //fixed by JHC
    else if (c == FL_KEYDOWN) {
        int x = Fl::event_x();
        int y = Fl::event_y();

        if (Fl::event_key('=')  ) {
            if (zoomFactor < 16) {
                zoomFactor ++;
                UpdateViewBuffer();
                redraw();
            }
            return 1;
        } else if (Fl::event_key('-')) {
            if (zoomFactor > 1) {
                zoomFactor --;
                UpdateViewBuffer();
                redraw();
            }
            return 1;
        } else if (Fl::event_key(FL_Enter)) {
            if (drawMode == IMAGE_WITH_CONTOUR && currentCntr.GetCount()) {
                int col = (x - zoomPort[0]) / zoomFactor + targetPort[0];
                int row = (y - zoomPort[2]) / zoomFactor + targetPort[2];

                if (IsPtInRect(col, row, targetPort)) {
                    UnMarkPath(freePtX, freePtY);


                    FinishCurrentContour();

                    CommitCurrentContour();

                    selectedCntr = contours.GetSize() - 1;

                    MarkAllContour();

                    redraw();
                }
            }
            return 1;
        } else if (Fl::event_key(FL_BackSpace)) {
            if (drawMode == IMAGE_WITH_CONTOUR) {
                if (currentCntr.GetCount()) {
                    UnMarkPath(freePtX, freePtY);
                    UnMarkCurrentContour();

                    ChopLastSeed();

                    if (currentCntr.GetCount()) {
                        Seed* lastSeed = currentCntr.GetTailPtr()->Data();
                        int col = lastSeed->x;
                        int row = lastSeed->y;
                        int seedIndex = row * imgWidth + col;

                        if (brushSelPtr == brushSelection && !brushSelection[seedIndex]) {
                            printf("current seed ( %d , %d ) is out of brush selected range!\n", col, row);
                        }

                        LiveWireDP(col, row, nodeBuf, imgWidth, imgHeight, brushSelPtr, imgWidth * imgHeight);

                        freePtX = (x - zoomPort[0]) / zoomFactor + targetPort[0];
                        freePtY = (y - zoomPort[2]) / zoomFactor + targetPort[2];

                        printf("minimum path tree is finished\n");

                        MarkAllContour(freePtX, freePtY);
                    } else {
                        MarkAllContour();
                    }

                    redraw();
                } else {
                    if (0 <= selectedCntr && selectedCntr < contours.GetSize()) {
                        UnMarkPath(contours[selectedCntr]);
                        CTypedPtrDblList <Seed>* oldCntr = contours[selectedCntr];
                        contours[selectedCntr] = contours[contours.GetSize() - 1];
                        contours.RemoveTail();
                        oldCntr->FreePtrs();
                        delete oldCntr;
                        selectedCntr = -1;
                        MarkAllContour();
                        redraw();
                    }

                }
            }
            return 1;
        } else
            return Fl_Double_Window::handle(c);

    } else {
        return Fl_Double_Window::handle(c);

    }
}

void ImgView::draw(void)
{
    //fl_rectf(0,0,w(),h(),BK_COLOR[0],BK_COLOR[1],BK_COLOR[2]);

    if (imgBuf) {
        fl_draw_image(viewBuf, 0, 0, viewWidth, viewHeight, 3, 0);
    } else {
        Fl_Window::draw();
    }
}

void ImgView::AboutMe(void)
{
    helpPageUI->show();
}
