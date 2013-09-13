/* iScissor.cpp */
/* Main file for implementing project 1.  See TODO statments below
 * (see also correlation->cpp and iScissor.h for additional TODOs) */

#include <assert.h>

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
  long double inten[8];
  for (int y=0;y<imgHeight;y++){
    for (int x=0;x<imgWidth;x++){
      NODE(nodes, x, y, imgWidth).column = x;
      NODE(nodes, x, y, imgWidth).row = y;
      long double maxD = 0;
      for (int i=0;i<8;i++){
        pixel_filter(px, x, y, img, imgWidth, imgHeight, kernels[i], 3, 3, 1, 0);
        inten[i] = sqrt((px[0]*px[0] + px[1]*px[1] + px[2]*px[2])/3);
        maxD = __max(maxD, inten[i]);
      }
      for (int i=0;i<8;i++){
        assert(maxD >= inten[i]);
        NODE(nodes, x, y, imgWidth).linkCost[i] = (maxD - inten[i]) * (i % 2 == 1 ? SQRT2 : 1) ;
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
      if (x2 > -1 && x2 < width && y2 > -1 && y2 < height){
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
 *SeeSnap:
 *	INPUT:
 *		img:				a RGB image buffer of size width by height;
 *		width, height:		dimensions of the image buffer;
 *		x,y:				an input seed position;
 *	OUTPUT:
 *		update the value of x,y to the closest edge based on local image information->
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

