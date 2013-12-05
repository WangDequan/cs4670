#ifndef DETECTION_H
#define DETECTION_H

#include "Common.h"

class Detection
{
public:
    Detection();
    Detection(double x, double y, double resp, double width, double height);

    void draw(CByteImage &img) const;

    double relativeOverlap(const Detection &other) const;
    double area() const;

    double x, y;          // Coordinates of the center of the detection
    double response;      // Response of the detector at (x, y) in the response image
    double width, height; // Width and height of the support of the feature vector
};

std::ostream &operator<<(std::ostream &s, const Detection &d);
std::istream &operator>>(std::istream &s, Detection &d);

void drawDetections(CByteImage &img, const std::vector<Detection> &dets);

// Determine if each detection in found is a match or not. Returns label and response,
// both with the same number of elements as found. Each entry in the vector label is -1
// if detection is not matched and 1 otherwise. The response vector simply
// copies the response from each detection.
void computeLabels(const std::vector<Detection> &gt, const std::vector<Detection> &found,
                   std::vector<float> &label, std::vector<float> &response);

void computeLabels(const std::vector<std::vector<Detection> > &gt, const std::vector<std::vector<Detection> > &found,
                   std::vector<float> &label, std::vector<float> &response, int& nDets);

#endif // DETECTION_H