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
