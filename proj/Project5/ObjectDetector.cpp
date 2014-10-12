#include "ObjectDetector.h"
#include <algorithm>

#define WIN_SIZE_NMS_KEY   "nms_win_size"
#define RESP_THESH_KEY     "sv_response_threshold"
#define OVERLAP_THRESH_KEY "detection_overlap_threshold"

ObjectDetector::ObjectDetector(const ParametersMap &params)
{
    _winSizeNMS = params.getInt(WIN_SIZE_NMS_KEY);
    _respThresh = params.getFloat(RESP_THESH_KEY);
    _overlapThresh = params.getFloat(OVERLAP_THRESH_KEY);
}

ParametersMap
ObjectDetector::getDefaultParameters()
{
    ParametersMap params;
    params.set(WIN_SIZE_NMS_KEY  , 11  );
    params.set(RESP_THESH_KEY    ,  0  );
    params.set(OVERLAP_THRESH_KEY,  0.2);
    return params;
}

ParametersMap
ObjectDetector::getParameters() const
{
    ParametersMap params;
    params.set(WIN_SIZE_NMS_KEY, _winSizeNMS);
    params.set(RESP_THESH_KEY, _respThresh);
    params.set(OVERLAP_THRESH_KEY, _overlapThresh);
    return params;
}

ObjectDetector::ObjectDetector(int winSizeNMS, double respThresh, double overlapThresh):
    _winSizeNMS(winSizeNMS),
    _respThresh(respThresh),
    _overlapThresh(overlapThresh)
{
}

void
ObjectDetector::operator()( const CFloatImage &svmResp, const Size &roiSize,
                            double featureScaleFactor, std::vector<Detection> &dets,
                            double imScale ) const
{
    /******** BEGIN TODO ********/
    // Non-Maxima Suppression on a single image
    //
    // For every window of size _winSizeNMS by _winSizeNMS determine if the
    // pixel at the center of the window is the local maxima and is also
    // greater than _respThresh. If so, create an instance of Detection and
    // store it in dets, remember to set the position of the central pixel
    // (x,y), as well as the dimensions of the detection (based on roiSize). Y
    // ou will have to correct location and dimensions using a scale factor
    // that is a function of featureScaleFactor and imScale.

    dets.resize(0);

	CShape svmRespShape = svmResp.Shape();
	double scale = imScale * featureScaleFactor;
	for (int x = 0; x < svmRespShape.width; x++) {
		for (int y = 0; y < svmRespShape.height; y++) {
            if(svmResp.Pixel(x,y,0) == getMaxofImageWindow(svmResp, x, y, _winSizeNMS)) {
				Detection det = Detection(x*scale, y*scale, svmResp.Pixel(x,y,0), roiSize.width*scale, roiSize.height*scale);
				dets.push_back(det);
			}
		}
	}

    /******** END TODO ********/
}

float 
ObjectDetector::getMaxofImageWindow(const CFloatImage &img, int xC, int yC, int _winSizeNMS) const 
{
	float max = 0.0;
	for (int x = xC - _winSizeNMS; x < xC + _winSizeNMS; x++) {
		for (int y = yC - _winSizeNMS; y < yC + _winSizeNMS; y++) {
			if(x > 0 && y > 0 && x < img.Shape().width && y < img.Shape().height) {
				if(img.Pixel(x, y, 0) > max)
					max = img.Pixel(x,y,0);
			}
		}
	}
	return max;
}

bool
sortByResponse(const Detection &d1, const Detection &d2)
{
    return d1.response >= d2.response;
}

void
ObjectDetector::operator()( const SBFloatPyramid &svmRespPyr, const Size &roiSize,
                            double featureScaleFactor, std::vector<Detection> &dets ) const
{
    /******** BEGIN TODO ********/
    // Non-Maxima Suppression across pyramid levels
    //
    // Given the pyramid of SVM responses, for each level you will find
    // the non maximas within a window of size _winSizeNMS by _winSizeNMS.
    // This functionality is impelmented in the other operator() method above.
    // Once all detections for all levels are found we perform another round of
    // non maxima suppression, this time across all levels. In this step you will
    // use the relativeOverlap method from the class Detection to determine if
    // a detection "competes" with another. If there is enough overlap between them
    // (i.e., if the relative overlap is greater then _overlapThresh) then only the
    // detection with strongest response is kept.
    //
    // Steps are:
    // 1) Find the local maxima per level of the pyramid for all levels
    // 2) Perform non maxima suppression across levels
    //
    // Useful functions:
    // sortByResponse, relativeOverlap, opreator()

    dets.resize(0);

    // Find detections per level
    std::vector<Detection> allDets;
    for (int i = 0; i < svmRespPyr.getNLevels(); i++) {

        std::vector<Detection> levelDets;
        this->operator()(svmRespPyr[i], roiSize, featureScaleFactor, levelDets, svmRespPyr.levelScale(i));

        allDets.insert(allDets.end(), levelDets.begin(), levelDets.end());
    }
	std::sort(allDets.begin(), allDets.end(), sortByResponse);
	std:vector<Detection> useDets;

	for (int i = 0; i < allDets.size(); i++) {
		Detection curDet = allDets[i];
		bool addDet = true;
		for (int j = 0;j < useDets.size(); j++) {
			if(curDet.relativeOverlap(useDets[j]) > _overlapThresh){
				addDet = false;
				break;
			}
		}
		if(addDet){
			useDets.insert(useDets.end(), curDet);
		}
	}
	dets = useDets;

	

printf("TODO: %s:%d\n", __FILE__, __LINE__); 

    /******** END TODO ********/
}

