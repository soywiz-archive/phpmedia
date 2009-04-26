PM_OBJECTDELETE(Font)
{
	PM_OBJECTDELETE_STD;
}

PM_OBJECTCLONE_IMPL(Font) {
}

PM_OBJECTNEW(Font);
PM_OBJECTCLONE(Font);


void FontCheckInit() { if (!TTF_WasInit()) TTF_Init(); }

// Font::fromFile($file, $size = 16)
PHP_METHOD_ARGS(Font, fromFile) ARG_INFO(file) ARG_INFO(size) ZEND_END_ARG_INFO()
PHP_METHOD(Font, fromFile)
{
	char *name = NULL; int name_len = 0; int size = 16;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s|l", &name, &name_len, &size) == FAILURE) RETURN_FALSE;
	FontCheckInit();
}

// Font::width($text = '')
PHP_METHOD_ARGS(Font, width) ARG_INFO(text) ZEND_END_ARG_INFO()
PHP_METHOD(Font, width)
{
	char *text = NULL; int text_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|s", &text, &text_len) == FAILURE) RETURN_FALSE;
	FontCheckInit();
}

// Font::height($text = '')
PHP_METHOD_ARGS(Font, height) ARG_INFO(text) ZEND_END_ARG_INFO()
PHP_METHOD(Font, height)
{
	char *text = NULL; int text_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &text, &text_len) == FAILURE) RETURN_FALSE;
	FontCheckInit();
}

// Font::blit(Bitmap $dest, $x, $y, $color)
PHP_METHOD_ARGS(Font, blit) ARG_INFO(text) ZEND_END_ARG_INFO()
PHP_METHOD(Font, blit)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "") == FAILURE) RETURN_FALSE;
	FontCheckInit();
}
