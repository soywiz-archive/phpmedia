PM_OBJECTDELETE(Font)
{
	if (object->font) {
		TTF_CloseFont(object->font);
	}
	PM_OBJECTDELETE_STD;
}

PM_OBJECTCLONE_IMPL(Font) {
	CLONE_COPY_FIELD(font);
}

PM_OBJECTNEW(Font);
PM_OBJECTCLONE(Font);

void FontCheckInit() { if (!TTF_WasInit()) TTF_Init(); }

// Font::fromFile($file, $size = 16, $index = 0)
PHP_METHOD_ARGS(Font, fromFile) ARG_INFO(file) ARG_INFO(size) ARG_INFO(index) ZEND_END_ARG_INFO()
PHP_METHOD(Font, fromFile)
{
	char *name = NULL; int name_len = 0; int size = 16, index = 0;
	FontStruct *object;
	FontCheckInit(); if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s|ll", &name, &name_len, &size, &index) == FAILURE) RETURN_FALSE;

	//ObjectInit(ClassEntry_Font, return_value, TSRMLS_C);
	ObjectInit(EG(called_scope), return_value, TSRMLS_C);
	object = zend_object_store_get_object(return_value, TSRMLS_C);	

	object->font = TTF_OpenFontIndexRW(SDL_RWFromFile(name, "r"), 1, size, index);
	if (object->font == NULL) {
		THROWF("Can't load from from file('%s') with size(%d)", name, size);
	}
}

// Font::fromString($data, $size = 16, $index = 0)
PHP_METHOD_ARGS(Font, fromString) ARG_INFO(data) ARG_INFO(size) ARG_INFO(index) ZEND_END_ARG_INFO()
PHP_METHOD(Font, fromString)
{
}

// Font::width($text = '')
PHP_METHOD_ARGS(Font, width) ARG_INFO(text) ZEND_END_ARG_INFO()
PHP_METHOD(Font, width)
{
	char *text = NULL; int text_len = 0;
	THIS_FONT;
	FontCheckInit(); if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|s", &text, &text_len) == FAILURE) RETURN_FALSE;
}

// Font::height($text = '')
PHP_METHOD_ARGS(Font, height) ARG_INFO(text) ZEND_END_ARG_INFO()
PHP_METHOD(Font, height)
{
	char *text = NULL; int text_len = 0, lines = 1, n;
	THIS_FONT;
	FontCheckInit(); if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|s", &text, &text_len) == FAILURE) RETURN_FALSE;
	for (n = 0; n < text_len; n++) {
		if (text[n] == '\n') lines++;
	}
	RETURN_LONG(TTF_FontHeight(font->font) * lines);
	
}

// Font::blit(Bitmap $dest, $x, $y, $color)
PHP_METHOD_ARGS(Font, blit) ARG_INFO(text) ZEND_END_ARG_INFO()
PHP_METHOD(Font, blit)
{
	THIS_FONT;
	FontCheckInit(); if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "") == FAILURE) RETURN_FALSE;
}
