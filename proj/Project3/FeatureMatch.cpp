/* FeatureMatch.cpp */
/* Match features */

#include <math.h>

#include "FeatureSet.h"
#include "FeatureMatch.h"

#include "anniface.h"
#include "vector.h"

void fastMatchFeatures(const FeatureSet &f1, const FeatureSet &f2, vector<FeatureMatch> &matches) 
{
    int m = f1.size();
    int n = f2.size();

    double rsum = 0.0;

    // matches.resize(m);

    /* Create a kd-tree for f2 */
    vec_t *f2_pts = (vec_t *) malloc(sizeof(vec_t) * n);
    
    int dim = f2[0].data.size();

    for (int i = 0; i < n; i++) {
	f2_pts[i] = vec_new(dim);
	for (int j = 0; j < dim; j++) {
	    Vn(f2_pts[i], j) = f2[i].data[j];
	}
    }

    vec_t axis_weights = vec_new(dim);
    for (int i = 0; i < dim; i++) {
	Vn(axis_weights, i) = 1.0;
    }

    ANNkd_tree_t *tree = 
	create_ann_tree(n, dim, f2_pts, axis_weights);

    vec_t q = vec_new(dim);

    int num_correct = 0;

    for (int i=0; i < m; i++) {
	for (int j = 0; j < dim; j++) {
	    Vn(q, j) = f1[i].data[j];
	}

	// double dist;
	// int idx = query_ann_tree_idx(tree, q, 0.0, &dist);
	int idx1, idx2;
	double dist1, dist2;

	query_ann_tree_idx2(tree, q, 0.0, &idx1, &idx2, &dist1, &dist2);

	// printf("ratio = %0.3f\n", dist1 / dist2);
	rsum += dist1 / dist2;

	if ((dist1 / dist2) < 0.6) {
	    FeatureMatch m;

	    m.id1 = f1[i].id;
	    m.id2 = f2[idx1].id;
	    m.score = exp(-dist1);
	    num_correct++;

	    matches.push_back(m);
	}
    }

    /* Cleanup */

    vec_free(q);
    vec_free(axis_weights);
    free_ann_tree(tree);

    for (int i = 0; i < n; i++) {
	vec_free(f2_pts[i]);
    }

    free(f2_pts);
}
