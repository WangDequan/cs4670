#include "svm.h"
#include "svmmath.h"

int 
main(int argc, char **argv)
{    

    Fl::visual(FL_DOUBLE|FL_RGB);

	svmUI svmUI;
    svmUI.show();

#if 0
	SVMPoint a,b,c,d;
	a.u = 147.5; a.v = 147.5; a.w = 1;
	a.X = -1; a.Y = -1; a.Z = 0; a.W = 1;

	b.u = 394.5; b.v = 209.5; b.w = 1;
	b.X = -2; b.Y = -1; b.Z = 0; b.W = 1;

	c.u = 103.5; c.v = 261.5; c.w = 1;
	c.X = -1; c.Y = -2; c.Z = 0; c.W = 1;

	d.u = 311.5; d.v = 305.5; d.w = 1;
	d.X = -2; d.Y = -2; d.Z = 0; d.W = 1;

	std::vector<SVMPoint> v;

	v.push_back(a); v.push_back(c);v.push_back(d); v.push_back(b);
#endif
	/*
	double H[3][3], Hinv[3][3];

	ComputeHomography(H, Hinv, v, false);

	printf("[ %f %f %f; %f %f %f; %f %f %f ]\n", H[0][0], 
		H[0][1], H[0][2], H[1][0], H[1][1], H[1][2], 
		H[2][0], H[2][1], H[2][2]);
	*/

	return Fl::run();
}


void 
my_svm(char helpInfo[2048])
{
	char info[2048] = {"\
Push a point onto the stack: \n\
	spacebar (while pointing at point)\n\
Pop a point from the stack: \n\
	spacebar (while pointing at empty space)\n\
Snap to a guideline: \n\
	hold down Ctrl\n\
Drag the image: \n\
	drag with right button down; \n\
\n\
"};

	strcpy(helpInfo,info);
}


