#ifndef PRECISION_RECALL_H
#define PRECISION_RECALL_H

#include "Common.h"

// A single point in the Precision Recall curve
typedef struct {
	float precision, recall, threshold;
} PecisionRecallPoint;

// Computes and stores a precision recall curve
class PrecisionRecall
{
private:
	std::vector<PecisionRecallPoint> _data;
	float _averagePrecision;

public:
	// Computes precision recall given a set of gound truth labels in gt and
	// a set of predictions made by our classifier in preds.
	PrecisionRecall(const std::vector<float>& gt, const std::vector<float>& preds);

	// Returns area under the curve
	double getAveragePrecision() const { return _averagePrecision; }

	// Find threshold that results in highest F1 measure
	double getBestThreshold() const;

	// Save curve points to a .pr file. See inclded plot_pr.m file for a
	// MATLAB script that can plot this data.
	void save(const char* filename) const;
};

#endif // PRECISION_RECALL_H