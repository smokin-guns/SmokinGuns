#include <stdio.h>
#include "cmdlib.h"
#include "mathlib.h"
#include "qfiles.h"

void MD3_Dump( const char *filename );
void MD3_ComputeTagFromTri( md3Tag_t *pTag, const float tri[3][3] );