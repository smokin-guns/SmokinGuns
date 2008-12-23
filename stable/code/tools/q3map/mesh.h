
// mesh.h


typedef struct {
	int			width, height;
	drawVert_t	*verts;
} mesh_t;

#define	MAX_EXPANDED_AXIS		128

extern	int	originalWidths[MAX_EXPANDED_AXIS];
extern	int	originalHeights[MAX_EXPANDED_AXIS];

void FreeMesh( mesh_t *m );
mesh_t *CopyMesh( mesh_t *mesh );
void PrintMesh( mesh_t *m );
mesh_t *TransposeMesh( mesh_t *in );
void InvertMesh( mesh_t *m );
mesh_t *SubdivideMesh( mesh_t in, float maxError, float minLength );
mesh_t *SubdivideMeshQuads( mesh_t *in, float minLength, int maxsize, int widthtable[], int heighttable[]);
mesh_t *RemoveLinearMeshColumnsRows( mesh_t *in );
void MakeMeshNormals( mesh_t in );
void PutMeshOnCurve( mesh_t in );


void MakeNormalVectors (vec3_t forward, vec3_t right, vec3_t up);
