/* iScissor.cpp */
/* Main file for implementing project 1.  See TODO statments below
 * (see also correlation.cpp and iScissor.h for additional TODOs) */

#include <assert.h>

#include "correlation.h"
#include "iScissor.h"
#include "PriorityQueue.h"

const double linkLengths[8] = { 1.0, SQRT2, 1.0, SQRT2, 1.0, SQRT2, 1.0, SQRT2 };

// select variables for which filter kernels to use
const int knlASel = 0;
const int knlBSel = 1;
const int knlCSel = 2;
const int knlDSel = 3;

// two inlined routines that may help;

inline Node& NODE(Node* n, int i, int j, int width)
{
    return *(n + j * width + i);
}

inline unsigned char PIXEL(const unsigned char* p, int i, int j, int c, int width)
{
    return *(p + 3 * (j * width + i) + c);
}

/**
 *InitNodeBuf
 *	INPUT:
 *		img      :	a RGB image of size imgWidth by imgHeight;
 *		imgWidth :	the width of the image
 *		imgHeight:	the height of the image
 *		nodes    :	a allocated buffer of Nodes of the same size, one node corresponds to a pixel in img;
 *  OUPUT:
 *      initializes the column, row, and linkCost fields of each node in the node buffer.
 */

void InitNodeBuf(Node* nodes, const unsigned char* img, int imgWidth, int imgHeight)
{
	double *resultImg = new double[imgWidth * imgHeight * 3];
	double* pxl;
	double maxD;
	int i, j, k;
	int nodeIndex, imgIndex;

	// MODIFY THIS VARIABLE TO CHOOSE FILTER KERNELS TO USE
	int knlSel = knlDSel;
	
	Node* node;
	
	// initialize column, row, state, linkCost and pathLength fields of each node in the buffer
    for (j = 0; j < imgHeight; j++) {
        for (i = 0; i < imgWidth; i++) {
			nodeIndex = j * imgWidth + i;
			node = nodes + nodeIndex;
			node->state = INITIAL;
			node->row = j;
			node->column = i;
			node->pathLength = 0;
		}
	}
	
	// applying filter kernels for each direction and calculating the corresponding link costs for each node
	for (k = 0; k < 9; k++) {

		// based on knlSel apply the corresponding filter kernels
		switch (knlSel) {
			case knlASel:	// Original 3x3 filter kernels
				image_filter(resultImg, img, NULL, imgWidth, imgHeight, &Akernels[k][0], 3, 3, 1, 0);
				break;
			case knlBSel:	// 5x5 filter kernels resulting from combining original 3x3 kernels with 3x3 average kernel
				image_filter(resultImg, img, NULL, imgWidth, imgHeight, &Bkernels[k][0], 5, 5, 1, 0);
				break;  
			case knlCSel:	// 5x5 filter kernels resulting from combining original 3x3 kernels with 3x3 disk kernel
				image_filter(resultImg, img, NULL, imgWidth, imgHeight, &Ckernels[k][0], 5, 5, 1, 0);
				break;    
			case knlDSel:	// 5x5 filter kernels resulting from combining original 3x3 kernels with 3x3 gaussian kernel (sigma = 0.5)
				image_filter(resultImg, img, NULL, imgWidth, imgHeight, &Dkernels[k][0], 5, 5, 1, 0);
				break;
			default:		// Default: Original 3x3 filter kernels
				image_filter(resultImg, img, NULL, imgWidth, imgHeight, &Akernels[k][0], 3, 3, 1, 0);
				break;
		}
		
		// iterate through pixels to find the maximum magnitude of intensity derivative maxD 
		maxD = 0;
		for (i = 0; i < imgWidth*imgHeight; i++) {
			imgIndex = 3 * i;
			pxl = resultImg + imgIndex;
			maxD = max(maxD, sqrt((pxl[0]*pxl[0] + pxl[1]*pxl[1] + pxl[2]*pxl[2])/3));
		}

		// iterate through the pixels and update the corresponding link costs for each node
		for (j = 0; j < imgHeight; j++) {
			for (i = 0; i < imgWidth; i++) {
				nodeIndex = j * imgWidth + i;
				imgIndex = 3 * nodeIndex;

				node = nodes + nodeIndex;
				pxl = resultImg + imgIndex;
				// update link costs according to the following formulas
				//   DR(link1) = |img(i+1,j)-img(i,j-1)|/sqrt(2)
				//   D(link) = sqrt( ( DR(link)^2+DG(link)^2+DB(link)^2 )/3 )
				//   cost(link) = (maxD-D(link))*length(link)
				node->linkCost[k] = (maxD - sqrt((pxl[0]*pxl[0] + pxl[1]*pxl[1] + pxl[2]*pxl[2])/3)) * linkLengths[k];
			}
		}
	}
}

static int offsetToLinkIndex(int dx, int dy)
{
    int indices[9] = { 3, 2, 1, 4, -1, 0, 5, 6, 7 };
    int tmp_idx = (dy + 1) * 3 + (dx + 1);
    assert(tmp_idx >= 0 && tmp_idx < 9 && tmp_idx != 4);
    return indices[tmp_idx];
}

/**
 *LiveWireDP:
 *	INPUT:
 *		seedX, seedY:	seed position in nodes
 *		nodes:			node buffer of size width by height;
 *      width, height:  dimensions of the node buffer;
 *		selection:		if selection != NULL, search path only in the subset of nodes[j*width+i] if selection[j*width+i] = 1;
 *						otherwise, search in the whole set of nodes.
 *		numExpanded:		compute the only the first numExpanded number of nodes; (for debugging)
 *	OUTPUT:
 *		computes the minimum path tree from the seed node, by assigning
 *		the prevNode field of each node to its predecessor along the minimum
 *		cost path from the seed to that node.
 */

void LiveWireDP(int seedX, int seedY, Node* nodes, int width, int height, const unsigned char* selection, int numExpanded)
{
	CTypedPtrHeap<Node> *pq = new CTypedPtrHeap<Node>();
	
	// iterate through all nodes in node buffer and initialize state, prevNode, and pathLength fields
	int i;
    for (i = 0; i < width*height; i++) {
		Node* node = nodes + i;
		node->state = INITIAL;
		node->prevNode = NULL;
		node->pathLength = 0;
	}

	// set total cost for seed node to be 0
	Node* seed = &nodes[seedX + seedY*width];
	seed->totalCost = 0;

	// insert seed into priority queue
	pq->Insert(seed);
	
	int dX, dY, newX, newY;

	while(!pq->IsEmpty()) {
		// extract the node with the minimum total cost in priority queue and mark the node as EXPANDED
		Node* current = pq->ExtractMin();
		current->state = EXPANDED;

		// iterate through the neighbor nodes for current node 
		for(i = 0; i < 8; i++) {
			// find the corresponding x.y coordinates for the neighbor node
			current->nbrNodeOffset(dX, dY, i);
			newX = current->column + dX;
			newY = current->row + dY;

			if(newX < 0 || newX >= width || newY < 0 || newY >= height) {
				// if the corresponding x,y coordinates point to an out-of-bound location, skip the following operations
				continue;
			}
			
			Node* neighbor = &nodes[newX + newY*width];
			
			// if the neighbor node is in INITIAL state, mark it as ACTIVE, insert it into priority queue with the sum of the current node's total cost 
			// and corresponding link cost as it's total cost, set current node as the prevNode, and set pathLength to be current node's pathLength 
			// incremented by one
			if(neighbor->state == INITIAL) {
				neighbor->totalCost = current->totalCost + current->linkCost[i];
				neighbor->state = ACTIVE;
				neighbor->prevNode = current;
				neighbor->pathLength = current->pathLength + 1;
				pq->Insert(neighbor);
			}
			// if the neighbor node is in ACTIVE state, check if the sum of the current node's total cost and corresponding link cost is less than the 
			// total cost of the neighbor node, update total cost, prevNode, and pathLength fields
			else if(neighbor->state == ACTIVE) {
				if(neighbor->totalCost > (current->totalCost + current->linkCost[i])) {
					neighbor->totalCost = current->totalCost + current->linkCost[i];
					neighbor->prevNode = current;
					neighbor->pathLength = current->pathLength + 1;
				}
				// in the case of a tie, check to see if the path through th current node is shorter than the one currently set for neighbor node
				// and if so update prevNode and pathLength accordingly
				else if(neighbor->totalCost == (current->totalCost + current->linkCost[i]))
				{
					if(neighbor->pathLength > current->pathLength + 1)
					{
						neighbor->prevNode = current;
						neighbor->pathLength = current->pathLength + 1;
					}
				}
			}
		}
	}
}

/**
 *MinimumPath:
 *	INPUT:
 *		nodes:				a node buffer of size width by height;
 *		width, height:		dimensions of the node buffer;
 *		freePtX, freePtY:	an input node position;
 *	OUTPUT:
 *		insert a list of nodes along the minimum cost path from the seed node to the input node.
 *		Notice that the seed node in the buffer has a NULL predecessor.
 *		And you want to insert a *pointer* to the Node into path, e.g.,
 *		insert nodes+j*width+i (or &(nodes[j*width+i])) if you want to insert node at (i,j), instead of nodes[nodes+j*width+i]!!!
 *		after the procedure, the seed should be the head of path and the input code should be the tail
 */

void MinimumPath(CTypedPtrDblList <Node>* path, int freePtX, int freePtY, Node* nodes, int width, int height)
{
	// add current node to tail of path
	Node* current = &nodes[freePtX + freePtY*width];
	path->AddTail(current);

	// add each node's prevNode to head of path until reaching a node whose prevNode is NULL (the seed node)
	while(current->prevNode != NULL) {
		current = current->prevNode;
		path->AddHead(current);
	}
}

/************************ An Extra Credit Item ***************************
 *SeeSnap:
 *	INPUT:
 *		img:				a RGB image buffer of size width by height;
 *		width, height:		dimensions of the image buffer;
 *		x,y:				an input seed position;
 *	OUTPUT:
 *		update the value of x,y to the closest edge based on local image information.
 */

void SeedSnap(int& x, int& y, unsigned char* img, int width, int height)
{
    printf("SeedSnap in iScissor.cpp: to be implemented for extra credit!\n");
}

//generate a cost graph from original image and node buffer with all the link costs;
void MakeCostGraph(unsigned char* costGraph, const Node* nodes, const unsigned char* img, int imgWidth, int imgHeight)
{
    int graphWidth = imgWidth * 3;
    int graphHeight = imgHeight * 3;
    int dgX = 3;
    int dgY = 3 * graphWidth;

    int i, j;
    for (j = 0; j < imgHeight; j++) {
        for (i = 0; i < imgWidth; i++) {
            int nodeIndex = j * imgWidth + i;
            int imgIndex = 3 * nodeIndex;
            int costIndex = 3 * ((3 * j + 1) * graphWidth + (3 * i + 1));

            const Node* node = nodes + nodeIndex;
            const unsigned char* pxl = img + imgIndex;
            unsigned char* cst = costGraph + costIndex;

            cst[0] = pxl[0];
            cst[1] = pxl[1];
            cst[2] = pxl[2];

            //r,g,b channels are grad info in seperate channels;
            DigitizeCost(cst	   + dgX, node->linkCost[0]);
            DigitizeCost(cst - dgY + dgX, node->linkCost[1]);
            DigitizeCost(cst - dgY      , node->linkCost[2]);
            DigitizeCost(cst - dgY - dgX, node->linkCost[3]);
            DigitizeCost(cst	   - dgX, node->linkCost[4]);
            DigitizeCost(cst + dgY - dgX, node->linkCost[5]);
            DigitizeCost(cst + dgY	   ,  node->linkCost[6]);
            DigitizeCost(cst + dgY + dgX, node->linkCost[7]);
        }
    }
}