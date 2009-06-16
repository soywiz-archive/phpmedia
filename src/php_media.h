#include "php.h"
#include "php_ini.h"
#include "zend_exceptions.h"
#include "SAPI.h"
#include "ext/standard/info.h"
#ifdef WIN32
	#include <windows.h>
	#include <shlobj.h>
	#include <winreg.h>
#endif
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <GL/gl.h>
#include <GL/glext.h>

struct TCCState;
typedef struct TCCState TCCState;

typedef struct {
	zend_object std;
	TCCState *state;
	int relocated;
} TCCStruct;

typedef struct _BitmapStruct {
	zend_object std;
	struct _BitmapStruct *parent;
	SDL_Surface *surface;
	GLint gltex;

	int cx, cy;
	int x, y, w, h;
	int smooth;
	int real_w, real_h;
} BitmapStruct;

typedef struct {
	zend_object std;
	Mix_Chunk *chunk;
} SoundStruct;

typedef struct {
	zend_object std;
	GLuint program;
	GLuint shader_fragment;
	GLuint shader_vertex;
} ShaderStruct;

typedef struct {
	Uint16 ch;
	Uint16 used;
	GLuint gltex;
	GLuint list;
	SDL_Surface *surface;
} FontGlyphCache;

#define GLYPH_MAX_CACHE 0x400
typedef struct {
	zend_object std;
	TTF_Font *font;
	FontGlyphCache glyphs[GLYPH_MAX_CACHE];
} FontStruct;

void sdl_load(TSRMLS_D);

#define PHP_METHODS_NAME(CLASS) CLASS##_Methods
#define PHP_METHOD_NAME_ARGINFO(CLASS, METHOD) arginfo_##CLASS##METHOD

//zval *object = getThis();
#define GET_THIS_TYPE(TYPE, NAME) TYPE *NAME = (TYPE *)zend_object_store_get_object(getThis(), TSRMLS_C);
#define THROWF(...) zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0, TSRMLS_C, __VA_ARGS__);
#define STRUCT_CREATE(TYPE, VAR) VAR = emalloc(sizeof(TYPE)); memset(VAR, 0, sizeof(TYPE));
#define PHP_METHOD_ARGS(CLASS, METHOD) ZEND_BEGIN_ARG_INFO_EX(PHP_METHOD_NAME_ARGINFO(CLASS, METHOD), 0, 0, 1)
#define ARG_INFO(name) ZEND_ARG_INFO(0, name)

// This
#define THIS_BITMAP    GET_THIS_TYPE(BitmapStruct, bitmap);
#define THIS_SOUND     GET_THIS_TYPE(SoundStruct, sound);
#define THIS_SHADER    GET_THIS_TYPE(ShaderStruct, shader);
#define THIS_FONT      GET_THIS_TYPE(FontStruct, font);
#define THIS_TCC       GET_THIS_TYPE(TCCStruct, tcc);

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
#define PM_OBJECTDELETE(TYPE) static void TYPE##__ObjectDelete(TYPE##Struct *object, TSRMLS_D)
#define PM_OBJECTDELETE_STD { zend_object_std_dtor(&object->std, TSRMLS_C); efree(object); }
#define PM_OBJECTNEW(TYPE) \
	static zend_object_value TYPE##__ObjectNew_ex(zend_class_entry *class_type, TYPE##Struct **ptr, TSRMLS_D) \
	{ \
		TYPE##Struct *intern; \
		zend_object_value retval; \
		zval *tmp; \
		 \
		STRUCT_CREATE(TYPE##Struct, intern); \
		if (ptr != NULL) *ptr = intern; \
		 \
		zend_object_std_init(&intern->std, class_type, TSRMLS_C); \
		zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *)); \
		 \
		retval.handle = zend_objects_store_put( \
			intern, \
			(zend_objects_store_dtor_t)zend_objects_destroy_object, \
			(zend_objects_free_object_storage_t) TYPE##__ObjectDelete, \
			NULL, \
			TSRMLS_C \
		); \
	 \
		retval.handlers = &Handlers_##TYPE; \
		 \
		return retval; \
	} \
	 \
	static zend_object_value TYPE##__ObjectNew(zend_class_entry *class_type, TSRMLS_D) \
	{ \
		return TYPE##__ObjectNew_ex(class_type, NULL, TSRMLS_C); \
	}

#define PM_OBJECTCLONE_IMPL(TYPE) PM_Clone_##TYPE(TYPE##Struct *old_obj, TYPE##Struct *new_obj)

#define PM_OBJECTCLONE(TYPE) \
	static zend_object_value TYPE##__ObjectClone(zval *this_ptr, TSRMLS_D) \
	{ \
		TYPE##Struct *new_obj = NULL; \
		TYPE##Struct *old_obj = (TYPE##Struct *)zend_object_store_get_object(this_ptr, TSRMLS_C); \
		zend_object_value new_ov = TYPE##__ObjectNew_ex(old_obj->std.ce, &new_obj, TSRMLS_C); \
		zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr), TSRMLS_C); \
		PM_Clone_##TYPE(old_obj, new_obj); \
		return new_ov; \
	}
	
#define DEFINE_FUNC(RETVAL, NAME, ...) RETVAL (*NAME) (__VA_ARGS__) = NULL;
#define LOAD_FUNC(name) name = (void *)GetProcAddress(lib, #name);
