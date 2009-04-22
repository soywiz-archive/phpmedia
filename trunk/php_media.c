#include "php.h"
#include "php_ini.h"
#include "zend_exceptions.h"
#include "SAPI.h"
#include "ext/standard/info.h"
#include <windows.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <GL/gl.h>
#include <GL/glext.h>

SDL_Surface *screen;
int keys_status[SDLK_LAST];
int keys_pressed[SDLK_LAST];

#define PHP_METHODS_NAME(CLASS) CLASS##_Methods
#define PHP_METHOD_NAME_ARGINFO(CLASS, METHOD) arginfo_##CLASS##METHOD

//zval *object = getThis();
#define GET_THIS_TYPE(TYPE, NAME) TYPE *NAME = (TYPE *)zend_object_store_get_object(getThis(), TSRMLS_C);
#define THROW    php_set_error_handling(EH_THROW, zend_exception_get_default(), TSRMLS_C);
#define NO_THROW php_set_error_handling(EH_NORMAL, NULL, TSRMLS_C);
#define STRUCT_CREATE(TYPE, VAR) VAR = emalloc(sizeof(TYPE)); memset(VAR, 0, sizeof(TYPE));
#define PHP_METHOD_ARGS(CLASS, METHOD) ZEND_BEGIN_ARG_INFO_EX(PHP_METHOD_NAME_ARGINFO(CLASS, METHOD), 0, 0, 1)
#define ARG_INFO(name) ZEND_ARG_INFO(0, name)
#define THIS_BITMAP GET_THIS_TYPE(BitmapStruct, bitmap);
#define PHP_ME_AI(CLASS, METHOD, ATT) PHP_ME(CLASS, METHOD, PHP_METHOD_NAME_ARGINFO(CLASS, METHOD), ATT)
#define PHP_ME_AI(CLASS, METHOD, ATT) PHP_ME(CLASS, METHOD, PHP_METHOD_NAME_ARGINFO(CLASS, METHOD), ATT)
#define PHP_ME_END {NULL, NULL, NULL}
#define PHP_FE_END {NULL, NULL, NULL}
#define PM_METHODS(CLASS) const zend_function_entry PHP_METHODS_NAME(CLASS)[] =
#define CLONE_COPY_FIELD(field) new_obj->field = old_obj->field;
#define CLASS_REGISTER_CONSTANT_STR(CONST, VALUE) zend_declare_class_constant_stringl(CurrentClassEntry, (CONST), sizeof(CONST) - 1, (VALUE), sizeof(VALUE) - 1, TSRMLS_C);
#define CLASS_REGISTER_CONSTANT_INT(CONST, VALUE) zend_declare_class_constant_long(CurrentClassEntry, (CONST), sizeof(CONST) - 1, VALUE, TSRMLS_C);
#define PM_CLASS_INIT(CLASS) INIT_CLASS_ENTRY(ClassEntry, #CLASS, PHP_METHODS_NAME(CLASS));
#define PM_CLASS_REGISTER() CurrentClassEntry = zend_register_internal_class_ex(&ClassEntry, NULL, NULL, TSRMLS_C);
#define PM_HANDLERS_INIT(TYPE) memcpy(CurrentHandlers = &TYPE, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
#define PM_HANDLERS_ADD(TYPE, FUNC) CurrentHandlers->TYPE = FUNC;
#define PM_CLASS_ADD(TYPE, FUNC) ClassEntry.TYPE = FUNC;

static zend_object_handlers Bitmap_Handlers;
static zend_class_entry    *Bitmap_ClassEntry;

typedef struct {
	SDL_Surface *surface;
	GLint gltex;
	int refcount;
	int updated_source;
} Image;

typedef struct _BitmapStruct {
	zend_object std;
	struct _BitmapStruct *parent;
	SDL_Surface *surface;
	GLint gltex;

	// new impl
	Image image;

	int cx, cy;
	int x, y, w, h;
} BitmapStruct;

int __texPow2 = 0;
int __texRectangle = 0;

int extframebuffer = 0;
GLuint fbo = -1;

PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC        glGenFramebuffersEXT        = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC   glFramebufferTexture2DEXT   = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC     glDeleteFramebuffersEXT     = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC        glBindFramebufferEXT        = NULL;

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

static zval *ObjectInit(zend_class_entry *pce, zval *object, TSRMLS_D)
{
	if (!object) ALLOC_ZVAL(object);
	Z_TYPE_P(object) = IS_OBJECT;
	object_init_ex(object, pce);
	Z_SET_REFCOUNT_P(object, 1);
	Z_UNSET_ISREF_P(object);
	return object;
}

static void Bitmap__ObjectDelete(void *object, TSRMLS_D)
{
	BitmapStruct *bitmap = (BitmapStruct *)object;
	{
		if (bitmap->surface)
		{
			if (bitmap->surface->refcount <= 1)
			{
				glDeleteTextures(1, &bitmap->gltex);
			}
			SDL_FreeSurface(bitmap->surface);
		}
		zend_object_std_dtor(&bitmap->std, TSRMLS_C);
		//printf("free\n");
	}
	efree(object);
}

static zend_object_value Bitmap__ObjectNew_ex(zend_class_entry *class_type, BitmapStruct **ptr, TSRMLS_D)
{
	BitmapStruct *intern;
	zend_object_value retval;
	zval *tmp;

	//printf("Bitmap__ObjectNew\n");

	STRUCT_CREATE(BitmapStruct, intern);
	if (ptr != NULL) *ptr = intern;
	
	zend_object_std_init(&intern->std, class_type, TSRMLS_C);
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
	
	retval.handle = zend_objects_store_put(
		intern,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t) Bitmap__ObjectDelete,
		NULL,
		TSRMLS_C
	);

	retval.handlers = &Bitmap_Handlers;
	
	return retval;
}

static zend_object_value Bitmap__ObjectNew(zend_class_entry *class_type, TSRMLS_D)
{
	return Bitmap__ObjectNew_ex(class_type, NULL, TSRMLS_C);
}

static zend_object_value Bitmap__ObjectClone(zval *this_ptr, TSRMLS_D)
{
	BitmapStruct *new_obj = NULL;
	BitmapStruct *old_obj = (BitmapStruct *)zend_object_store_get_object(this_ptr, TSRMLS_C);

	zend_object_value new_ov = Bitmap__ObjectNew_ex(old_obj->std.ce, &new_obj, TSRMLS_C);
	
	zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr), TSRMLS_C);

	//CLONE_COPY_FIELD(surface->w);
	//CLONE_COPY_FIELD(surface->h);
	CLONE_COPY_FIELD(parent);
	CLONE_COPY_FIELD(surface);
	CLONE_COPY_FIELD(gltex);
	CLONE_COPY_FIELD(cx);
	CLONE_COPY_FIELD(cy);
	CLONE_COPY_FIELD(x);
	CLONE_COPY_FIELD(y);
	CLONE_COPY_FIELD(w);
	CLONE_COPY_FIELD(h);
	new_obj->surface->refcount++;
	
	return new_ov;
}


// Bitmap::__construct($w, $h)
PHP_METHOD_ARGS(Bitmap, __construct) ARG_INFO(w) ARG_INFO(h) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, __construct)
{
	int w, h;
	THIS_BITMAP;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ll", &w, &h) == FAILURE) RETURN_FALSE;

	bitmap->surface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
	if (bitmap->surface) {
		bitmap->x = 0;
		bitmap->y = 0;
		bitmap->w = w;
		bitmap->h = h;
		bitmap->cx = 0;
		bitmap->cy = 0;
		BitmapPrepare(bitmap);
	}
}

// Bitmap::__set($key, $value)
PHP_METHOD_ARGS(Bitmap, __set) ARG_INFO(key) ARG_INFO(value) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, __set)
{
	char *key; int key_l; int v;
	THIS_BITMAP;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "sl", &key, &key_l, &v) == FAILURE) RETURN_FALSE;

	switch (key_l) {
		case 2:
			if (strcmp(key, "cx") == 0) RETURN_LONG((bitmap) ? bitmap->cx = v : 0);
			if (strcmp(key, "cy") == 0) RETURN_LONG((bitmap) ? bitmap->cy = v : 0);
		break;
	}

	RETURN_FALSE;
}

// Bitmap::__get($key)
PHP_METHOD_ARGS(Bitmap, __get) ARG_INFO(key) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, __get)
{
	char *key; int key_l;
	THIS_BITMAP;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &key, &key_l) == FAILURE) RETURN_FALSE;

	switch (key_l) {
		case 1:
			if (strcmp(key, "w") == 0) RETURN_LONG((bitmap && bitmap->surface) ? bitmap->w : 0);
			if (strcmp(key, "h") == 0) RETURN_LONG((bitmap && bitmap->surface) ? bitmap->h : 0);
		break;
		case 2:
			if (strcmp(key, "cx") == 0) RETURN_LONG((bitmap) ? bitmap->cx : 0);
			if (strcmp(key, "cy") == 0) RETURN_LONG((bitmap) ? bitmap->cy : 0);
		break;
	}

	RETURN_FALSE;
}

void clamp(int *v, int min, int max) {
	if (*v < min) *v = min;
	if (*v > max) *v = max;
}

// Bitmap::slice($x, $y, $w, $h)
PHP_METHOD_ARGS(Bitmap, slice) ARG_INFO(x) ARG_INFO(y) ARG_INFO(w) ARG_INFO(h) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, slice)
{
	int x = 0, y = 0, w = 0, h = 0;
	BitmapStruct *new_bitmap;
	THIS_BITMAP;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "llll", &x, &y, &w, &h) == FAILURE) RETURN_FALSE;
	
	clamp(&x, 0, bitmap->w);
	clamp(&y, 0, bitmap->h);
	clamp(&w, 0, bitmap->w - x);
	clamp(&h, 0, bitmap->h - y);

	ObjectInit(Bitmap_ClassEntry, return_value, TSRMLS_C);
	new_bitmap = zend_object_store_get_object(return_value, TSRMLS_C);
	new_bitmap->parent = bitmap;
	new_bitmap->surface = bitmap->surface;
	new_bitmap->gltex = bitmap->gltex;
	new_bitmap->x = x + bitmap->x;
	new_bitmap->y = y + bitmap->y;
	new_bitmap->cx = 0;
	new_bitmap->cy = 0;
	new_bitmap->w = w;
	new_bitmap->h = h;
	new_bitmap->surface->refcount++;
}

// Bitmap::split($w, $h)
PHP_METHOD_ARGS(Bitmap, split) ARG_INFO(w) ARG_INFO(h) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, split)
{
	int w = 0, h = 0;
	int x, y;
	zval *object;
	BitmapStruct *new_bitmap;
	THIS_BITMAP;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ll", &w, &h) == FAILURE) RETURN_FALSE;
	
	array_init(return_value);
	
	for (y = 0; y < bitmap->h; y += h) {
		for (x = 0; x < bitmap->w; x += w) {
			object = ObjectInit(Bitmap_ClassEntry, NULL, TSRMLS_C);
			new_bitmap = zend_object_store_get_object(object, TSRMLS_C);
			new_bitmap->parent = bitmap;
			new_bitmap->surface = bitmap->surface;
			new_bitmap->gltex = bitmap->gltex;
			new_bitmap->x = x + bitmap->x;
			new_bitmap->y = y + bitmap->y;
			new_bitmap->cx = 0;
			new_bitmap->cy = 0;
			new_bitmap->w = w;
			new_bitmap->h = h;
			new_bitmap->surface->refcount++;
			add_next_index_zval(return_value, object);
		}
	}
}

// Bitmap::fromFile($name)
PHP_METHOD_ARGS(Bitmap, fromFile) ARG_INFO(name) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, fromFile)
{
	char *name; int name_len;
	SDL_Surface *surface;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &name, &name_len) == FAILURE) RETURN_FALSE;

	if (surface = IMG_Load(name)) {
		BitmapStruct *bitmap;
		ObjectInit(Bitmap_ClassEntry, return_value, TSRMLS_C);
		bitmap = zend_object_store_get_object(return_value, TSRMLS_C);
		bitmap->surface = surface;
		bitmap->x = bitmap->y = bitmap->cx = bitmap->cy = 0;
		bitmap->w = surface->w;
		bitmap->h = surface->h;
		BitmapPrepare(bitmap);
	} else {
		zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0, TSRMLS_C, "Can't load image '%s'", name);
		RETURN_FALSE;
	}
}

// Bitmap::fromString($data)
PHP_METHOD_ARGS(Bitmap, fromString) ARG_INFO(name) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, fromString)
{
	char *data; int data_len;
	SDL_Surface *surface;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &data, &data_len) == FAILURE) RETURN_FALSE;

	if (surface = IMG_Load_RW(SDL_RWFromConstMem(data, data_len), 1)) {
		BitmapStruct *bitmap;
		ObjectInit(Bitmap_ClassEntry, return_value, TSRMLS_C);
		bitmap = zend_object_store_get_object(return_value, TSRMLS_C);
		bitmap->surface = surface;
		bitmap->x = bitmap->y = bitmap->cx = bitmap->cy = 0;
		bitmap->w = surface->w;
		bitmap->h = surface->h;
		BitmapPrepare(bitmap);
	} else {
		zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0, TSRMLS_C, "Can't load image from string");
		RETURN_FALSE;
	}
}

// Bitmap::clear($r = 0, $g = 0, $b = 0, $a = 0)
PHP_METHOD_ARGS(Bitmap, clear) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, clear)
{
	double r = 0, g = 0, b = 0, a = 0;
	THIS_BITMAP;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|dddd", &r, &g, &b, &a) == FAILURE) RETURN_FALSE;
	
	BitmapPrepareDraw(bitmap);

	glClearColor((float)r, (float)g, (float)b, (float)a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RETURN_TRUE;
}

// Bitmap::center($x, $y)
// Bitmap::center()
PHP_METHOD_ARGS(Bitmap, center) ARG_INFO(cx) ARG_INFO(cy) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, center)
{
	int cx = 0, cy = 0;
	THIS_BITMAP;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|ll", &cx, &cy) == FAILURE) RETURN_FALSE;
	
	if (ZEND_NUM_ARGS() < 2) {
		bitmap->cx = bitmap->surface->w;
		bitmap->cy = bitmap->surface->h;
	} else {
		bitmap->cx = cx;
		bitmap->cy = cy;
	}
}

// Bitmap::blit($bmp, $x = 0, $y = 0, $size = 1, $rotation = 0)
PHP_METHOD_ARGS(Bitmap, blit) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, blit)
{
	zval *object;
	double x = 0, y = 0;
	double size = 1, rotation = 0, alpha = 1;
	BitmapStruct *source;
	double w, h, cx, cy;
	double tx[2], ty[2];
	THIS_BITMAP;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "O|ddddd", &object, Bitmap_ClassEntry, &x, &y, &size, &rotation, &alpha) == FAILURE) RETURN_FALSE;

	source = (BitmapStruct *)zend_object_store_get_object(object, TSRMLS_C);
	//printf("%d\n", source->gltex);
	BitmapPrepare(source);
	
	BitmapPrepareDraw(bitmap);

	glLoadIdentity();
	glTranslated(x, y, 0);
	glRotated(rotation, 0, 0, 1);
	//glScaled((double)source->surface->w, (double)source->surface->h, 1);
	glScaled(size, size, 1);

	glBindTexture(GL_TEXTURE_2D, source->gltex);
	glTexParameterf(GL_TEXTURE_2D, 0x84FF, 16);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glColor4d(1, 1, 1, alpha);
	w = source->w;
	h = source->h;
	cx = source->cx;
	cy = source->cy;
	tx[0] = (double)(source->x + 0) / (double)source->surface->w;
	tx[1] = (double)(source->x + w) / (double)source->surface->w;
	ty[0] = (double)(source->y + 0) / (double)source->surface->h;
	ty[1] = (double)(source->y + h) / (double)source->surface->h;
	
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
		glTexCoord2d(tx[0], ty[0]); glVertex2d(0 - cx, 0 - cy);
		glTexCoord2d(tx[1], ty[0]); glVertex2d(w - cx, 0 - cy);
		glTexCoord2d(tx[1], ty[1]); glVertex2d(w - cx, h - cy);
		glTexCoord2d(tx[0], ty[1]); glVertex2d(0 - cx, h - cy);
	glEnd();
	
	//SDL_BlitSurface(source->surface, NULL, bitmap->surface, NULL);

	RETURN_TRUE;
}

#ifdef WIN32
HICON icon;
struct zWMcursor { void* curs; };
HWND hwnd;
SDL_SysWMinfo wminfo;
#endif

// Screen::init($w, $h, $fullscreen = false)
PHP_METHOD_ARGS(Screen, init) ARG_INFO(w) ARG_INFO(h) ARG_INFO(fullscreen) ZEND_END_ARG_INFO()
PHP_METHOD(Screen, init)
{
	int w, h, fullscreen = 0, flags = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ll|l", &w, &h, &fullscreen) == FAILURE) RETURN_FALSE;

	flags |= SDL_OPENGL;
	flags |= SDL_HWSURFACE;
	if (fullscreen) flags |= SDL_FULLSCREEN;

	#ifdef WIN32
	{
		SDL_Cursor *cursor = SDL_GetCursor();

		HINSTANCE handle = GetModuleHandle(NULL);
		//((struct zWMcursor *)cursor->wm_cursor)->curs = (void *)LoadCursorA(NULL, IDC_ARROW);
		((struct zWMcursor *)cursor->wm_cursor)->curs = (void *)LoadCursor(NULL, IDC_ARROW);
		SDL_SetCursor(cursor);

		icon = LoadIcon(handle, (char *)101);
		SDL_GetWMInfo(&wminfo);
		hwnd = (HANDLE)wminfo.window;
		SetClassLong(hwnd, GCL_HICON, (LONG)icon);
	}
	#endif

	SDL_putenv("SDL_VIDEO_WINDOW_POS=center");
	SDL_putenv("SDL_VIDEO_CENTERED=center");

	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	screen = SDL_SetVideoMode(w, h, 0, flags);

    #ifdef WIN32
    	SetClassLong(hwnd, GCL_HICON, (LONG)icon);
    #endif

	glEnable(GL_LINE_SMOOTH);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	SDL_GL_SwapBuffers();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glViewport(0, 0, w, h + 1);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_TEXTURE   ); glLoadIdentity();
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(0, w, h, 0, -1.0, 1.0);
	glTranslatef(0, 1, 0);

	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glShadeModel(GL_SMOOTH);

	glEnable(GL_SCISSOR_TEST);
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (screen) {
		ObjectInit(Bitmap_ClassEntry, return_value, TSRMLS_C);
		{
			BitmapStruct *bmp = zend_object_store_get_object(return_value, TSRMLS_C);
			bmp->surface = screen;
			bmp->w = screen->w;
			bmp->h = screen->h;
			bmp->cx = bmp->cy = bmp->x = bmp->y = 0;
		}
	} else {
		RETURN_FALSE;
	}
}

// Screen::frame()
PHP_METHOD_ARGS(Screen, frame) ZEND_END_ARG_INFO()
PHP_METHOD(Screen, frame)
{
	SDL_Event event;
	static unsigned int ticks = 0;

	memset(keys_pressed, 0, sizeof(keys_pressed));

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				exit(0);
			break;
			case SDL_KEYDOWN:
				keys_pressed[event.key.keysym.sym] = 1;
				keys_status[event.key.keysym.sym] = 1;
			break;
			case SDL_KEYUP:
				keys_pressed[event.key.keysym.sym] = 0;
				keys_status[event.key.keysym.sym] = 0;
			break;
			case SDL_MOUSEMOTION:
			break;
			default:
				//printf("I don't know what this event is! (%d)\n", event.type);
			break;
		}
	}

	while (SDL_GetTicks() < ticks + 1000 / 60) SDL_Delay(1);
	ticks = SDL_GetTicks();

	SDL_GL_SwapBuffers();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return;
}

// Screen::title(string $title) : string
// Screen::title() : string
PHP_METHOD_ARGS(Screen, title) ARG_INFO(title) ZEND_END_ARG_INFO()
PHP_METHOD(Screen, title)
{
	char *str = NULL; int str_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|s", &str, &str_len) == FAILURE) RETURN_FALSE;

	if (str != NULL) {
		SDL_WM_SetCaption(str, NULL);
	}
	
	SDL_WM_GetCaption(&str, NULL);
	if (str != NULL) {
		RETURN_STRING(str, 1);
	} else {
		RETURN_FALSE;
	}
}

// Keyboard::pressed($key)
PHP_METHOD_ARGS(Keyboard, pressed) ARG_INFO(key) ZEND_END_ARG_INFO()
PHP_METHOD(Keyboard, pressed)
{
	int key;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "l", &key) == FAILURE) RETURN_FALSE;
	if (key >= 0 && key < SDLK_LAST && keys_pressed[key]) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

// Keyboard::down($key)
PHP_METHOD_ARGS(Keyboard, down) ARG_INFO(key) ZEND_END_ARG_INFO()
PHP_METHOD(Keyboard, down)
{
	int key;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "l", &key) == FAILURE) RETURN_FALSE;
	if (key >= 0 && key < SDLK_LAST && keys_status[key]) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

// Mouse::show()
PHP_METHOD_ARGS(Mouse, show) ZEND_END_ARG_INFO()
PHP_METHOD(Mouse, show)
{
	SDL_ShowCursor(SDL_ENABLE);
}

// Mouse::hide()
PHP_METHOD_ARGS(Mouse, hide) ZEND_END_ARG_INFO()
PHP_METHOD(Mouse, hide)
{
	SDL_ShowCursor(SDL_DISABLE);
}

// Audio::init()
PHP_METHOD_ARGS(Audio, init) ARG_INFO(frequency) ZEND_END_ARG_INFO()
PHP_METHOD(Audio, init)
{
	int frequency = 22050;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|l", &frequency) == FAILURE) RETURN_FALSE;
	Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, 2, 1024);
}

// Math::clamp(&$var, $min, $max)
PHP_METHOD_ARGS(Math, clamp) ARG_INFO(var) ARG_INFO(min) ARG_INFO(max) ZEND_END_ARG_INFO()
PHP_METHOD(Math, clamp)
{
	zval **var, *min, *max;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "Zzz", &var, &min, &max) == FAILURE) RETURN_FALSE;
	switch (Z_TYPE(**var)) {
		case IS_DOUBLE: {
			double min, max;
			if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "Zdd", &var, &min, &max) == FAILURE) RETURN_FALSE;
			if (Z_DVAL(**var) < min) Z_DVAL(**var) = min;
			if (Z_DVAL(**var) > max) Z_DVAL(**var) = max;
		} break;
		case IS_LONG: {
			int min, max;
			if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "Zll", &var, &min, &max) == FAILURE) RETURN_FALSE;
			if (Z_LVAL(**var) < min) Z_LVAL(**var) = min;
			if (Z_LVAL(**var) > max) Z_LVAL(**var) = max;
		} break;
		default: {
			int min, max;
			if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "Zll", &var, &min, &max) == FAILURE) RETURN_FALSE;
			ZVAL_LONG(*var, min);
		} break;
	}
}

PHP_FUNCTION(dummy)
{
	RETURN_LONG(0);
}

PM_METHODS(Bitmap)
{
	PHP_ME_AI(Bitmap, __construct, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, __set      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, __get      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, center     , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, clear      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, blit       , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, slice      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, split      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, fromFile   , ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, fromString , ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
	PHP_ME_END
};

PM_METHODS(Screen)
{
	PHP_ME_AI(Screen, init  , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_AI(Screen, frame , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_AI(Screen, title , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_END
};

PM_METHODS(Keyboard)
{
	PHP_ME_AI(Keyboard, pressed, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_AI(Keyboard, down   , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_END
};

PM_METHODS(Mouse)
{
	PHP_ME_AI(Mouse, show, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_AI(Mouse, hide, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_END
};

PM_METHODS(Audio)
{
	PHP_ME_AI(Audio, init, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_END
};

PM_METHODS(Math)
{
	PHP_ME_AI(Math, clamp, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_END
};

const zend_function_entry module_functions[] =
{
	PHP_FE(dummy, NULL)
	PHP_FE_END
};
	
static void register_classes(TSRMLS_D)
{
	zend_class_entry ClassEntry, *CurrentClassEntry;
	zend_object_handlers *CurrentHandlers;

	{ // Bitmap
		PM_CLASS_INIT(Bitmap);
		PM_CLASS_ADD(create_object, Bitmap__ObjectNew)
		PM_CLASS_REGISTER();
		Bitmap_ClassEntry = CurrentClassEntry;

		PM_HANDLERS_INIT(Bitmap_Handlers);
		PM_HANDLERS_ADD(clone_obj, Bitmap__ObjectClone);
	}

	{ // Screen
		PM_CLASS_INIT(Screen);
		PM_CLASS_REGISTER();

		// Constants
		CLASS_REGISTER_CONSTANT_STR("TEST", "1");
	}

	{ // Keyboard
		PM_CLASS_INIT(Keyboard);
		PM_CLASS_REGISTER();
		
		#define DEFINE_KEY_EX(KEY, VAL) CLASS_REGISTER_CONSTANT_INT(#KEY, SDLK_##VAL)
		#define DEFINE_KEY(KEY) CLASS_REGISTER_CONSTANT_INT(#KEY, SDLK_##KEY)

		// Constants
		DEFINE_KEY(UP);
		DEFINE_KEY(DOWN);
		DEFINE_KEY(LEFT);
		DEFINE_KEY(RIGHT);
		DEFINE_KEY(ESCAPE);
		DEFINE_KEY(SPACE);
		DEFINE_KEY(TAB);
		DEFINE_KEY_EX(ESC, ESCAPE);
		DEFINE_KEY_EX(ENTER, RETURN);
		DEFINE_KEY_EX(A, a);
		DEFINE_KEY_EX(B, b);
		DEFINE_KEY_EX(C, c);
		DEFINE_KEY_EX(D, d);
		DEFINE_KEY_EX(E, e);
		DEFINE_KEY_EX(F, f);
		DEFINE_KEY_EX(G, g);
		DEFINE_KEY_EX(H, h);
		DEFINE_KEY_EX(I, i);
		DEFINE_KEY_EX(J, j);
		DEFINE_KEY_EX(K, k);
		DEFINE_KEY_EX(L, l);
		DEFINE_KEY_EX(M, m);
		DEFINE_KEY_EX(N, n);
		DEFINE_KEY_EX(O, o);
		DEFINE_KEY_EX(P, p);
		DEFINE_KEY_EX(Q, q);
		DEFINE_KEY_EX(R, r);
		DEFINE_KEY_EX(S, s);
		DEFINE_KEY_EX(T, t);
		DEFINE_KEY_EX(U, u);
		DEFINE_KEY_EX(V, v);
		DEFINE_KEY_EX(W, w);
		DEFINE_KEY_EX(X, x);
		DEFINE_KEY_EX(Y, y);
		DEFINE_KEY_EX(Z, z);
	}

	{ // Mouse
		PM_CLASS_INIT(Mouse);
		PM_CLASS_REGISTER();

		// Constants
		CLASS_REGISTER_CONSTANT_INT("LEFT", 0);
		CLASS_REGISTER_CONSTANT_INT("MIDDLE", 1);
		CLASS_REGISTER_CONSTANT_INT("RIGHT", 2);
		CLASS_REGISTER_CONSTANT_INT("SCROLL_UP", 3);
		CLASS_REGISTER_CONSTANT_INT("SCROLL_DOWN", 4);
	}

	{ // Audio
		PM_CLASS_INIT(Audio);
		PM_CLASS_REGISTER();
	}

	{ // Math
		PM_CLASS_INIT(Math);
		PM_CLASS_REGISTER();
	}
}

PHP_MINIT_FUNCTION(module)
{
	register_classes(TSRMLS_C);
	
	SDL_Init(0);
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableKeyRepeat(120, 50);
	SDL_EnableUNICODE(1);

	memset(keys_pressed, 0, sizeof(keys_pressed));
	memset(keys_status, 0, sizeof(keys_status));
	
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(module)
{
	return 0;
}

PHP_MINFO_FUNCTION(module)
{
	char temp[800];
	php_info_print_table_start();
	php_info_print_table_row(2, "phpmedia", "2.0");
	{
		const SDL_version *v = SDL_Linked_Version();
		sprintf(temp, "%d.%d.%d", v->major, v->minor, v->patch);
		php_info_print_table_row(2, "SDL", temp);
	}
	php_info_print_table_end();
}


zend_module_entry module_module_entry =
{
	STANDARD_MODULE_HEADER,
	"phpmedia",
	module_functions,
	PHP_MINIT(module),
	PHP_MSHUTDOWN(module),
	NULL,
	NULL,
	PHP_MINFO(module),
    NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(module)