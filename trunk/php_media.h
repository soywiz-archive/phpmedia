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

typedef struct _SoundStruct {
	zend_object std;
	Mix_Chunk *chunk;
} SoundStruct;

#define PHP_METHODS_NAME(CLASS) CLASS##_Methods
#define PHP_METHOD_NAME_ARGINFO(CLASS, METHOD) arginfo_##CLASS##METHOD

//zval *object = getThis();
#define GET_THIS_TYPE(TYPE, NAME) TYPE *NAME = (TYPE *)zend_object_store_get_object(getThis(), TSRMLS_C);
#define THROWF(...) zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0, TSRMLS_C, __VA_ARGS__);
#define STRUCT_CREATE(TYPE, VAR) VAR = emalloc(sizeof(TYPE)); memset(VAR, 0, sizeof(TYPE));
#define PHP_METHOD_ARGS(CLASS, METHOD) ZEND_BEGIN_ARG_INFO_EX(PHP_METHOD_NAME_ARGINFO(CLASS, METHOD), 0, 0, 1)
#define ARG_INFO(name) ZEND_ARG_INFO(0, name)
#define THIS_BITMAP GET_THIS_TYPE(BitmapStruct, bitmap);
#define THIS_SOUND GET_THIS_TYPE(SoundStruct, sound);
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