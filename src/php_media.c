#include "php_media.h"

#define CONFIG_TCCDIR "."
#define GCC_MAJOR "3.2"
#define HOST_I386 1
#define TCC_TARGET_I386 1
#define CONFIG_WIN32 1
#define HAVE_GPROF 1
#define TCC_VERSION "0.9.24"

Mix_Music *music = NULL;
SDL_Surface *screen;
int keys_status[SDLK_LAST];
int keys_pressed[SDLK_LAST];
int mouse_status[0x10];
int mouse_pressed[0x10];
static char gl_error[0x800];

#define CLASS_EXTRA_STRUCT(CLASS) static zend_object_handlers Handlers_##CLASS; static zend_class_entry *ClassEntry_##CLASS;

CLASS_EXTRA_STRUCT(Bitmap);
CLASS_EXTRA_STRUCT(Sound);
CLASS_EXTRA_STRUCT(Shader);
CLASS_EXTRA_STRUCT(Font);
CLASS_EXTRA_STRUCT(TCC);

#include "php_media_utils.c"
#include "php_media_shader.c"
#include "php_media_bitmap.c"
#include "php_media_screen.c"
#include "php_media_input.c"
#include "php_media_audio.c"
#include "php_media_font.c"
#include "php_media_math.c"
#include "php_media_tcc.c"

PM_METHODS(Font)
{
	PHP_ME_AI(Font, __set     , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Font, __get     , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Font, fromFile  , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Font, fromString, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Font, fromName  , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Font, metrics   , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Font, width     , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Font, height    , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Font, blit      , ZEND_ACC_PUBLIC)
	PHP_ME_END
};

PM_METHODS(Bitmap)
{
	PHP_ME_AI(Bitmap, __construct, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, __set      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, __get      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, fromFile   , ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, fromString , ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, center     , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, clear      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, blit       , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, slice      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, split      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Bitmap, saveFile   , ZEND_ACC_PUBLIC)
	PHP_ME_END
};

PM_METHODS(Screen)
{
	PHP_ME_AI(Screen, init  , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Screen, frame , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Screen, title , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_END
};

PM_METHODS(Keyboard)
{
	PHP_ME_AI(Keyboard, pressed, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Keyboard, down   , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_END
};

PM_METHODS(Mouse)
{
	PHP_ME_AI(Mouse, show    , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Mouse, hide    , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Mouse, position, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Mouse, pressed , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Mouse, down    , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_END
};

PM_METHODS(Audio)
{
	PHP_ME_AI(Audio, init, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_END
};

PM_METHODS(Sound)
{
	PHP_ME_AI(Sound, fromFile, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Sound, fromString, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Sound, play, ZEND_ACC_PUBLIC)
	PHP_ME_END
};

PM_METHODS(Music)
{
	PHP_ME_AI(Music, play, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Music, stop, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_AI(Music, playing, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME_END
};

PM_METHODS(Math)
{
	PHP_ME_AI(Math, clamp, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_AI(Math, smoothstep, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_END
};

PM_METHODS(Shader)
{
	PHP_ME_AI(Shader, __construct, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	PHP_ME_AI(Shader, __get      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Shader, begin      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(Shader, end        , ZEND_ACC_PUBLIC)
	PHP_ME_END
};

PM_METHODS(External)
{
	/*
	PHP_ME_AI(External, load  , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_AI(External, unload, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME_AI(External, call  , ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	*/
	PHP_ME_END
};

PM_METHODS(TCC)
{
	PHP_ME_AI(TCC, __construct , ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME_AI(TCC, define      , ZEND_ACC_PUBLIC)
	PHP_ME_AI(TCC, sourceString, ZEND_ACC_PUBLIC)
	PHP_ME_AI(TCC, sourceFile  , ZEND_ACC_PUBLIC)
	PHP_ME_AI(TCC, libFile     , ZEND_ACC_PUBLIC)
	PHP_ME_AI(TCC, libPath     , ZEND_ACC_PUBLIC)
	PHP_ME_AI(TCC, includePath , ZEND_ACC_PUBLIC)
	PHP_ME_AI(TCC, runMain     , ZEND_ACC_PUBLIC)
	PHP_ME_AI(TCC, call        , ZEND_ACC_PUBLIC)
	PHP_ME_END
};

const zend_function_entry module_functions[] =
{
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
		ClassEntry_Bitmap = CurrentClassEntry;

		PM_HANDLERS_INIT(Handlers_Bitmap);
		PM_HANDLERS_ADD(clone_obj, Bitmap__ObjectClone);
		
		CLASS_REGISTER_CONSTANT_INT("FORMAT_BMP", 0);
		CLASS_REGISTER_CONSTANT_INT("FORMAT_TGA", 1);
		CLASS_REGISTER_CONSTANT_INT("FORMAT_PNG", 2);
		CLASS_REGISTER_CONSTANT_INT("FORMAT_JPG", 3);
	}
	
	{ // Shader
		PM_CLASS_INIT(Shader);
		PM_CLASS_ADD(create_object, Shader__ObjectNew)
		PM_CLASS_REGISTER();
		ClassEntry_Shader = CurrentClassEntry;

		PM_HANDLERS_INIT(Handlers_Shader);
		PM_HANDLERS_ADD(clone_obj, Shader__ObjectClone);
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
		CLASS_REGISTER_CONSTANT_INT("LEFT"      , SDL_BUTTON_LEFT);
		CLASS_REGISTER_CONSTANT_INT("MIDDLE"    , SDL_BUTTON_MIDDLE);
		CLASS_REGISTER_CONSTANT_INT("RIGHT"     , SDL_BUTTON_RIGHT);
		CLASS_REGISTER_CONSTANT_INT("WHEEL_UP"  , SDL_BUTTON_WHEELUP);
		CLASS_REGISTER_CONSTANT_INT("WHEEL_DOWN", SDL_BUTTON_WHEELDOWN);
	}

	{ // Audio
		PM_CLASS_INIT(Audio);
		PM_CLASS_REGISTER();
	}

	{ // Sound
		PM_CLASS_INIT(Sound);
		PM_CLASS_ADD(create_object, Sound__ObjectNew)
		PM_CLASS_REGISTER();
		ClassEntry_Sound = CurrentClassEntry;
		PM_HANDLERS_INIT(Handlers_Sound);
		PM_HANDLERS_ADD(clone_obj, Sound__ObjectClone);
	}

	{ // Music
		PM_CLASS_INIT(Music);
		PM_CLASS_REGISTER();
	}

	{ // Font
		PM_CLASS_INIT(Font);
		PM_CLASS_ADD(create_object, Font__ObjectNew)
		PM_CLASS_REGISTER();
		ClassEntry_Font = CurrentClassEntry;
		PM_HANDLERS_INIT(Handlers_Font);
		PM_HANDLERS_ADD(clone_obj, Font__ObjectClone);
		
		CLASS_REGISTER_CONSTANT_INT("STYLE_NORMAL", TTF_STYLE_NORMAL);
		CLASS_REGISTER_CONSTANT_INT("STYLE_BOLD", TTF_STYLE_BOLD);
		CLASS_REGISTER_CONSTANT_INT("STYLE_ITALIC", TTF_STYLE_ITALIC);
		CLASS_REGISTER_CONSTANT_INT("STYLE_UNDERLINE", TTF_STYLE_UNDERLINE);
	}

	{ // Math
		PM_CLASS_INIT(Math);
		PM_CLASS_REGISTER();
	}
	
	{ // External
		PM_CLASS_INIT(External);
		PM_CLASS_REGISTER();
	}

	{ // TCC
		PM_CLASS_INIT(TCC);
		PM_CLASS_ADD(create_object, TCC__ObjectNew)
		PM_CLASS_REGISTER();
		ClassEntry_TCC = CurrentClassEntry;
		PM_HANDLERS_INIT(Handlers_TCC);
		PM_HANDLERS_ADD(clone_obj, TCC__ObjectClone);
	}
}

PHP_MINIT_FUNCTION(module)
{
	register_classes(TSRMLS_C);
	
	if (SDL_Init(0) != 0) THROWF("Can't initialize SDL");
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) THROWF("Can't initialize video subsystem");
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