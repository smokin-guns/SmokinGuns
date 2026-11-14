/* SDL2/WebGL window and context setup for the Emscripten client. */
#include <SDL.h>

#include <math.h>
#include <stdlib.h>

#include "../renderercommon/tr_common.h"
#include "../sys/sys_local.h"

#define R_MODE_FALLBACK 3 /* 640x480 */

extern void APIENTRY glDrawBuffers(GLsizei count, const GLenum *buffers);

typedef enum { RSERR_OK, RSERR_INVALID_MODE } rserr_t;

static SDL_Window *sdlWindow;
static SDL_GLContext sdlContext;
int qglesMajorVersion;
int qglesMinorVersion;

void APIENTRY GLimp_GLES_Begin(GLenum mode) { (void)mode; }
void APIENTRY GLimp_GLES_End(void) { }
void APIENTRY GLimp_GLES_Vertex3fv(const GLfloat *vertex) { (void)vertex; }
void APIENTRY GLimp_GLES_Vertex3f(GLfloat x, GLfloat y, GLfloat z) { (void)x; (void)y; (void)z; }
void APIENTRY GLimp_GLES_Color3f(GLfloat red, GLfloat green, GLfloat blue)
{ (void)red; (void)green; (void)blue; }
void APIENTRY GLimp_GLES_Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{ (void)red; (void)green; (void)blue; (void)alpha; }
void APIENTRY GLimp_GLES_CallList(GLuint list) { (void)list; }
void APIENTRY GLimp_GLES_TexEnvf(GLenum target, GLenum pname, GLfloat param)
{ (void)target; (void)pname; (void)param; }
void APIENTRY GLimp_GLES_PolygonMode(GLenum face, GLenum mode)
{ (void)face; (void)mode; }
void APIENTRY GLimp_GLES_AlphaFunc(GLenum func, GLclampf ref)
{ (void)func; (void)ref; }
void APIENTRY GLimp_GLES_DrawBuffer(GLenum mode)
{
	if (mode == GL_FRONT)
		mode = GL_BACK;
	glDrawBuffers(1, &mode);
}
void APIENTRY GLimp_GLES_Enable(GLenum cap)
{
	if (cap != GL_ALPHA_TEST && cap != GL_TEXTURE_2D)
		glEnable(cap);
}
void APIENTRY GLimp_GLES_Disable(GLenum cap)
{
	if (cap != GL_ALPHA_TEST && cap != GL_TEXTURE_2D)
		glDisable(cap);
}

cvar_t *r_allowSoftwareGL;
cvar_t *r_allowResize;
cvar_t *r_centerWindow;
cvar_t *r_sdlDriver;
#ifdef FRAMEBUFFER_AND_GLSL_SUPPORT
cvar_t *r_ext_framebuffer;
#endif

void (APIENTRYP qglActiveTextureARB)(GLenum texture);
void (APIENTRYP qglClientActiveTextureARB)(GLenum texture);
void (APIENTRYP qglMultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t);
void (APIENTRYP qglLockArraysEXT)(GLint first, GLsizei count);
void (APIENTRYP qglUnlockArraysEXT)(void);

#if 0 /* OpenGL2's tr_extensions.c owns these entry points in this build. */
void (APIENTRYP qglGenFramebuffersEXT)(GLsizei, GLuint *);
void (APIENTRYP qglBindFramebufferEXT)(GLenum, GLuint);
void (APIENTRYP qglGenRenderbuffersEXT)(GLsizei, GLuint *);
void (APIENTRYP qglBindRenderbufferEXT)(GLenum, GLuint);
void (APIENTRYP qglRenderbufferStorageEXT)(GLenum, GLenum, GLsizei, GLsizei);
void (APIENTRYP qglRenderbufferStorageMultisampleEXT)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
void (APIENTRYP qglFramebufferRenderbufferEXT)(GLenum, GLenum, GLenum, GLuint);
void (APIENTRYP qglFramebufferTexture2DEXT)(GLenum, GLenum, GLenum, GLuint, GLint);
GLenum (APIENTRYP qglCheckFramebufferStatusEXT)(GLenum);
void (APIENTRYP qglDeleteFramebuffersEXT)(GLsizei, const GLuint *);
void (APIENTRYP qglDeleteRenderbuffersEXT)(GLsizei, const GLuint *);
void (APIENTRYP qglBlitFramebufferEXT)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLenum, GLbitfield);
void (APIENTRYP qglAttachShader)(GLuint, GLuint);
void (APIENTRYP qglBindAttribLocation)(GLuint, GLuint, const GLchar *);
void (APIENTRYP qglCompileShader)(GLuint);
GLuint (APIENTRYP qglCreateProgram)(void);
GLuint (APIENTRYP qglCreateShader)(GLenum);
void (APIENTRYP qglDeleteProgram)(GLuint);
void (APIENTRYP qglDeleteShader)(GLuint);
void (APIENTRYP qglShaderSource)(GLuint, GLsizei, const GLchar **, const GLint *);
void (APIENTRYP qglLinkProgram)(GLuint);
void (APIENTRYP qglUseProgram)(GLuint);
GLint (APIENTRYP qglGetUniformLocation)(GLuint, const GLchar *);
void (APIENTRYP qglUniform1f)(GLint, GLfloat);
void (APIENTRYP qglUniform2f)(GLint, GLfloat, GLfloat);
void (APIENTRYP qglUniform1i)(GLint, GLint);
void (APIENTRYP qglGetProgramiv)(GLuint, GLenum, GLint *);
void (APIENTRYP qglGetProgramInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);
void (APIENTRYP qglGetShaderiv)(GLuint, GLenum, GLint *);
void (APIENTRYP qglGetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);
qboolean framebufferSupported;
qboolean glslSupported;
qboolean multisampleSupported;
#endif

static void GLimp_DestroyContext(void)
{
	if (sdlContext) {
		SDL_GL_DeleteContext(sdlContext);
		sdlContext = NULL;
	}
	if (sdlWindow) {
		SDL_DestroyWindow(sdlWindow);
		sdlWindow = NULL;
	}
}

void GLimp_Shutdown(void)
{
	ri.IN_Shutdown();
	GLimp_DestroyContext();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void GLimp_Minimize(void)
{
	if (sdlWindow)
		SDL_MinimizeWindow(sdlWindow);
}

void GLimp_LogComment(char *comment)
{
	(void)comment;
}

static int GLimp_SetMode(int mode)
{
	int width, height;
	float aspect;
	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	const char *renderer;

	if (mode == -2) {
		width = 960;
		height = 540;
		aspect = (float)width / height;
	} else if (!R_GetModeInfo(&width, &height, &aspect, mode)) {
		return RSERR_INVALID_MODE;
	}

	if (width > 1280) width = 1280;
	if (height > 720) height = 720;
	aspect = (float)width / height;
	glConfig.vidWidth = width;
	glConfig.vidHeight = height;
	glConfig.windowAspect = aspect;
	glConfig.isFullscreen = qfalse;
	glConfig.stereoEnabled = qfalse;
	displayAspect = aspect;

	if (r_allowResize && r_allowResize->integer)
		flags |= SDL_WINDOW_RESIZABLE;
	if (r_noborder && r_noborder->integer)
		flags |= SDL_WINDOW_BORDERLESS;

	GLimp_DestroyContext();
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);

	sdlWindow = SDL_CreateWindow(CLIENT_WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, width, height, flags);
	if (!sdlWindow) {
		ri.Printf(PRINT_WARNING, "SDL_CreateWindow failed: %s\n", SDL_GetError());
		return RSERR_INVALID_MODE;
	}
	sdlContext = SDL_GL_CreateContext(sdlWindow);
	if (!sdlContext) {
		ri.Printf(PRINT_WARNING, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
		GLimp_DestroyContext();
		return RSERR_INVALID_MODE;
	}

	glConfig.colorBits = 32;
	glConfig.depthBits = 24;
	glConfig.stencilBits = 8;
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;
	/* This path requests an ES 3.0 context, which is WebGL 2. */
	qglesMajorVersion = 3;
	qglesMinorVersion = 0;
	renderer = (const char *)qglGetString(GL_RENDERER);
	ri.Printf(PRINT_ALL, "GL_RENDERER: %s\n", renderer ? renderer : "unknown");
	return RSERR_OK;
}

static qboolean GLimp_StartDriverAndSetMode(int mode)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO) && SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		ri.Printf(PRINT_WARNING, "SDL video initialization failed: %s\n", SDL_GetError());
		return qfalse;
	}
	if (GLimp_SetMode(mode) == RSERR_OK)
		return qtrue;
	return qfalse;
}

static qboolean GLimp_HaveExtension(const char *extension)
{
	const char *extensions = (const char *)qglGetString(GL_EXTENSIONS);
	const char *where;
	size_t length = strlen(extension);
	if (!extensions || !*extension || strchr(extension, ' '))
		return qfalse;
	for (where = extensions; (where = strstr(where, extension)) != NULL; where += length) {
		if ((where == extensions || where[-1] == ' ') &&
		    (where[length] == ' ' || where[length] == '\0'))
			return qtrue;
	}
	return qfalse;
}

static void GLimp_InitExtensions(void)
{
	/* glActiveTexture is part of the WebGL 2 / OpenGL ES 3 core API. */
	qglActiveTextureARB = SDL_GL_GetProcAddress("glActiveTexture");
	if (!qglActiveTextureARB)
		qglActiveTextureARB = SDL_GL_GetProcAddress("glActiveTextureARB");
	if (GLimp_HaveExtension("GL_ARB_multitexture")) {
		qglMultiTexCoord2fARB = SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
		qglClientActiveTextureARB = SDL_GL_GetProcAddress("glClientActiveTextureARB");
	}
	if (GLimp_HaveExtension("GL_EXT_compiled_vertex_array")) {
		qglLockArraysEXT = SDL_GL_GetProcAddress("glLockArraysEXT");
		qglUnlockArraysEXT = SDL_GL_GetProcAddress("glUnlockArraysEXT");
	}
#if 0 /* OpenGL2's tr_extensions.c owns these entry points in this build. */
	if (GLimp_HaveExtension("GL_EXT_framebuffer_object")) {
		qglGenFramebuffersEXT = SDL_GL_GetProcAddress("glGenFramebuffersEXT");
		qglBindFramebufferEXT = SDL_GL_GetProcAddress("glBindFramebufferEXT");
		qglGenRenderbuffersEXT = SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
		qglBindRenderbufferEXT = SDL_GL_GetProcAddress("glBindRenderbufferEXT");
		qglRenderbufferStorageEXT = SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
		qglFramebufferRenderbufferEXT = SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
		qglFramebufferTexture2DEXT = SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
		qglCheckFramebufferStatusEXT = SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
		qglDeleteFramebuffersEXT = SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
		qglDeleteRenderbuffersEXT = SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");
		qglBlitFramebufferEXT = SDL_GL_GetProcAddress("glBlitFramebufferEXT");
		qglRenderbufferStorageMultisampleEXT = SDL_GL_GetProcAddress("glRenderbufferStorageMultisampleEXT");
		framebufferSupported = qglGenFramebuffersEXT && qglBindFramebufferEXT && qglGenRenderbuffersEXT &&
			qglBindRenderbufferEXT && qglRenderbufferStorageEXT && qglFramebufferRenderbufferEXT &&
			qglFramebufferTexture2DEXT && qglCheckFramebufferStatusEXT;
	}
	if (GLimp_HaveExtension("GL_ARB_shader_objects")) {
		qglAttachShader = SDL_GL_GetProcAddress("glAttachShader");
		qglBindAttribLocation = SDL_GL_GetProcAddress("glBindAttribLocation");
		qglCompileShader = SDL_GL_GetProcAddress("glCompileShader");
		qglCreateProgram = SDL_GL_GetProcAddress("glCreateProgram");
		qglCreateShader = SDL_GL_GetProcAddress("glCreateShader");
		qglDeleteProgram = SDL_GL_GetProcAddress("glDeleteProgram");
		qglDeleteShader = SDL_GL_GetProcAddress("glDeleteShader");
		qglShaderSource = SDL_GL_GetProcAddress("glShaderSource");
		qglLinkProgram = SDL_GL_GetProcAddress("glLinkProgram");
		qglUseProgram = SDL_GL_GetProcAddress("glUseProgram");
		qglGetUniformLocation = SDL_GL_GetProcAddress("glGetUniformLocation");
		qglUniform1f = SDL_GL_GetProcAddress("glUniform1f");
		qglUniform2f = SDL_GL_GetProcAddress("glUniform2f");
		qglUniform1i = SDL_GL_GetProcAddress("glUniform1i");
		qglGetProgramiv = SDL_GL_GetProcAddress("glGetProgramiv");
		qglGetProgramInfoLog = SDL_GL_GetProcAddress("glGetProgramInfoLog");
		qglGetShaderiv = SDL_GL_GetProcAddress("glGetShaderiv");
		qglGetShaderInfoLog = SDL_GL_GetProcAddress("glGetShaderInfoLog");
		glslSupported = qglCreateShader && qglCreateProgram && qglUseProgram;
	}
#endif
}

void GLimp_Init(void)
{
	r_allowSoftwareGL = ri.Cvar_Get("r_allowSoftwareGL", "0", CVAR_LATCH);
	r_sdlDriver = ri.Cvar_Get("r_sdlDriver", "", CVAR_ROM);
	r_allowResize = ri.Cvar_Get("r_allowResize", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_centerWindow = ri.Cvar_Get("r_centerWindow", "0", CVAR_ARCHIVE | CVAR_LATCH);
#ifdef FRAMEBUFFER_AND_GLSL_SUPPORT
	r_ext_framebuffer = ri.Cvar_Get("r_ext_framebuffer", "0", CVAR_ARCHIVE | CVAR_LATCH);
#endif

	ri.Sys_GLimpInit();
	if (!GLimp_StartDriverAndSetMode(r_mode->integer) &&
	    !GLimp_StartDriverAndSetMode(R_MODE_FALLBACK))
		ri.Error(ERR_FATAL, "GLimp_Init() - could not create WebGL 2 context");

	glConfig.deviceSupportsGamma = qfalse;
	Q_strncpyz(glConfig.vendor_string, (const char *)qglGetString(GL_VENDOR), sizeof(glConfig.vendor_string));
	Q_strncpyz(glConfig.renderer_string, (const char *)qglGetString(GL_RENDERER), sizeof(glConfig.renderer_string));
	Q_strncpyz(glConfig.version_string, (const char *)qglGetString(GL_VERSION), sizeof(glConfig.version_string));
	Q_strncpyz(glConfig.extensions_string, (const char *)qglGetString(GL_EXTENSIONS), sizeof(glConfig.extensions_string));
	GLimp_InitExtensions();
	ri.Cvar_Get("r_availableModes", "", CVAR_ROM);
	ri.IN_Init();
}

void GLimp_EndFrame(void)
{
	if (sdlWindow && Q_stricmp(r_drawBuffer->string, "GL_FRONT") != 0)
		SDL_GL_SwapWindow(sdlWindow);
	if (r_fullscreen->modified) {
		if (r_fullscreen->integer)
			ri.Printf(PRINT_ALL, "Browser fullscreen is controlled by the page.\n");
		r_fullscreen->modified = qfalse;
	}
}
