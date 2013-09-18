/* iScissor.cpp */
/* Main file for implementing project 1.  See TODO statments below
 * (see also correlation->cpp and iScissor.h for additional TODOs) */

#include <assert.h>
#include <math.h>

#include "correlation.h"
#include "iScissor.h"

const double linkLengths[8] = { 1.0, SQRT2, 1.0, SQRT2, 1.0, SQRT2, 1.0, SQRT2 };

// two inlined routines that may help;

inline Node& NODE(Node* n, int i, int j, int width)
{
    return *(n + j * width + i);
}

inline unsigned char PIXEL(const unsigned char* p, int i, int j, int c, int width)
{
    return *(p + 3 * (j * width + i) + c);
}

/************************ TODO 1 ***************************
 *InitNodeBuf
 *	INPUT:
 *		img:	a RGB image of size imgWidth by imgHeight;
 *		nodes:	a allocated buffer of Nodes of the same size, one node corresponds to a pixel in img;
 *  OUPUT:
 *      initializes the column, row, and linkCost fields of each node in the node buffer.
 */

void InitNodeBuf(Node* nodes, const unsigned char* img, int imgWidth, int imgHeight)
{
  double px[3];
  double maxD = 0;
  for (int y=0;y<imgHeight;y++){
    for (int x=0;x<imgWidth;x++){
      NODE(nodes, x, y, imgWidth).column = x;
      NODE(nodes, x, y, imgWidth).row = y;
      for (int i=0;i<4;i++){
        pixel_filter(px, x, y, img, imgWidth, imgHeight, kernels[i], 3, 3, 1, 0);
        int v = sqrt((px[0]*px[0] + px[1]*px[1] + px[2]*px[2])/3);
        NODE(nodes, x, y, imgWidth).linkCost[i] = v;
        int dx, dy;
        switch(i){
        case 0: dx = 1; dy = 0; break;
        case 1: dx = 1; dy = 1; break;
        case 2: dx = 0; dy = 1; break;
        case 3: dx = -1; dy = 1; break;
        }
        if (((x+dx) > -1)&&((x+dx)<imgWidth)&&((y+dy)>-1)&&((y+dy)<imgHeight)){
          NODE(nodes, x + dx, y + dy, imgWidth).linkCost[i+4] = v;
        }
        maxD = __max(maxD, v);
        assert(v >= px[0] * sqrt(1/3));
        assert(v >= px[1] * sqrt(1/3));
        assert(v >= px[2] * sqrt(1/3));
      }
    }
  }
  for (int y=0;y<imgHeight;y++){
    for (int x=0;x<imgWidth;x++){
      for (int i=0;i<8;i++){
        double* c = &(NODE(nodes, x, y, imgWidth).linkCost[i]);
        *c = (maxD - *c) * linkLengths[i];
      }
    }
  }
}
/************************ END OF TODO 1 ***************************/

static int offsetToLinkIndex(int dx, int dy)
{
    int indices[9] = { 3, 2, 1, 4, -1, 0, 5, 6, 7 };
    int tmp_idx = (dy + 1) * 3 + (dx + 1);
    assert(tmp_idx >= 0 && tmp_idx < 9 && tmp_idx != 4);
    return indices[tmp_idx];
}

/************************ TODO 4 ***************************
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
  CTypedPtrHeap<Node> pq;
  for (int i=0;i<width*height;i++){ nodes[i].state = INITIAL;  }
  NODE(nodes, seedX, seedY, width).totalCost = 0;
  NODE(nodes, seedX, seedY, width).prevNode = NULL;
  pq.Insert(&NODE(nodes,seedX,seedY,width));
  
  Node* cur;
  int dx, dy;
  while (!pq.IsEmpty() && numExpanded > 0){
    cur = pq.ExtractMin();
    cur->state = EXPANDED;
    for(int i=0;i<8;i++){
      cur->nbrNodeOffset( dx, dy, i);
      int x2 = cur->column+dx;
      int y2 = cur->row + dy;
      if (x2 > -1 && x2 < width && y2 > -1 && y2 < height && (selection == NULL || selection[y2*width+x2])){
        Node* n = &NODE(nodes, cur->column + dx, cur->row + dy, width);
        int v = cur->totalCost + cur->linkCost[i];
        switch(n->state){
        case INITIAL:
          n->totalCost = v;
          n->state = ACTIVE;
          n->prevNode = cur;
          pq.Insert(n);
          break;
        case ACTIVE:
          if (n->totalCost > v) {
            n->totalCost = v;
            n->prevNode = cur;
            pq.Update(n);
          }
          break;
        }
      }
    }
  }
}
/************************ END OF TODO 4 ***************************/

/************************ TODO 5 ***************************
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
  Node* cur = &NODE(nodes, freePtX, freePtY, width);
  assert(path->IsEmpty());
  while (cur != NULL){
    path->AddHead(cur);
    cur = cur->prevNode;
  }
}
/************************ END OF TODO 5 ***************************/

/************************ An Extra Credit Item ***************************
 *SeedSnap:
 *	INPUT:
 *		img:				a RGB image buffer of size width by height;
 *		width, height:		dimensions of the image buffer;
 *		x,y:				an input seed position;
 *	OUTPUT:
 *		update the value of x,y to the closest edge based on local image information->
 */

void SeedSnap(int& x, int& y, unsigned char* img, int width, int height)
{
  const double xSobel[9] = {
    1.0000, 0.0000, -1.0000,
    2.0000, 0.0000, -2.0000,
    1.0000, 0.0000, -1.0000
  };
  const double ySobel[9] = {
    1.0000, 2.0000, 1.0000,
    0.0000, 0.0000, 0.0000,
   -1.0000,-2.0000,-1.0000
  };
  double* xGrad = new double[3 * width * height];
  double* yGrad = new double[3 * width * height];
  double* gradient = new double[width * height];
  double maxGrad = 0.0;
  int idx, row, col;
  for (row = 0; row < height; row++) {
    for (col = 0; col < width; col++) {
      idx = 3 * (row * width + col);
      pixel_filter(xGrad + idx, row, col, img, width, height, xSobel, 3, 3, 8, 0);
      pixel_filter(yGrad + idx, row, col, img, width, height, ySobel, 3, 3, 8, 0);
      gradient[idx / 3] = sqrt((pow(xGrad[idx+0],2) + pow(xGrad[idx+1],2) + pow(xGrad[idx+2],2) +
                                pow(yGrad[idx+0],2) + pow(yGrad[idx+1],2) + pow(yGrad[idx+2],2))/3);
      maxGrad = std::max(maxGrad, gradient[idx / 3]);
    }
  }
  double delta;
  double threshold = 0.4;
  bool found = false;
  int maxD = std::max(x,std::max(y,std::max(width-x,height-y)));
  int d = 0;
  while (++d < maxD && !found) {
    int i = -d - 1;
    while (++i <= d && !found) {
      int j = -d - 1;
      while (++j <= d && !found) {
        if (abs(i) == d || abs(j) == d) {
          row = y + j;
          col = x + i;
          if (col > 0 && row > 0 && col < width-1 && row < height-1) {
            idx = row * width + col;
            delta = maxGrad - gradient[idx];
            printf("(%d,%d) %f\n", col, row, delta);
            if (delta/maxGrad <= threshold) {
              printf("Delta: %f, Max: %f\n", delta, maxGrad);
              printf("New (x,y): (%d,%d) %f\n", col, row, delta/maxGrad);
              found = true;
              x = col;
              y = row;
            }
          }
        }
      }
    }
  }

  // for (row = 1; row < height-1; row++) {
  //   for (col = 1; col < width-1; col++) {
  //     idx = row * width + col;
  //     delta = maxGrad - gradient[idx];
  //     dist = sqrt(pow(row - y, 2) + pow(col - x, 2));
  //     cost = pow(delta/maxGrad, 2) + dist/sqrt((double)width*width + (double)height*height);
  //     if (cost < minCost && col != x && row != y) {
  //       minCost = cost;
  //       newX = col;
  //       newY = row;
  //       printf("New (x,y): (%d,%d) %f\n", col, row, cost);
  //     }
  //   }
  // }
  // x = newX;
  // y = newY;
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

