#include "Detection.h"

Detection::Detection():
    x(0), y(0), response(0), width(0), height(0)
{}

Detection::Detection(double x, double y, double response, double width, double height):
    x(x), y(y), response(response), width(width), height(height)
{}

double
Detection::area() const
{
    return width * height;
}

double
Detection::relativeOverlap(const Detection &other) const
{
    /******** BEGIN TODO ********/
    // Compute the relative overlap between two detections. This
    // is defined as the ratio
    //
    //                    AreaInter(r1, r2)
    //                   ------------------
    //                    AreaUnion(r1, r2)
    //
    // where AreaInter is the area of the intersection of the two
    // rectangles r1 and r2, and AreaUnion is the area of the union
    // of the two rectangles.

    double relOver = 0.0;

printf("TODO: %s:%d\n", __FILE__, __LINE__); 

    /******** END TODO ********/
    return relOver;
}

class Color
{
public:
    Color(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b) {}
    uint8_t r, g, b;
};

static
void
drawHorizLine(CByteImage &img, int xStart, int xEnd, int y, const Color &color)
{
    CShape shape = img.Shape();
    assert(shape.nBands == 3);

    if(y < 0 || y >= shape.height) return;

    xStart = std::max(0, xStart);
    xEnd = std::min(xEnd, shape.width - 1);

    char *pix = (char *)img.PixelAddress(xStart, y, 0);
    for (int i = 0; i < (xEnd - xStart); i++, pix += 3) {
        pix[0] = color.b;
        pix[1] = color.g;
        pix[2] = color.r;
    }
}

static
void
drawVertLine(CByteImage &img, int x, int yStart, int yEnd, const Color &color)
{
    CShape shape = img.Shape();
    assert(shape.nBands == 3);

    if(x < 0 || x >= shape.width) return;

    yStart = std::max(0, yStart);
    yEnd = std::min(yEnd, shape.height - 1);

    //char* pix = (char*)img.PixelAddress(x, yStart, 0);
    for (int i = 0; i < (yEnd - yStart); i++) { //, pix += stride) {
        char *pix = (char *)img.PixelAddress(x, yStart + i, 0);

        pix[0] = color.b;
        pix[1] = color.g;
        pix[2] = color.r;
    }
}

void
Detection::draw(CByteImage &img) const
{
    static const Color red(255, 50, 50);
    // Bounding box
    drawHorizLine(img, x - width / 2.0, x + width / 2.0, y - height / 2.0, red);
    drawHorizLine(img, x - width / 2.0, x + width / 2.0, y + height / 2.0, red);

    drawVertLine(img, x - width / 2.0, y - height / 2.0, y + height / 2.0, red);
    drawVertLine(img, x + width / 2.0, y - height / 2.0, y + height / 2.0, red);

    // Central point
    const int crossSize = 3;
    drawHorizLine(img, x - crossSize, x + crossSize, y, red);
    drawVertLine(img, x, y - crossSize, y + crossSize, red);
}

void
drawDetections(CByteImage &img, const std::vector<Detection> &dets)
{
    for(std::vector<Detection>::const_iterator det = dets.begin(), detEnd = dets.end(); det != detEnd; det++) {
        det->draw(img);
    }
}

std::ostream &
operator<<(std::ostream &s, const Detection &d)
{
    s << d.x << " " << d.y << " " << d.width << " " << d.height << " " << d.response;
    return s;
}

std::istream &
operator>>(std::istream &s, Detection &d)
{
    s >> d.x >> d.y >> d.width >> d.height >> d.response;
    return s;
}

bool
sortByResponse(const std::pair<int, float> &a, const std::pair<int, float> &b)
{
    return a.second > b.second;
}

void
computeLabels(const std::vector<Detection> &gt, const std::vector<Detection> &found,
              std::vector<float> &label, std::vector<float> &response)
{
    const double overlapThresh = 0.5;

    vector<float> labels;
    vector<pair<int, float> > idxResp(found.size());
    for (int i = 0; i < found.size(); i++) {
        idxResp[i] = pair<int, float>(i, found[i].response);
    }

    sort(idxResp.begin(), idxResp.end(), sortByResponse);

    // debug
    for(int i = 0; i < idxResp.size() - 1; i++) {
        assert(idxResp[i].second >= idxResp[i + 1].second);
    }

    label = vector<float>(found.size(), -1);
    response = vector<float>(found.size(), 0);

    std::vector<bool> taken(gt.size(), false);
    for (int i = 0; i < idxResp.size(); i++) {

        int idx = idxResp[i].first;

        const Detection &det = found[idx];

        response[idx] = det.response;

        float bestIdx = -1;
        float bestOverlap = -1;

        for (int j = 0; j < gt.size(); j++) {
            if(taken[j]) continue;

            float overlap = gt[j].relativeOverlap(det);
            if(overlap < overlapThresh) continue;

            if((bestIdx < 0) || (overlap > bestOverlap)) {
                bestOverlap = overlap;
                bestIdx = j;
            }
        }

        if(bestIdx >= 0) {
            label[idx] = 1;
            taken[bestIdx] = true;
        }
    }
}

void
computeLabels(const std::vector<std::vector<Detection> > &gt, const std::vector<std::vector<Detection> > &found,
              std::vector<float> &label, std::vector<float> &response)
{
    using namespace std;
    assert(gt.size() == found.size());

    response.resize(0);
    label.resize(0);

    for(int i = 0; i < gt.size(); i++) {

        vector<float> resp, lab;
        PRINT_EXPR(found.size());
        computeLabels(gt[i], found[i], resp, lab);

        response.insert(response.end(), resp.begin(), resp.end());
        label.insert(label.end(), lab.begin(), lab.end());
    }
}



