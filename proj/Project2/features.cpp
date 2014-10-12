#include <assert.h>
#include <math.h>
#include <FL/Fl.H>
#include <FL/Fl_Image.H>
#include "features.h"
#include "ImageLib/FileIO.h"

#define PI 3.14159265358979323846

// Compute features of an image.
bool computeFeatures(CFloatImage &image, FeatureSet &features, int featureType, int descriptorType) {
	// Compute different types of features depending on value of featureType
	//   1: Dummy Features  - Arbitrary example features for demonstration purposes only DONT USE THIS
	//   2: Harris Features - Features selected using Harris corner detection method
	switch (featureType) {
	case 1:
		dummyComputeFeatures(image, features);
		break;
	case 2:
		ComputeHarrisFeatures(image, features);
		break;
	default:
		return false;
	}

	// Compute different types of feature descriptors depending on value of descriptorType
	//   1: Simple Descriptor - 5x5 square window without orientation centered on feature
	//   2: MOPS Descriptor   - 8x8 oriented window sub-sampled from a 41x41 pixel region around feature
	//   3: Custom Descriptor - extra credit TBD
	switch (descriptorType) {
	case 1:
		ComputeSimpleDescriptors(image, features);
		break;
	case 2:
		ComputeMOPSDescriptors(image, features);
		break;
	case 3:
		ComputeCustomDescriptors(image, features);
		break;
	default:
		return false;
	}

	// This is just to make sure the IDs are assigned in order, because
	// the ID gets used to index into the feature array.
	for (unsigned int i=0; i<features.size(); i++) {
		features[i].id = i;
	}

	return true;
}

// Perform a query on the database.  This simply runs matchFeatures on
// each image in the database, and returns the feature set of the best
// matching image.
bool performQuery(const FeatureSet &f, const ImageDatabase &db, int &bestIndex, vector<FeatureMatch> &bestMatches, double &bestDistance, int matchType) {
    vector<FeatureMatch> tempMatches;

    for (unsigned int i=0; i<db.size(); i++) {
        if (!matchFeatures(f, db[i].features, tempMatches, matchType)) {
            return false;
        }

        bestIndex = i;
        bestMatches = tempMatches;
    }

    return true;
}

// Match one feature set with another.
bool matchFeatures(const FeatureSet &f1, const FeatureSet &f2, vector<FeatureMatch> &matches, int matchType) {
	// Match features using one of two matching functions depending on value of matchType
	//   1: SSD Match   - Set match score to be the SSD (summed squared distance) between two features 
	//   2: Ratio Match - Set match score to be the SSD of the best feature divided by the SSD of the
	//                    second best feature
	switch (matchType) {
	case 1:
		ssdMatchFeatures(f1, f2, matches);
		return true;
	case 2:
		ratioMatchFeatures(f1, f2, matches);
		return true;
	default:
		return false;
	}
}

// Compute silly example features.  This doesn't do anything
// meaningful, but may be useful to use as an example.
void dummyComputeFeatures(CFloatImage &image, FeatureSet &features) {
    CShape sh = image.Shape();
    Feature f;

    for (int y=0; y<sh.height; y++) {
        for (int x=0; x<sh.width; x++) {
            double r = image.Pixel(x,y,0);
            double g = image.Pixel(x,y,1);
            double b = image.Pixel(x,y,2);

            if ((int)(255*(r+g+b)+0.5) % 100 == 1) {
                // If the pixel satisfies this meaningless criterion,
                // make it a feature.

                f.type = 1;
                f.id += 1;
                f.x = x;
                f.y = y;

                f.data.resize(1);
                f.data[0] = r + g + b;

                features.push_back(f);
            }
        }
    }
}

// Compute features using Harris corner detection method
void ComputeHarrisFeatures(CFloatImage &image, FeatureSet &features)
{
	// Create grayscale image used for Harris detection
	CFloatImage grayImage = ConvertToGray(image);

	// Create image to store Harris values
	CFloatImage harrisImage(image.Shape().width,image.Shape().height,1);

	// Create image to store local maximum harris values as 1, other pixels 0
	CByteImage harrisMaxImage(image.Shape().width,image.Shape().height,1);

	// Create image to store orientation values
	CFloatImage orientationImage(image.Shape().width, image.Shape().height, 1);

	// Compute the harris score at each pixel position, storing the result in in harrisImage. 
	computeHarrisValues(grayImage, harrisImage, orientationImage);

	// Threshold the harris image and compute local maxima.
	computeLocalMaxima(harrisImage,harrisMaxImage);

	// Save images
	CByteImage tmp(harrisImage.Shape());
	CByteImage tmp2(harrisImage.Shape());
	convertToByteImage(harrisImage, tmp);
	convertToByteImage(grayImage, tmp2);
	WriteFile(tmp2, "grayImg.tga");
	WriteFile(tmp, "harris.tga");
	WriteFile(harrisMaxImage, "harrisMax.tga");

	// Loop through feature points in harrisMaxImage and fill in id, type, x, y, and angle 
	// information needed for descriptor computation for each feature point, then add them
	// to feature set
	int id = 0;
	for (int y=0; y < harrisMaxImage.Shape().height; y++) {
		for (int x=0; x < harrisMaxImage.Shape().width; x++) {
			if (harrisMaxImage.Pixel(x, y, 0) == 1) {
				Feature f;
				f.id = id;
				f.type = 2;
				f.x = x;
				f.y = y;
				f.angleRadians = orientationImage.Pixel(x,y,0);
				
				features.push_back(f);
				id++;
			}
		}
	}
}

// Find the corresponding (newX, newY) coordinates for the point specified by index in a 
// 5x5 matrix centered at position (x,y)
//
//    0  1  2  3  4
//    5  6  7  8  9
//    10 11 12 13 14
//    15 16 17 18 19
//    20 21 22 23 24
//
void find5x5Index(int x, int y, int index, int *newX, int *newY){
	if(index < 0 || index > 24) {
		printf("index out of bounds");
		return;
	}
	*newX = (index / 5) - 2 + x;
	*newY = (index % 5) - 2 + y;
}

// Find the corresponding (newX, newY) coordinates for the point specified by index in a 
// 3x3 matrix centered at position (x,y)
//
//    0  1  2
//    3  4  5
//    6  7  8
//
void find3x3Index(int x, int y, int index, int *newX, int *newY){
	if(index < 0 || index > 8) {
		printf("index out of bounds"); 
		return;
	}
	*newX = (index / 3) - 1 + x;
	*newY = (index % 3) - 1 + y;
}

// Loop through the image to compute the harris corner values as described in class
// srcImage:  grayscale of original image
// harrisImage:  populate the harris values per pixel in this image
void computeHarrisValues(CFloatImage &srcImage, CFloatImage &harrisImage, CFloatImage &orientationImage)
{
	int w = srcImage.Shape().width;	 // image width
	int h = srcImage.Shape().height; // image height

	// Create images to store x-derivative and y-derivative values
	CFloatImage Ix(w,h,1);
	CFloatImage Iy(w,h,1);
	CFloatImage Ix_blur(w,h,1);
	CFloatImage Iy_blur(w,h,1);

	// Compute x-derivative values by convolving image with x sobel filter 
	Convolve(srcImage, Ix, ConvolveKernel_SobelX);

	// Compute y-derivative values by convolving image with y sobel filter
	Convolve(srcImage, Iy, ConvolveKernel_SobelY);
    
	// Apply a 7x7 gaussian blur to the grayscale image
	CFloatImage blurImage(w,h,1);
	Convolve(srcImage, blurImage, ConvolveKernel_7x7);

	// Compute x-derivative values by convolving blurred image with x sobel filter 
	Convolve(blurImage, Ix_blur, ConvolveKernel_SobelX);

	// Compute y-derivative values by convolving blurred image with y sobel filter
	Convolve(blurImage, Iy_blur, ConvolveKernel_SobelY);

	// Declare additional variables
	int newX, newY;		// (x,y) coordinate for pixel in 5x5 sliding window
	float dx, dy;		// x-derivative, y-derivative values
	double HMatrix[4];	// Harris matrix
	double determinant;	// determinant of Harris matrix
	double trace;		// trace of Harris matrix
	int padType = 2;	// select variable for what type of padding to use: , 0->zero, 1->edge, 2->reflect

	// Loop through 'srcImage' and compute harris score for each pixel
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {

			// reset Harris matrix values to 0
			memset(HMatrix, 0, sizeof(HMatrix));

			// Loop through pixels in 5x5 window to calculate Harris matrix
			for (int j = 0; j < 25; j++) {
				find5x5Index(x,y,j,&newX,&newY);
				if(srcImage.Shape().InBounds(newX, newY)) {
					dx = Ix.Pixel(newX,newY,0);
					dy = Iy.Pixel(newX,newY,0);
				} else {
					// Depending on value of padType, perform different types of border padding
					switch (padType) {
						case 1:
							// 1 -> replicate border values
							if (newX < 0) {
								newX = 0;
							} else if (newX >= w) {
								newX = w-1;
							}
				
							if (newY < 0) {
								newY = 0;
							} else if (newY >= h) {
								newY = h-1;
							}

							dx = Ix.Pixel(newX,newY,0);
							dy = Iy.Pixel(newX,newY,0);
							break;
						case 2:
							// 2 -> reflect border pixels
							if (newX < 0) {
								newX = -newX;
							} else if (newX >= w) {
								newX = w-(newX%w)-1;
							}
				
							if (newY < 0) {
								newY = -newY;
							} else if (newY >= h) {
								newY = h-(newY%h)-1;
							}

							dx = Ix.Pixel(newX,newY,0);
							dy = Iy.Pixel(newX,newY,0);
							break;
						default:
							// 0 -> zero padding
							dx = 0.0;
							dy = 0.0;
							break;
					}
				}
				HMatrix[0] += dx*dx*gaussian5x5[j];
				HMatrix[1] += dx*dy*gaussian5x5[j];
				HMatrix[2] += dx*dy*gaussian5x5[j];
				HMatrix[3] += dy*dy*gaussian5x5[j];
			}

			// Calculate determinant and trace of harris matrix
			determinant = (HMatrix[0] * HMatrix[3]) - (HMatrix[1] * HMatrix[2]);
			trace = HMatrix[0] + HMatrix[3];

			// Compute corner strength function c(H) = determinant(H)/trace(H) 
			// and save result into harrisImage 
			if(trace == 0)
				harrisImage.Pixel(x,y,0) = 0.0;
			else
				harrisImage.Pixel(x,y,0) = (determinant / trace);

			// Compute orientation and save result in 'orientationImage'
			dx = Ix_blur.Pixel(x,y,0);
			dy = Iy_blur.Pixel(x,y,0);

			if(dx == 0.0 && dy == 0.0)
				orientationImage.Pixel(x,y,0) = 0.0;
			else
				orientationImage.Pixel(x,y,0) = atan2(dy, dx);
		}
	}
}


//Loop through the image to determine suitable feature points
// srcImage:  image with Harris values
// destImage: Assign 1 to local maximum in 3x3 window that are above a given 
//            threshold, 0 otherwise
void computeLocalMaxima(CFloatImage &srcImage,CByteImage &destImage)
{
	int w = srcImage.Shape().width;	 // image width
	int h = srcImage.Shape().height; // image height
	
	//float threshold = .024;
	float threshold = .01;	// threshold value for identifying features


	// Declare additional variables
	float max;		// harris value to check as local max
	int newX, newY;	// (x,y) coordinate for pixel in 5x5 sliding window 
	int j;			// int for iterating through 5x5 window

	// Loop through 'srcImage' and determine suitable feature points that
	// fit the following criteria:
	//   - harris value c is greater than a predefined threshold
	//   - c is a local maximum in a 5x5 neighborhood
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {

			max = srcImage.Pixel(x,y,0);
			
			// If harris value is greater than a predefined threshold check
			// if value is a local maximum in a 5x5 neighborhood.
			if (max > threshold) {
				for (j = 0; j < 25; j++) {
					find5x5Index(x,y,j,&newX,&newY);
					if(srcImage.Shape().InBounds(newX, newY) && srcImage.Pixel(newX, newY, 0) > max) {
						destImage.Pixel(x,y,0) = 0;
						break;
					}
				}
				if (j != 25) 
					continue;
				destImage.Pixel(x,y,0) = 1;
			} else {
				destImage.Pixel(x,y,0) = 0;
			}
		}
	}
}

// Compute Simple descriptors.
void ComputeSimpleDescriptors(CFloatImage &image, FeatureSet &features)
{
	// Create grayscale image used for Harris detection
	CFloatImage grayImage=ConvertToGray(image);

	int w = grayImage.Shape().width;  // image width
	int h = grayImage.Shape().height; // image height

	// Declare additional variables
	int newX, newY;  // (x,y) coordinate for pixel in 5x5 sample window
	int padType = 0; // select variable for what type of padding to use: , 0->zero, 1->edge, 2->reflect

	// Iterate through feature set and store simple descriptors for each feature into
	// corresponding feature
	for (vector<Feature>::iterator i = features.begin(); i != features.end(); i++) {
		Feature &f = *i;

		// Set angle to 0 since simple descriptors do not include orientation 
		f.angleRadians = 0;

		// Resize data field for a 5x5 square window
		f.data.resize(5 * 5);

		// The descriptor is a 5x5 window of intensities sampled centered on the feature point
		for (int j = 0; j < 25; j++) {
			find5x5Index(f.x,f.y,j,&newX,&newY);
			if(grayImage.Shape().InBounds(newX, newY)) {
				f.data[j] = grayImage.Pixel(newX, newY, 0);
			} else {
				// Depending on value of padType, perform different types of border padding
				switch (padType) {
					case 1:
						// 1 -> replicate border values
						if (newX < 0) {
							newX = 0;
						} else if (newX >= w) {
							newX = w-1;
						}
				
						if (newY < 0) {
							newY = 0;
						} else if (newY >= h) {
							newY = h-1;
						}

						f.data[j] = grayImage.Pixel(newX, newY, 0);
						break;
					case 2:
						// 2 -> reflect border pixels
						if (newX < 0) {
							newX = -newX;
						} else if (newX >= w) {
							newX = w-(newX%w)-1;
						}
						if (newY < 0) {
							newY = -newY;
						} else if (newY >= h) {
							newY = h-(newY%h)-1;
						}
						
						f.data[j] = grayImage.Pixel(newX, newY, 0);
						break;
					default:
						// 0 -> zero padding
						f.data[j] = 0;
						break;
				}
			}
		}
	}
}

// Compute MOPs descriptors.
void ComputeMOPSDescriptors(CFloatImage &image, FeatureSet &features)
{
	int w = image.Shape().width;  // image width
	int h = image.Shape().height; // image height

	// Create grayscale image used for Harris detection
	CFloatImage grayImage=ConvertToGray(image);

	// Apply a 7x7 gaussian blur to the grayscale image
	CFloatImage blurImage(w,h,1);
	Convolve(grayImage, blurImage, ConvolveKernel_7x7);

	// Transform matrices
	CTransform3x3 xform;
	CTransform3x3 trans1;
	CTransform3x3 rotate;
	CTransform3x3 scale;
	CTransform3x3 trans2;

	// Declare additional variables
	float pxl;					// pixel value
	double mean, sq_sum, stdev; // variables for normailizing data set

	// This image represents the window around the feature you need to compute to store as the feature descriptor
	const int windowSize = 8;
	CFloatImage destImage(windowSize, windowSize, 1);

	for (vector<Feature>::iterator i = features.begin(); i != features.end(); i++) {
		Feature &f = *i;

		// Compute the transform from each pixel in the 8x8 image to sample from the appropriate 
		// pixels in the 40x40 rotated window surrounding the feature
		trans1 = CTransform3x3::Translation(f.x, f.y);						// translate window to feature point
		rotate = CTransform3x3::Rotation(f.angleRadians * 180.0 / PI);		// rotate window by angle
		scale = CTransform3x3::Scale(5.0);									// scale window by 5
		trans2 = CTransform3x3::Translation(-windowSize/2, -windowSize/2);	// translate window to origin

		// transform resulting from combining above transforms
		xform = trans1*scale*rotate*trans2;

		//Call the Warp Global function to do the mapping
		WarpGlobal(blurImage, destImage, xform, eWarpInterpLinear);

		// Resize data field for a 8x8 square window
		f.data.resize(windowSize * windowSize);	

		// Find mean of window
		mean = 0;
		for (int y = 0; y < windowSize; y++) {
			for (int x = 0; x < windowSize; x++) {
				pxl = destImage.Pixel(x, y, 0);
				f.data[y*windowSize + x] = pxl;
				mean += pxl/(windowSize*windowSize);
			}
		}

		// Find standard deviation of window
		sq_sum = 0;
		for (int k = 0; k < windowSize*windowSize; k++) {
			sq_sum += (mean - f.data[k]) * (mean - f.data[k]);
		}
		stdev = sqrt(sq_sum/(windowSize*windowSize));

		// Normalize window to have 0 mean and unit variance by subtracting
		// by mean and dividing by standard deviation
		for (int k = 0; k < windowSize*windowSize; k++) {
			f.data[k] = (f.data[k]-mean)/stdev;
		}
	}
}

// Compute Custom descriptors (extra credit)
void ComputeCustomDescriptors(CFloatImage &image, FeatureSet &features)
{
	
}

// Perform simple feature matching.  This just uses the SSD
// distance between two feature vectors, and matches a feature in the
// first image with the closest feature in the second image.  It can
// match multiple features in the first image to the same feature in
// the second image.
void ssdMatchFeatures(const FeatureSet &f1, const FeatureSet &f2, vector<FeatureMatch> &matches) {
    int m = f1.size();
    int n = f2.size();

    matches.resize(m);

    double d;
    double dBest;
    int idBest;

    for (int i=0; i<m; i++) {
        dBest = 1e100;
        idBest = 0;

        for (int j=0; j<n; j++) {
            d = distanceSSD(f1[i].data, f2[j].data);

            if (d < dBest) {
                dBest = d;
                idBest = f2[j].id;
            }
        }

        matches[i].id1 = f1[i].id;
        matches[i].id2 = idBest;
        matches[i].distance = dBest;
    }
}

// Perform ratio feature matching. This just uses the ratio of the SSD distance of the 
// two best matches and matches a feature in the first image with the closest feature 
// in the second image. It can match multiple features in the first image to the same 
// feature in the second image.
void ratioMatchFeatures(const FeatureSet &f1, const FeatureSet &f2, vector<FeatureMatch> &matches) 
{
	int m = f1.size();
    int n = f2.size();

    matches.resize(m);

    double d;
    double dBest, dBest2;
    int idBest;

    for (int i=0; i<m; i++) {
        dBest = 1e100;
		dBest2 = 1e100;
        idBest = 0;
        for (int j=0; j<n; j++) {
            d = distanceSSD(f1[i].data, f2[j].data);
            if (d < dBest) {
				dBest2 = dBest;
                dBest = d;
                idBest = f2[j].id;
            }
			else if (d < dBest2) {
				dBest2 = d;
			}
        }

        matches[i].id1 = f1[i].id;
        matches[i].id2 = idBest;
        matches[i].distance = dBest/dBest2;
    }
}

// Convert Fl_Image to CFloatImage.
bool convertImage(const Fl_Image *image, CFloatImage &convertedImage) {
    if (image == NULL) {
        return false;
    }

    // Let's not handle indexed color images.
    if (image->count() != 1) {
        return false;
    }

    int w = image->w();
    int h = image->h();
    int d = image->d();

    // Get the image data.
    const char *const *data = image->data();

    int index = 0;

    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            if (d < 3) {
                // If there are fewer than 3 channels, just use the
                // first one for all colors.
                convertedImage.Pixel(x,y,0) = ((uchar) data[0][index]) / 255.0f;
                convertedImage.Pixel(x,y,1) = ((uchar) data[0][index]) / 255.0f;
                convertedImage.Pixel(x,y,2) = ((uchar) data[0][index]) / 255.0f;
            }
            else {
                // Otherwise, use the first 3.
                convertedImage.Pixel(x,y,0) = ((uchar) data[0][index]) / 255.0f;
                convertedImage.Pixel(x,y,1) = ((uchar) data[0][index+1]) / 255.0f;
                convertedImage.Pixel(x,y,2) = ((uchar) data[0][index+2]) / 255.0f;
            }

            index += d;
        }
    }

    return true;
}

// Convert CFloatImage to CByteImage.
void convertToByteImage(CFloatImage &floatImage, CByteImage &byteImage) {
    CShape sh = floatImage.Shape();

    assert(floatImage.Shape().nBands == byteImage.Shape().nBands);
    for (int y=0; y<sh.height; y++) {
        for (int x=0; x<sh.width; x++) {
            for (int c=0; c<sh.nBands; c++) {
                float value = floor(255*floatImage.Pixel(x,y,c) + 0.5f);

                if (value < byteImage.MinVal()) {
                    value = byteImage.MinVal();
                }
                else if (value > byteImage.MaxVal()) {
                    value = byteImage.MaxVal();
                }

                // We have to flip the image and reverse the color
                // channels to get it to come out right.  How silly!
                byteImage.Pixel(x,sh.height-y-1,sh.nBands-c-1) = (uchar) value;
            }
        }
    }
}

// Compute SSD distance between two vectors.
double distanceSSD(const vector<double> &v1, const vector<double> &v2) {
    int m = v1.size();
    int n = v2.size();

    if (m != n) {
        // Here's a big number.
        return 1e100;
    }

    double dist = 0;

    for (int i=0; i<m; i++) {
        dist += pow(v1[i]-v2[i], 2);
    }


    return sqrt(dist);
}

// Transform point by homography.
void applyHomography(double x, double y, double &xNew, double &yNew, double h[9]) {
    double d = h[6]*x + h[7]*y + h[8];

    xNew = (h[0]*x + h[1]*y + h[2]) / d;
    yNew = (h[3]*x + h[4]*y + h[5]) / d;
}

// Evaluate a match using a ground truth homography.  This computes the
// average SSD distance between the matched feature points and
// the actual transformed positions.
double evaluateMatch(const FeatureSet &f1, const FeatureSet &f2, const vector<FeatureMatch> &matches, double h[9]) {
    double d = 0;
    int n = 0;

    double xNew;
    double yNew;

    unsigned int num_matches = matches.size();
    for (unsigned int i=0; i<num_matches; i++) {
        int id1 = matches[i].id1;
        int id2 = matches[i].id2;
        applyHomography(f1[id1].x, f1[id1].y, xNew, yNew, h);
        d += sqrt(pow(xNew-f2[id2].x,2)+pow(yNew-f2[id2].y,2));
        n++;
    }	

    return d / n;
}

void addRocData(const FeatureSet &f1, const FeatureSet &f2, const vector<FeatureMatch> &matches, double h[9],
    vector<bool> &isMatch, double threshold, double &maxD) 
{
    double d = 0;

    double xNew;
    double yNew;

    unsigned int num_matches = matches.size();
    for (unsigned int i=0; i<num_matches; i++) {
        int id1 = matches[i].id1;
        int id2 = matches[i].id2;
        applyHomography(f1[id1].x, f1[id1].y, xNew, yNew, h);

        // Ignore unmatched points.  There might be a better way to
        // handle this.
        d = sqrt(pow(xNew-f2[id2].x,2)+pow(yNew-f2[id2].y,2));
        if (d<=threshold) {
            isMatch.push_back(1);
        } else {
            isMatch.push_back(0);
        }

        if (matches[i].distance > maxD)
            maxD = matches[i].distance;
    }	
}

vector<ROCPoint> computeRocCurve(vector<FeatureMatch> &matches,vector<bool> &isMatch,vector<double> &thresholds)
{
    vector<ROCPoint> dataPoints;

    for (int i=0; i < (int)thresholds.size();i++)
    {
        //printf("Checking threshold: %lf.\r\n",thresholds[i]);
        int tp=0;
        int actualCorrect=0;
        int fp=0;
        int actualError=0;
        int total=0;

        int num_matches = (int) matches.size();
        for (int j=0;j < num_matches;j++) {
            if (isMatch[j]) {
                actualCorrect++;
                if (matches[j].distance < thresholds[i]) {
                    tp++;
                }
            } else {
                actualError++;
                if (matches[j].distance < thresholds[i]) {
                    fp++;
                }
            }

            total++;
        }

        ROCPoint newPoint;
        //printf("newPoints: %lf,%lf",newPoint.trueRate,newPoint.falseRate);
        newPoint.trueRate=(double(tp)/actualCorrect);
        newPoint.falseRate=(double(fp)/actualError);
        //printf("newPoints: %lf,%lf",newPoint.trueRate,newPoint.falseRate);

        dataPoints.push_back(newPoint);
    }

    return dataPoints;
}

// Compute AUC given a ROC curve
double computeAUC(vector<ROCPoint> &results)
{
    double auc=0;
    double xdiff,ydiff;
    for (int i = 1; i < (int) results.size(); i++)
    {
        //fprintf(stream,"%lf\t%lf\t%lf\n",thresholdList[i],results[i].falseRate,results[i].trueRate);
        xdiff=(results[i].falseRate-results[i-1].falseRate);
        ydiff=(results[i].trueRate-results[i-1].trueRate);
        auc=auc+xdiff*results[i-1].trueRate+xdiff*ydiff/2;

    }
    return auc;
}

