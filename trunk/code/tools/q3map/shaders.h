

typedef struct shaderInfo_s {
	char		shader[MAX_QPATH];
	int			surfaceFlags;
	int			contents;
	int			value;

	char		backShader[MAX_QPATH];	// for surfaces that generate different front and back passes
	char		flareShader[MAX_QPATH];	// for light flares

	float		subdivisions;			// from a "tesssize xxx"
	float		backsplashFraction;		// floating point value, usually 0.05
	float		backsplashDistance;		// default 16
	float		lightSubdivide;			// default 120
	int			lightmapSampleSize;		// lightmap sample size

	qboolean	hasPasses;				// false if the shader doesn't define any rendering passes

	qboolean	globalTexture;			// don't normalize texture repeats

	qboolean	twoSided;				// cull none
	qboolean	autosprite;				// autosprite shaders will become point lights
										// instead of area lights
	qboolean	lightFilter;			// light rays that cross surfaces of this type
										// should test against the filter image
	qboolean	forceTraceLight;		// always use -light for this surface
	qboolean	forceVLight;			// always use -vlight for this surface
	qboolean	patchShadows;			// have patches casting shadows when using -light for this surface
	qboolean	vertexShadows;			// shadows will be casted at this surface even when vertex lit
	qboolean	noVertexShadows;		// no shadows will be casted at this surface in vertex lighting
	qboolean	forceSunLight;			// force sun light at this surface even tho we might not calculate shadows in vertex lighting
	qboolean	notjunc;				// don't use this surface for tjunction fixing
	float		vertexScale;			// vertex light scale

	char		editorimage[MAX_QPATH];	// use this image to generate texture coordinates
	char		lightimage[MAX_QPATH];	// use this image to generate color / averageColor
	vec3_t		color;					// colorNormalized
	vec3_t		averageColor;

	int			width, height;
	byte		*pixels;

	vec3_t		sunLight;
	vec3_t		sunDirection;
} shaderInfo_t;

void LoadShaderInfo( void );
shaderInfo_t	*ShaderInfoForShader( const char *shader );
int GetSurfaceParm(const char *tex);

