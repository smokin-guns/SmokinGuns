#include "qbsp.h"

#define NUM_PREFIXINFO 11 //very important

//prefixInfo-stats
typedef struct {
	char	*name;
	int		surfaceFlags;
} prefixInfo_t;


static prefixInfo_t prefixInfo[] = {
	{ "metal", SURF_METALSTEPS },
	{ "wood", SURF_WOOD },
	{ "cloth", SURF_CLOTH },
	{ "dirt", SURF_DIRT },
	{ "glass", SURF_GLASS},
	{ "plant", SURF_PLANT},
	{ "sand", SURF_SAND},
	{ "snow", SURF_SNOW},
	{ "stone", SURF_STONE},
	{ "water", SURF_WATER},
	{ "grass", SURF_GRASS},
};


//Added by Spoon to recoginze surfaceparms by shadernames
int GetSurfaceParm(const char *tex){
	char surf[MAX_QPATH], tex2[MAX_QPATH];
	int	i, j = 0;

	strcpy(tex2, tex);

	//find last dir
	for(i = 0; i < 64, tex2[i] != '\0'; i++){
		if(tex2[i] == '\\' || tex2[i] == '/')
			j=i+1;
	}

	strcpy(surf, tex2+j);

	for(i=0; i<10; i++){
		if(surf[i] == '_')
			break;
	}
	surf[i] = '\0';

	//_printf("%s\n", surf);

	for(i=0; i < NUM_PREFIXINFO; i++){
		if(!stricmp(surf, prefixInfo[i].name)){
			return prefixInfo[i].surfaceFlags;
		}
	}
	return 0;
}

/*
============
EmitShader
modified by Spoon
============
*/
int	EmitShader( const char *shader ) {
	int				i;
	shaderInfo_t	*si;

	if ( !shader ) {
		shader = "noshader";
	}

	for ( i = 0 ; i < numShaders ; i++ ) {
		if ( !Q_stricmp( shader, dshaders[i].shader ) ) {
			return i;
		}
	}

	if ( i == MAX_MAP_SHADERS ) {
		Error( "MAX_MAP_SHADERS" );
	}
	numShaders++;
	strcpy( dshaders[i].shader, shader );

	si = ShaderInfoForShader( shader );
	dshaders[i].surfaceFlags = si->surfaceFlags;
	dshaders[i].surfaceFlags |= GetSurfaceParm(si->shader);
	dshaders[i].contentFlags = si->contents;

	return i;
}


/*
============
EmitPlanes

There is no oportunity to discard planes, because all of the original
brushes will be saved in the map.
============
*/
void EmitPlanes (void)
{
	int			i;
	dplane_t	*dp;
	plane_t		*mp;

	mp = mapplanes;
	for (i=0 ; i<nummapplanes ; i++, mp++)
	{

		dp = &dplanes[numplanes];
		VectorCopy ( mp->normal, dp->normal);
		dp->dist = mp->dist;
		numplanes++;
	}
}



/*
==================
EmitLeaf
==================
*/
void EmitLeaf (node_t *node)
{
	dleaf_t				*leaf_p;
	bspbrush_t			*b;
	drawSurfRef_t		*dsr;

	// emit a leaf
	if (numleafs >= MAX_MAP_LEAFS)
		Error ("MAX_MAP_LEAFS");

	leaf_p = &dleafs[numleafs];
	numleafs++;

	leaf_p->cluster = node->cluster;
	leaf_p->area = node->area;

	//
	// write bounding box info
	//	
	VectorCopy (node->mins, leaf_p->mins);
	VectorCopy (node->maxs, leaf_p->maxs);
	
	//
	// write the leafbrushes
	//
	leaf_p->firstLeafBrush = numleafbrushes;
	for ( b = node->brushlist ; b ; b = b->next ) {
		if ( numleafbrushes >= MAX_MAP_LEAFBRUSHES ) {
			Error( "MAX_MAP_LEAFBRUSHES" );
		}
		dleafbrushes[numleafbrushes] = b->original->outputNumber;
		numleafbrushes++;
	}
	leaf_p->numLeafBrushes = numleafbrushes - leaf_p->firstLeafBrush;

	//
	// write the surfaces visible in this leaf
	//
	if ( node->opaque ) {
		return;		// no leaffaces in solids
	}
	
	// add the drawSurfRef_t drawsurfs
	leaf_p->firstLeafSurface = numleafsurfaces;
	for ( dsr = node->drawSurfReferences ; dsr ; dsr = dsr->nextRef ) {
		if ( numleafsurfaces >= MAX_MAP_LEAFFACES)
			Error ("MAX_MAP_LEAFFACES");
		dleafsurfaces[numleafsurfaces] = dsr->outputNumber;
		numleafsurfaces++;			
	}


	leaf_p->numLeafSurfaces = numleafsurfaces - leaf_p->firstLeafSurface;
}


/*
============
EmitDrawNode_r
============
*/
int EmitDrawNode_r (node_t *node)
{
	dnode_t	*n;
	int		i;

	if (node->planenum == PLANENUM_LEAF)
	{
		EmitLeaf (node);
		return -numleafs;
	}

	// emit a node	
	if (numnodes == MAX_MAP_NODES)
		Error ("MAX_MAP_NODES");
	n = &dnodes[numnodes];
	numnodes++;

	VectorCopy (node->mins, n->mins);
	VectorCopy (node->maxs, n->maxs);

	if (node->planenum & 1)
		Error ("WriteDrawNodes_r: odd planenum");
	n->planeNum = node->planenum;

	//
	// recursively output the other nodes
	//	
	for (i=0 ; i<2 ; i++)
	{
		if (node->children[i]->planenum == PLANENUM_LEAF)
		{
			n->children[i] = -(numleafs + 1);
			EmitLeaf (node->children[i]);
		}
		else
		{
			n->children[i] = numnodes;	
			EmitDrawNode_r (node->children[i]);
		}
	}

	return n - dnodes;
}

//=========================================================



/*
============
SetModelNumbers
============
*/
void SetModelNumbers (void)
{
	int		i;
	int		models;
	char	value[10];

	models = 1;
	for ( i=1 ; i<num_entities ; i++ ) {
		if ( entities[i].brushes || entities[i].patches ) {
			sprintf ( value, "*%i", models );
			models++;
			SetKeyValue (&entities[i], "model", value);
		}
	}

}

/*
============
SetLightStyles
============
*/
#define	MAX_SWITCHED_LIGHTS	32
void SetLightStyles (void)
{
	int		stylenum;
	const char	*t;
	entity_t	*e;
	int		i, j;
	char	value[10];
	char	lighttargets[MAX_SWITCHED_LIGHTS][64];


	// any light that is controlled (has a targetname)
	// must have a unique style number generated for it

	stylenum = 0;
	for (i=1 ; i<num_entities ; i++)
	{
		e = &entities[i];

		t = ValueForKey (e, "classname");
		if (Q_strncasecmp (t, "light", 5))
			continue;
		t = ValueForKey (e, "targetname");
		if (!t[0])
			continue;
		
		// find this targetname
		for (j=0 ; j<stylenum ; j++)
			if (!strcmp (lighttargets[j], t))
				break;
		if (j == stylenum)
		{
			if (stylenum == MAX_SWITCHED_LIGHTS)
				Error ("stylenum == MAX_SWITCHED_LIGHTS");
			strcpy (lighttargets[j], t);
			stylenum++;
		}
		sprintf (value, "%i", 32 + j);
		SetKeyValue (e, "style", value);
	}
}

//===========================================================

/*
==================
BeginBSPFile
==================
*/
void BeginBSPFile( void ) {
	// these values may actually be initialized
	// if the file existed when loaded, so clear them explicitly
	nummodels = 0;
	numnodes = 0;
	numbrushsides = 0;
	numleafsurfaces = 0;
	numleafbrushes = 0;

	// leave leaf 0 as an error, because leafs are referenced as
	// negative number nodes
	numleafs = 1;
}

//added by spoon to get back the changed surfaceflags
void RestoreSurfaceFlags(char *filename){
	int i;
	FILE	*texfile;
	int		surfaceFlags[MAX_MAP_DRAW_SURFS];
	int		numTexInfos;

	//first parse the tex-file
	texfile = fopen(filename, "r");
	if(texfile){
		fscanf( texfile, "TEXFILE\n%i\n", &numTexInfos);
		//_printf("%i\n", numTexInfos);

		for(i=0; i<numTexInfos; i++){
			vec3_t color;
			fscanf( texfile, "%i %f %f %f\n", &surfaceFlags[i],
				&color[0], &color[1], &color[2]);
			dshaders[i].surfaceFlags = surfaceFlags[i];
			//_printf("%i\n", surfaceFlags[i]);
		}
	} else
		_printf("couldn't find %s not tex-file is now writed without surfaceFlags!\n", filename);
}

void WriteTexFile( char* name){
	FILE			*texfile;
	char			filename[1024];
	int				i;

	sprintf (filename, "%s.tex", name);

	if(!compile_map){
		RestoreSurfaceFlags(filename);
	}

	texfile = fopen (filename, "w");

	fprintf( texfile, "TEXFILE\n");

	fprintf( texfile, "%i", numShaders);
	for ( i = 0 ; i < numShaders ; i++ ) {
		shaderInfo_t	*se = ShaderInfoForShader(dshaders[i].shader);

		fprintf( texfile, "\n%i %f %f %f", dshaders[i].surfaceFlags,
			se->color[0], se->color[1], se->color[2]);

		dshaders[i].surfaceFlags = i;
	}
	fclose(texfile);
}


/*
============
EndBSPFile
============
*/
void EndBSPFile( void ) {
	char	path[1024];

	EmitPlanes ();
	UnparseEntities ();

	// write the map
	sprintf (path, "%s.bsp", source);
	_printf ("Writing %s\n", path);

	//only create tex file if it is the first compile
	WriteTexFile (source);

	WriteBSPFile (path);
}


//===========================================================

void StripName(const char *in, char *out){
	char path[64];
	char name[64];
	int i, j = 0;

	strcpy(path, in);

	for(i=0;i<64;i++){
		if(path[i] == '\\' || path[i] == '/'){
			j = i;
		}
		if(path[i] == '\0')
			break;
	}

	strcpy(name, path+j+1);
	path[j] = '\0';

	sprintf(out, "%s/tex/%s", path, name);
}
#define NOCONTENTS	8
#define	NOSURFPARMS	5

int	nocontents[NOCONTENTS] = {
	CONTENTS_ORIGIN,
	CONTENTS_AREAPORTAL,
	CONTENTS_CLUSTERPORTAL,
	CONTENTS_DONOTENTER,
	CONTENTS_PLAYERCLIP,
	CONTENTS_MONSTERCLIP,
	CONTENTS_BOTCLIP,
	CONTENTS_FOG
};
int nosurfparms[NOSURFPARMS] = {
	SURF_NODRAW,
	SURF_SKY,
	SURF_NOMARKS,
	SURF_NOIMPACT,
	SURF_LADDER
};

qboolean nodraw(int contents, int surfaceFlags){
	int i;

	for(i=0; i<NOCONTENTS; i++){
		if(contents & nocontents[i])
			return qtrue;
	}
	for(i=0; i<NOSURFPARMS; i++){
		if(surfaceFlags & nosurfparms[i])
			return qtrue;
	}

	return qfalse;
}


/*
============
EmitBrushes
============
*/
void EmitBrushes ( bspbrush_t *brushes ) {
	int				j;
	dbrush_t		*db;
	bspbrush_t		*b;
	dbrushside_t	*cp;

	for ( b = brushes ; b ; b = b->next ) {
		if ( numbrushes == MAX_MAP_BRUSHES ) {
			Error( "MAX_MAP_BRUSHES" );
		}
		b->outputNumber = numbrushes;
		db = &dbrushes[numbrushes];
		numbrushes++;

		db->shaderNum = EmitShader( b->contentShader->shader );
		db->firstSide = numbrushsides;

		// don't emit any generated backSide sides
		db->numSides = 0;
		for ( j=0 ; j<b->numsides ; j++ ) {
			if ( b->sides[j].backSide ) {
				continue;
			}
			if ( numbrushsides == MAX_MAP_BRUSHSIDES ) {
				Error( "MAX_MAP_BRUSHSIDES ");
			}
			cp = &dbrushsides[numbrushsides];
			db->numSides++;
			numbrushsides++;
			cp->planeNum = b->sides[j].planenum;
			cp->shaderNum = EmitShader( b->sides[j].shaderInfo->shader );
		}	
	}
}

/*
==================
BeginModel
==================
*/
void BeginModel( void ) {
	dmodel_t	*mod;
	bspbrush_t	*b;
	entity_t	*e;
	vec3_t		mins, maxs;
	parseMesh_t	*p;
	int			i;

	if ( nummodels == MAX_MAP_MODELS ) {
		Error( "MAX_MAP_MODELS" );
	}
	mod = &dmodels[nummodels];

	//
	// bound the brushes
	//
	e = &entities[entity_num];

	ClearBounds (mins, maxs);
	for ( b = e->brushes ; b ; b = b->next ) {
		if ( !b->numsides ) {
			continue;	// not a real brush (origin brush, etc)
		}
		AddPointToBounds (b->mins, mins, maxs);
		AddPointToBounds (b->maxs, mins, maxs);
	}

	for ( p = e->patches ; p ; p = p->next ) {
		for ( i = 0 ; i < p->mesh.width * p->mesh.height ; i++ ) {
			AddPointToBounds( p->mesh.verts[i].xyz, mins, maxs );
		}
	}

	VectorCopy (mins, mod->mins);
	VectorCopy (maxs, mod->maxs);

	mod->firstSurface = numDrawSurfaces;
	mod->firstBrush = numbrushes;

	EmitBrushes( e->brushes );
}

/*
==================
EndModel
==================
*/
void EndModel( node_t *headnode ) {
	dmodel_t	*mod;

	qprintf ("--- EndModel ---\n");

	mod = &dmodels[nummodels];
	EmitDrawNode_r (headnode);
	mod->numSurfaces = numDrawSurfaces - mod->firstSurface;
	mod->numBrushes = numbrushes - mod->firstBrush;

	nummodels++;
}

