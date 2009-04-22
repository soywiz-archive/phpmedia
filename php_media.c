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
#include "php_media.h"

Mix_Music *music = NULL;
SDL_Surface *screen;
int keys_status[SDLK_LAST];
int keys_pressed[SDLK_LAST];

static zend_object_handlers Bitmap_Handlers;
static zend_class_entry    *Bitmap_ClassEntry;

int __texPow2 = 0;
int __texRectangle = 0;

int extframebuffer = 0;
GLuint fbo = -1;

PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC        glGenFramebuffersEXT        = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC   glFramebufferTexture2DEXT   = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC     glDeleteFramebuffersEXT     = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC        glBindFramebufferEXT        = NULL;

#include "php_media_utils.c"
#include "php_media_bitmap.c"
#include "php_media_screen.c"
#include "php_media_input.c"
#include "php_media_audio.c"
#include "php_media_math.c"

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

PM_METHODS(Music)
{
	PHP_ME_AI(Music, play, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_AI(Music, stop, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_AI(Music, playing, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
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

	{ // Music
		PM_CLASS_INIT(Music);
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