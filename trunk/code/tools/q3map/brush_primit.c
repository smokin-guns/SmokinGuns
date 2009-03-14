#include "qbsp.h"


// global flag
int	g_bBrushPrimit;

// NOTE : ComputeAxisBase here and in editor code must always BE THE SAME !
// WARNING : special case behaviour of atan2(y,x) <-> atan(y/x) might not be the same everywhere when x == 0
// rotation by (0,RotY,RotZ) assigns X to normal
void ComputeAxisBase(vec3_t normal,vec3_t texX,vec3_t texY)
{
	vec_t RotY,RotZ;
	// do some cleaning
	if (fabs(normal[0])<1e-6)
		normal[0]=0.0f;
	if (fabs(normal[1])<1e-6)
		normal[1]=0.0f;
	if (fabs(normal[2])<1e-6)
		normal[2]=0.0f;
	// compute the two rotations around Y and Z to rotate X to normal
	RotY=-atan2(normal[2],sqrt(normal[1]*normal[1]+normal[0]*normal[0]));
	RotZ=atan2(normal[1],normal[0]);
	// rotate (0,1,0) and (0,0,1) to compute texX and texY
	texX[0]=-sin(RotZ);
	texX[1]=cos(RotZ);
	texX[2]=0;
	// the texY vector is along -Z ( T texture coorinates axis )
	texY[0]=-sin(RotY)*cos(RotZ);
	texY[1]=-sin(RotY)*sin(RotZ);
	texY[2]=-cos(RotY);
}
