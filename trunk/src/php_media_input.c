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

// Mouse::position()
// Mouse::position($x, $y)
PHP_METHOD_ARGS(Mouse, position) ARG_INFO(x) ARG_INFO(y) ZEND_END_ARG_INFO()
PHP_METHOD(Mouse, position)
{
	int x, y;
	if (ZEND_NUM_ARGS() == 0) {
		SDL_GetMouseState(&x, &y);
		array_init(return_value);
		add_next_index_long(return_value, x);
		add_next_index_long(return_value, y);
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ll", &x, &y) == FAILURE) RETURN_FALSE;
		SDL_WarpMouse(x, y);
	}
}

// Mouse::pressed($but)
PHP_METHOD_ARGS(Mouse, pressed) ARG_INFO(but) ZEND_END_ARG_INFO()
PHP_METHOD(Mouse, pressed)
{
	int but;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "l", &but) == FAILURE) RETURN_FALSE;
	if (but >= 0 && but < 6 && mouse_pressed[but]) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

// Mouse::down($but)
PHP_METHOD_ARGS(Mouse, down) ARG_INFO(but) ZEND_END_ARG_INFO()
PHP_METHOD(Mouse, down)
{
	int but;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "l", &but) == FAILURE) RETURN_FALSE;
	if (but >= 0 && but < 6 && mouse_status[but]) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}