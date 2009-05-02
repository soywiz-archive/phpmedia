// http://bellard.org/tcc/

#define TCC_OUTPUT_MEMORY   0
#define TCC_OUTPUT_EXE      1
#define TCC_OUTPUT_DLL      2
#define TCC_OUTPUT_OBJ      3
#define TCC_OUTPUT_PREPROCESS 4
#define TCC_OUTPUT_FORMAT_ELF    0
#define TCC_OUTPUT_FORMAT_BINARY 1
#define TCC_OUTPUT_FORMAT_COFF   2

DEFINE_FUNC(TCCState *, tcc_new                  , void);
DEFINE_FUNC(void      , tcc_delete               , TCCState *s);
DEFINE_FUNC(void      , tcc_enable_debug         , TCCState *s);
DEFINE_FUNC(void      , tcc_set_error_func       , TCCState *s, void *error_opaque, void (*error_func)(void *opaque, const char *msg));
DEFINE_FUNC(int       , tcc_set_warning          , TCCState *s, const char *warning_name, int value);
DEFINE_FUNC(int       , tcc_add_include_path     , TCCState *s, const char *pathname);
DEFINE_FUNC(int       , tcc_add_sysinclude_path  , TCCState *s, const char *pathname);
DEFINE_FUNC(void      , tcc_define_symbol        , TCCState *s, const char *sym, const char *value);
DEFINE_FUNC(void      , tcc_undefine_symbol      , TCCState *s, const char *sym);
DEFINE_FUNC(int       , tcc_add_file             , TCCState *s, const char *filename);
DEFINE_FUNC(int       , tcc_compile_string       , TCCState *s, const char *buf);
DEFINE_FUNC(int       , tcc_set_output_type      , TCCState *s, int output_type);
DEFINE_FUNC(int       , tcc_add_library_path     , TCCState *s, const char *pathname);
DEFINE_FUNC(int       , tcc_add_library          , TCCState *s, const char *libraryname);
DEFINE_FUNC(int       , tcc_add_symbol           , TCCState *s, const char *name, unsigned long val);
DEFINE_FUNC(int       , tcc_output_file          , TCCState *s, const char *filename);
DEFINE_FUNC(int       , tcc_run                  , TCCState *s, int argc, char **argv);
DEFINE_FUNC(int       , tcc_relocate             , TCCState *s);
DEFINE_FUNC(int       , tcc_get_symbol           , TCCState *s, unsigned long *pval, const char *name);

int tcc_initialized = 0;

#define TCC_LOAD_FUNC(name) { LOAD_FUNC(name); if (name == NULL) { tcc_initialized = 0; fprintf(stderr, "Can't load TCC::%s\n", #name); } }

int tcc_init(TSRMLS_D) {
	if (!tcc_initialized) {
		HMODULE lib;

		tcc_initialized = 1;

		lib = LoadLibrary("libtcc.dll");
		TCC_LOAD_FUNC(tcc_new);
		TCC_LOAD_FUNC(tcc_delete);
		//TCC_LOAD_FUNC(tcc_enable_debug);
		TCC_LOAD_FUNC(tcc_set_error_func);
		TCC_LOAD_FUNC(tcc_set_warning);
		TCC_LOAD_FUNC(tcc_add_include_path);
		TCC_LOAD_FUNC(tcc_add_sysinclude_path);
		TCC_LOAD_FUNC(tcc_define_symbol);
		TCC_LOAD_FUNC(tcc_undefine_symbol);
		TCC_LOAD_FUNC(tcc_add_file);
		TCC_LOAD_FUNC(tcc_compile_string);
		TCC_LOAD_FUNC(tcc_set_output_type);
		TCC_LOAD_FUNC(tcc_add_library_path);
		TCC_LOAD_FUNC(tcc_add_library);
		TCC_LOAD_FUNC(tcc_add_symbol);
		TCC_LOAD_FUNC(tcc_output_file);
		TCC_LOAD_FUNC(tcc_run);
		TCC_LOAD_FUNC(tcc_relocate);
		TCC_LOAD_FUNC(tcc_get_symbol);
	}
	
	return tcc_initialized;
}

PM_OBJECTDELETE(TCC)
{
	if (object->state) {
		tcc_delete(object->state);
	}
	PM_OBJECTDELETE_STD;
}

PM_OBJECTCLONE_IMPL(TCC) {
}

PM_OBJECTNEW(TCC);
PM_OBJECTCLONE(TCC);

PHP_METHOD_ARGS(TCC, __construct) ZEND_END_ARG_INFO()
PHP_METHOD(TCC, __construct)
{
	THIS_TCC;
	if (!tcc_init(TSRMLS_C)) {
		THROWF("Can't load tcc.dll");
		return;
	}
	tcc->state = tcc_new();
}

PHP_METHOD_ARGS(TCC, sourceFile) ZEND_END_ARG_INFO()
PHP_METHOD(TCC, sourceFile)
{
	char *name = NULL; int name_len = 0;
	THIS_TCC;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &name, &name_len) == FAILURE) RETURN_FALSE;

	RETURN_LONG(tcc_add_file(tcc->state, name));
}

PHP_METHOD_ARGS(TCC, libFile) ZEND_END_ARG_INFO()
PHP_METHOD(TCC, libFile)
{
	char *name = NULL; int name_len = 0;
	THIS_TCC;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &name, &name_len) == FAILURE) RETURN_FALSE;

	RETURN_LONG(tcc_add_library(tcc->state, name));
}

PHP_METHOD_ARGS(TCC, libPath) ZEND_END_ARG_INFO()
PHP_METHOD(TCC, libPath)
{
	char *name = NULL; int name_len = 0;
	THIS_TCC;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &name, &name_len) == FAILURE) RETURN_FALSE;

	RETURN_LONG(tcc_add_library_path(tcc->state, name));
}

PHP_METHOD_ARGS(TCC, includePath) ZEND_END_ARG_INFO()
PHP_METHOD(TCC, includePath)
{
	char *name = NULL; int name_len = 0;
	THIS_TCC;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &name, &name_len) == FAILURE) RETURN_FALSE;

	tcc_add_include_path(tcc->state, name);
	tcc_add_sysinclude_path(tcc->state, name);
}

PHP_METHOD_ARGS(TCC, sourceString) ZEND_END_ARG_INFO()
PHP_METHOD(TCC, sourceString)
{
	char *source = NULL; int source_len = 0;
	THIS_TCC;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &source, &source_len) == FAILURE) RETURN_FALSE;

	RETURN_LONG(tcc_compile_string(tcc->state, source));
}

PHP_METHOD_ARGS(TCC, define) ZEND_END_ARG_INFO()
PHP_METHOD(TCC, define)
{
	char *key = NULL, *value = NULL; int key_len = 0, value_len = 0;
	THIS_TCC;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s|s", &key, &key_len, &value, &value_len) == FAILURE) RETURN_FALSE;

	if (value == NULL) value = "1";

	if (key && value) tcc_define_symbol(tcc->state, key, value);

	RETURN_TRUE;
}

PHP_METHOD_ARGS(TCC, runMain) ZEND_END_ARG_INFO()
PHP_METHOD(TCC, runMain)
{
	THIS_TCC;
	RETURN_LONG(tcc_run(tcc->state, 0, NULL));
}

PHP_METHOD_ARGS(TCC, call) ZEND_END_ARG_INFO()
PHP_METHOD(TCC, call)
{
	char symbol[0x100], params[0x100], retype = '\0';
	unsigned int int_params[0x40], int_params_count = 0;
	zval *array = NULL, **entry = NULL;
	char *key; int key_len;
	char *ptr1, *ptr2;
	int retval = 0;
	LONG addr;
	int n, l, cparam;
	HashTable *hash_table;
	THIS_TCC;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s|a", &key, &key_len, &array) == FAILURE) RETURN_FALSE;

	memset(symbol, 0, sizeof(symbol));
	memset(params, 0, sizeof(params));

	ptr1 = key;
	ptr2 = strchr(ptr1, ':');
	if (ptr2) {
		strncpy(symbol, ptr1, ptr2 - ptr1);
		ptr1 = ptr2 + 1;
		ptr2 = strchr(ptr1, ':');
		if (ptr2) {
			strncpy(params, ptr1, ptr2 - ptr1);
			ptr1 = ptr2 + 1;
			retype = *ptr1;
		}
	}

	//printf("%s\t%s\t%c\n", symbol, params, retype);

	if (array) {
		hash_table = Z_ARRVAL_P(array);

		//count = zend_hash_num_elements(hash_table);

		cparam = 0;
		for (n = 0, l = strlen(params); n < l; n++) {
			zval **v = NULL;
			zend_hash_index_find(hash_table, cparam, (void **)&v);
			if (v == NULL) continue;
			switch (params[n]) {
				case 's': case 'S': case 'Z': {
					convert_to_string(*v);
					int_params[int_params_count++] = (unsigned int)(void *)(char *)Z_STRVAL(**v);
					if (params[n] == 'S') {
						//typedef struct { char *ptr; int  len; } STRING;
						int_params[int_params_count++] = (unsigned int)Z_STRLEN(**v);
					} else if (params[n] == 'Z') {
						// typedef struct { char *ptr; int *len; } STRING_REF;
						int_params[int_params_count++] = (unsigned int)(void *)&Z_STRLEN(**v);
					}
				} break;
				case 'l': {
					convert_to_long(*v);
					int_params[int_params_count++] = (unsigned int)Z_LVAL(**v);
				} break;
				case 'f': {
					float f;
					convert_to_double(*v);
					f = (float)Z_DVAL(**v);
					int_params[int_params_count++] = *(unsigned int *)&f;
				} break;
				default:
					THROWF("Unknown param type '%c'", params[n]);
				break;
			}
			cparam++;
		}
	}

	//printf("%d\n", int_params_count);
	//printf("%s\n", int_params[0]);

	if (!tcc->relocated) {
		tcc_relocate(tcc->state);
		tcc->relocated = 1;
	}
	tcc_get_symbol(tcc->state, &addr, symbol);

	if (addr != 0) {
		char *call_start = malloc(0x1000);
		char *call = call_start;
		int n;

		#define PUSH(v) \
			*call++ = 0x68; \
			*(unsigned int *)call = (unsigned int)(void *)(v); \
			call += 4;

		#define CALL(v) { \
			*call++ = 0xB8; \
			*(unsigned int *)call = (unsigned int)(void *)(v); \
			call += 4; \
			*call++ = 0xFF; \
			*call++ = 0xD0; \
		}

		#define RET() *call++ = 0xC3;
		#define NOP() *call++ = 0x90;
		#define ADD_ESP(v) \
			*call++ = 0x83; \
			*call++ = 0xC4; \
			*call++ = (v);

		for (n = int_params_count - 1; n >= 0; n--) {
			PUSH(int_params[n]);
		}
		CALL(addr);
		// Calling "C"
		if (1) {
			ADD_ESP(int_params_count * 4);
		}
		RET();

		_asm {
			call call_start;
			mov retval, eax;
		}

		free(call_start);
	}

	switch (retype) {
		case '0': RETURN_FALSE; break;
		case 'l': RETURN_LONG(retval); break;
		case 's': RETURN_STRING((char *)(void *)retval, 1); break;
		default:
			THROWF("Unknown return type '%c'", retype);
		break;
	}

	// l - long (int32)
	// f - float (float32)
	// p - pointer (pointer to a string)
	// s - string (pointer to a string + length)
	// 0 - void

	// func:ss:s

	/*
	if (s = tcc_new()) {
		//tcc_enable_debug(s);
		tcc_compile_string(s, "void main() { int n, v = 0; for (n = 0; n < 999999999; n++) v++; printf(\"%d\n\", n); }");
		//tcc_run(s, 0, NULL);
		tcc_relocate(s);
		tcc_get_symbol(s, &addr, "main");
		__asm {
			call addr;
		}
		//((void *())(void *)addr)();
		//printf("%08X\n", addr);
		tcc_delete(s);
	}
	*/
}