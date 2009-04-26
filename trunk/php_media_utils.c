void clamp(int *v, int min, int max) {
	if (*v < min) *v = min;
	if (*v > max) *v = max;
}

unsigned short utf8_decode(unsigned char *ptr, int *len) {
	if (ptr[0] < 128) {
		if (len != NULL) *len = 1;
		return ptr[0];
	}
	else if ((ptr[0] > 191) && (ptr[0] < 224)) {
		if (len != NULL) *len = 2;
		return ((ptr[0] & 31) << 6) | (ptr[1] & 63);
	}
	else {
		if (len != NULL) *len = 3;
		return ((ptr[0] & 15) << 12) | ((ptr[1] & 63) << 6) | (ptr[2] & 63);
	}
	return 0;
}

void FrameProcess() {
	if (music != NULL && !Mix_PlayingMusic()) {
		Mix_FreeMusic(music);
		music = NULL;
	}
}

static zval *ObjectInit(zend_class_entry *pce, zval *object, TSRMLS_D)
{
	if (!object) ALLOC_ZVAL(object);
	Z_TYPE_P(object) = IS_OBJECT;
	object_init_ex(object, pce);
	Z_SET_REFCOUNT_P(object, 1);
	Z_UNSET_ISREF_P(object);
	return object;
}

int __texPow2 = 0;
int __texRectangle = 0;

int extframebuffer = 0;
GLuint fbo = -1;

PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC        glGenFramebuffersEXT        = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC   glFramebufferTexture2DEXT   = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC     glDeleteFramebuffersEXT     = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC        glBindFramebufferEXT        = NULL;
// Shaders
PFNGLCREATEPROGRAMPROC             glCreateProgram             = NULL;
PFNGLLINKPROGRAMPROC               glLinkProgram               = NULL;
PFNGLUSEPROGRAMPROC                glUseProgram                = NULL;
PFNGLDELETEPROGRAMPROC             glDeleteProgram             = NULL;

PFNGLCREATESHADERPROC              glCreateShader              = NULL;
PFNGLSHADERSOURCEPROC              glShaderSource              = NULL;
PFNGLCOMPILESHADERPROC             glCompileShader             = NULL;
PFNGLDELETESHADERPROC              glDeleteShader              = NULL;
PFNGLGETSHADERIVPROC               glGetShaderiv               = NULL;
PFNGLATTACHSHADERPROC              glAttachShader              = NULL;
PFNGLGETSHADERINFOLOGPROC          glGetShaderInfoLog          = NULL;

PFNGLGETUNIFORMLOCATIONPROC        glGetUniformLocation        = NULL;
PFNGLGETACTIVEUNIFORMPROC          glGetActiveUniform          = NULL;

PFNGLUNIFORM1FVPROC                glUniform1fv                = NULL;
PFNGLUNIFORM2FVPROC                glUniform2fv                = NULL;
PFNGLUNIFORM3FVPROC                glUniform3fv                = NULL;
PFNGLUNIFORM4FVPROC                glUniform4fv                = NULL;

PFNGLUNIFORM1IVPROC                glUniform1iv                = NULL;
PFNGLUNIFORM2IVPROC                glUniform2iv                = NULL;
PFNGLUNIFORM3IVPROC                glUniform3iv                = NULL;
PFNGLUNIFORM4IVPROC                glUniform4iv                = NULL;

PFNGLACTIVETEXTUREPROC             glActiveTexture             = NULL;
PFNGLMULTITEXCOORD2DPROC           glMultiTexCoord2d           = NULL;

#define GL_LOAD_FUNC(name) name = SDL_GL_GetProcAddress(#name)

void glInitExsts() {
	GL_LOAD_FUNC(glCreateProgram);
	GL_LOAD_FUNC(glLinkProgram);
	GL_LOAD_FUNC(glUseProgram);
	GL_LOAD_FUNC(glDeleteProgram);

	GL_LOAD_FUNC(glCreateShader);
	GL_LOAD_FUNC(glShaderSource);
	GL_LOAD_FUNC(glDeleteShader);
	GL_LOAD_FUNC(glCompileShader);
	GL_LOAD_FUNC(glGetShaderiv);
	GL_LOAD_FUNC(glAttachShader);
	GL_LOAD_FUNC(glGetShaderInfoLog);

	GL_LOAD_FUNC(glGetUniformLocation);
	GL_LOAD_FUNC(glGetActiveUniform);

	GL_LOAD_FUNC(glUniform1fv);
	GL_LOAD_FUNC(glUniform2fv);
	GL_LOAD_FUNC(glUniform3fv);
	GL_LOAD_FUNC(glUniform4fv);

	GL_LOAD_FUNC(glUniform1iv);
	GL_LOAD_FUNC(glUniform2iv);
	GL_LOAD_FUNC(glUniform3iv);
	GL_LOAD_FUNC(glUniform4iv);

	GL_LOAD_FUNC(glActiveTexture);
	GL_LOAD_FUNC(glMultiTexCoord2d);
}


// Eliminamos el FrameBufferObject
void RemoveFrameBufferObject(void) {
	if (!fbo) return;
	glDeleteFramebuffersEXT(1, &fbo);
	fbo = 0;
}

void InitializeFrameBufferObject(void) {
	// Comprobamos si ya hemos inicializado el FrameBufferObject
	if (fbo != -1) return;

	// Comprobamos que exista la extensión del framebuffer_object
	if (strstr(glGetString(GL_EXTENSIONS), "EXT_framebuffer_object") == NULL) {
		SDL_SetError("EXT_framebuffer_object extension was not found");
		extframebuffer = 0;
		return;
	}

	// Obtenemos la dirección de los procedimientos
	GL_LOAD_FUNC(glGenFramebuffersEXT);
	GL_LOAD_FUNC(glCheckFramebufferStatusEXT);
	GL_LOAD_FUNC(glFramebufferTexture2DEXT);
	GL_LOAD_FUNC(glDeleteFramebuffersEXT);
	GL_LOAD_FUNC(glBindFramebufferEXT);

	// Comprobamos que se han obtenido correctamente los procedimientos
	if(!glCheckFramebufferStatusEXT || !glGenFramebuffersEXT || !glFramebufferTexture2DEXT || !glBindFramebufferEXT || !glDeleteFramebuffersEXT) {
		SDL_SetError("One or more EXT_framebuffer_object functions were not found");
		extframebuffer = 0;
		return;
	}

	// Generamos el FrameBufferObject
	glGenFramebuffersEXT(1, &fbo);

	// Comprobamos su estado
	switch (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)) {
		// Si se ha completado el proceso satisfactoriamente seguimos
		case GL_FRAMEBUFFER_COMPLETE_EXT: break;
		// Si se ha producido un error, mostramos un error
		default: case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			RemoveFrameBufferObject();
			SDL_SetError("Can't initialize framebuffer object");
			extframebuffer = 0;
			return;
		break;
	}

	// Finalmente ya podemos concluir que la extensión ha sido cargada satisfactoriamente
	// y puede ser usada.
	extframebuffer = 1;

	atexit(RemoveFrameBufferObject);
}

int fbo_selected_draw = 0;

void BitmapPrepareDraw(BitmapStruct *bitmap) {
	// Is screen.
	if (bitmap->surface == screen) {
		if (fbo_selected_draw == 0) return;
		fbo_selected_draw = 0;

		InitializeFrameBufferObject();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		glViewport(0, 0, screen->w, screen->h + 1);

		glMatrixMode(GL_TEXTURE   );
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, screen->w, screen->h, 0, -1.0, 1.0);
		glTranslatef(0, 1, 0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glShadeModel(GL_SMOOTH);
	} else {
		glViewport(bitmap->x, bitmap->y, bitmap->w, bitmap->h);
		//printf("(%f, %f)-(%f, %f)\n", (float)bitmap->x, (float)bitmap->y, (float)bitmap->w, (float)bitmap->h);

		if (fbo_selected_draw == bitmap->gltex) return;
		fbo_selected_draw = bitmap->gltex;
		InitializeFrameBufferObject();
		glFlush();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, bitmap->gltex, 0);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, bitmap->surface->w, 0, bitmap->surface->h, -1.0, 1.0);
		glTranslatef(0, 1, 0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glShadeModel(GL_SMOOTH);
		glEnable(GL_SCISSOR_TEST);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

__inline int __NextPowerOfTwo(int v) {
	int c = 1; while ((c <<= 1) < v); return c;
}

void __checkTexRectangle() {
	__texRectangle = (strstr((char *)glGetString(GL_EXTENSIONS), "EXT_texture_rectangle") != NULL) ? 1 : -1;
}

void __checkPow2() {
	__texPow2 = 1;
}


SDL_Surface *__SDL_CreateRGBSurfaceForOpenGL(int w, int h) {
	SDL_Surface *i;
	int rw, rh;

	//if (__texPow2 == 0) __checkPow2();
	//if (__texRectangle == 0) __checkTexRectangle();

	if (__texPow2 > 0) {
		rw = __NextPowerOfTwo(w);
		rh = __NextPowerOfTwo(h);
	} else {
		rw = w;
		rh = h;
	}

	if (__texRectangle > 0) {
		if (rw > rh) rh = rw; else rw = rh;
	}

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		i = SDL_CreateRGBSurface(SDL_SWSURFACE, rw, rh, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	#else
		i = SDL_CreateRGBSurface(SDL_SWSURFACE, rw, rh, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	#endif

	return i;
}

void BitmapPrepare(BitmapStruct *bitmap) {
	SDL_Surface *surfaceogl = NULL;
	if (bitmap->gltex != 0) return;
	if (!(surfaceogl = __SDL_CreateRGBSurfaceForOpenGL(bitmap->surface->w, bitmap->surface->h))) return;

	glGenTextures(1, &bitmap->gltex);
	glBindTexture(GL_TEXTURE_2D, bitmap->gltex);

	SDL_SetAlpha(bitmap->surface, 0, SDL_ALPHA_OPAQUE);
	SDL_BlitSurface(bitmap->surface, 0, surfaceogl, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, surfaceogl->w, surfaceogl->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surfaceogl->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glEnable(GL_CLAMP_TO_EDGE);
	
	SDL_FreeSurface(surfaceogl);
}