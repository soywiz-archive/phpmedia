void clamp(int *v, int min, int max) {
	if (*v < min) *v = min;
	if (*v > max) *v = max;
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
	glGenFramebuffersEXT        = (PFNGLGENFRAMEBUFFERSEXTPROC)       SDL_GL_GetProcAddress("glGenFramebuffersEXT");
	glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
	glFramebufferTexture2DEXT   = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)  SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
	glDeleteFramebuffersEXT     = (PFNGLDELETEFRAMEBUFFERSEXTPROC)    SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
	glBindFramebufferEXT        = (PFNGLBINDFRAMEBUFFEREXTPROC)       SDL_GL_GetProcAddress("glBindFramebufferEXT");

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

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_TEXTURE   );
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
	} else {
		glViewport(bitmap->x, bitmap->y, bitmap->w, bitmap->h);

		if (fbo_selected_draw == bitmap->gltex) return;
		fbo_selected_draw = bitmap->gltex;
		InitializeFrameBufferObject();
		glFlush();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, bitmap->gltex, 0);

		glMatrixMode(GL_TEXTURE);
		glPushMatrix();

		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, bitmap->surface->w, 0, bitmap->surface->h, -1.0, 1.0);
		glTranslatef(0, 1, 0);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
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