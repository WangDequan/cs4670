#ifndef iScissor_H
#define iScissor_H

#include "imgflt.h"

const int INITIAL = 0;
const int ACTIVE = 1;
const int EXPANDED = 2;

// added by dewey
const double SQRT2 = 1.4142135623730950488016887242097;
const double SQINV = 1.0 / SQRT2;

// The eight filter kernels
///////////////////////////////////////////////////////////////////////
// TODO: put all the correct numbers where they belong in the 3 x 3 filter kernels
// hint: you may want to make use of one of the constants defined above

const double kernels[8][9] = {
    {
        // 0
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000
    },
    {
        // 1
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000
    },
    {
        // 2
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000
    },
    {
        // 3
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000
    },
    {
        // 4
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000
    },
    {
        // 5
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000
    },
    {
        // 6
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000
    },
    {
        // 7
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000 ,
        0.0000, 0.0000, 0.0000
    }
};

struct Node {
    //column and row:	remember the position of the node in original image so that its neighboring nodes can be located.
    //linkCost:			contains the costs of each link, as described in project page and in the SIGGRAPH95 paper.
    //state:			used to tag the node as being INITIAL, ACTIVE, or EXPANDED during the min-cost tree computation.
    //totalCost:		the minimum total cost from this node to the seed node.
    //prevNode:			points to its predecessor along the minimum cost path from the seed to that node.

    int column, row;
    double linkCost[8];
    int state;
    double totalCost;
    Node* prevNode;

    Node () {
        prevNode = NULL;
    }
    // this function helps to locate neighboring node in node buffer.
    void nbrNodeOffset(int& offsetX, int& offsetY, int linkIndex) {
        /*
         *  321
         *  4 0
         *  567
         */

        if (linkIndex == 0) {
            offsetX = 1;
            offsetY = 0;
        } else if (linkIndex == 1) {
            offsetX = 1;
            offsetY = -1;
        } else if (linkIndex == 2) {
            offsetX = 0;
            offsetY = -1;
        } else if (linkIndex == 3) {
            offsetX = -1;
            offsetY = -1;
        } else if (linkIndex == 4) {
            offsetX = -1;
            offsetY = 0;
        } else if (linkIndex == 5) {
            offsetX = -1;
            offsetY = 1;
        } else if (linkIndex == 6) {
            offsetX = 0;
            offsetY = 1;
        } else if (linkIndex == 7) {
            offsetX = 1;
            offsetY = 1;
        }
    }

    // used by the binary heap operation,
    // pqIndex is the index of this node in the heap.
    // you don't need to know about it to finish the assignment;

    int pqIndex;

    int& Index(void) {
        return pqIndex;
    }

    int Index(void) const {
        return pqIndex;
    }
};

inline int operator < (const Node& a, const Node& b)
{
    return a.totalCost < b.totalCost;
}


void InitNodeBuf(Node* nodes, const unsigned char* img, int width, int height);
double CrossCorrelate(const double filterKernel[3][3], const unsigned char* img, int imgWidth, int imgHeight, int x, int y, int c);
void LiveWireDP(int seedX, int seedY, Node* nodes, int width, int height, const unsigned char* selection, int expanded);

void MinimumPath(CTypedPtrDblList <Node>* path, int freePtX, int freePtY, Node* nodes, int width, int height);

void SeedSnap(int& x, int& y, unsigned char* img, int width, int height);






inline void DigitizeCost(unsigned char* cst, double lCost)
{
    cst[0] = cst[1] = cst[2] = (unsigned char)(floor(__max(0.0, __min(255.0, lCost))));
}

void MakeCostGraph(unsigned char* costGraph, const Node* nodes, const unsigned char* img, int width, int height);

//students don't need this data structure in assignment for the required work.
struct Seed {
    int x, y;
    int seed; //seed = 1, if it is a seed, otherwise, it is a ordinary contour location;

    Seed(int i, int j) {
        x = i;
        y = j;
    }
    Seed(int i, int j, int s) {
        x = i;
        y = j;
        seed = s;
    }
    Seed(void) {
    }
};

#endif
