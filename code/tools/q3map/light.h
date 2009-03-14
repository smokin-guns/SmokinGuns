
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "polylib.h"
#include "imagelib.h"
#include "threads.h"
#include "scriplib.h"

#include "shaders.h"
#include "mesh.h"

//added by Spoon
void WriteTexFile(char *name);
extern	char		source[1024];

typedef enum
{
	emit_point,
	emit_area,
	emit_spotlight,
	emit_sun
} emittype_t;

#define	MAX_LIGHT_EDGES		8
typedef struct light_s
{
	struct light_s *next;
	emittype_t	type;
	struct shaderInfo_s	*si;

	vec3_t		origin;
	vec3_t		normal;		// for surfaces, spotlights, and suns
	float		dist;		// plane location along normal

	qboolean	linearLight;
	int			photons;
	int			style;
	vec3_t		color;
	float		radiusByDist;	// for spotlights

	qboolean	twosided;		// fog lights both sides

	winding_t	*w;
	vec3_t		emitColor;		// full out-of-gamut value
} light_t;


extern	float	lightscale;
extern	float	ambient;
extern	float	maxlight;
extern	float	direct_scale;
extern	float	entity_scale;

extern	qboolean	noSurfaces;

//===============================================================

// light_trace.c

// a facet is a subdivided element of a patch aproximation or model
typedef struct cFacet_s {
	float	surface[4];
	int		numBoundaries;		// either 3 or 4, anything less is degenerate
	float	boundaries[4][4];	// positive is outside the bounds

	vec3_t	points[4];			// needed for area light subdivision

	float	textureMatrix[2][4];	// compute texture coordinates at point of impact for translucency
} cFacet_t;

typedef struct {
	vec3_t		mins, maxs;
	vec3_t		origin;
	float		radius;

	qboolean	patch;

	int			numFacets;
	cFacet_t	*facets;

	shaderInfo_t	*shader;		// for translucency
} surfaceTest_t;


typedef struct {
	vec3_t		filter;				// starts out 1.0, 1.0, 1.0, may be reduced if
									// transparent surfaces are crossed

	vec3_t		hit;				// the impact point of a completely opaque surface
	float		hitFraction;		// 0 = at start, 1.0 = at end
	qboolean	passSolid;
} trace_t;

extern	surfaceTest_t	*surfaceTest[MAX_MAP_DRAW_SURFS];

void	InitTrace( void );

// traceWork_t is only a parameter to crutch up poor large local allocations on
// winNT and macOS.  It should be allocated in the worker function, but never
// looked at.
typedef struct {
	vec3_t		start, end;
	int			numOpenLeafs;
	int			openLeafNumbers[MAX_MAP_LEAFS];
	trace_t		*trace;
	int			patchshadows;
} traceWork_t;

void TraceLine( const vec3_t start, const vec3_t stop, trace_t *trace,
			   qboolean testAll, traceWork_t *tw );
qboolean PointInSolid( vec3_t start );

//===============================================================

//===============================================================


typedef struct {
	int		textureNum;
	int		x, y, width, height;

	// for patches
	qboolean	patch;
	mesh_t		mesh;

	// for faces
	vec3_t	origin;
	vec3_t	vecs[3];
} lightmap_t;


