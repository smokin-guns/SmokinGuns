
#include "cmdlib.h"
#include "mathlib.h"
#include "polyset.h"

void		ASE_Load( const char *filename, qboolean verbose, qboolean meshanims );
int			ASE_GetNumSurfaces( void );
polyset_t	*ASE_GetSurfaceAnimation( int ndx, int *numFrames, int skipFrameStart, int skipFrameEnd, int maxFrames );
const char  *ASE_GetSurfaceName( int ndx );
void		ASE_Free( void );
