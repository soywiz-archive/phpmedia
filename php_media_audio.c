// Audio::init($frequency = 22050)
PHP_METHOD_ARGS(Audio, init) ARG_INFO(frequency) ZEND_END_ARG_INFO()
PHP_METHOD(Audio, init)
{
	int frequency = 22050;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|l", &frequency) == FAILURE) RETURN_FALSE;
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, 2, 1024);
}

// Music::play($file, $loops = -1, $fadeIn = 1, $position = 0)
PHP_METHOD_ARGS(Music, play) ARG_INFO(frequency) ARG_INFO(loops) ARG_INFO(fadeIn) ARG_INFO(position) ZEND_END_ARG_INFO()
PHP_METHOD(Music, play)
{
	char *str = NULL; int str_len = 0;
	int loops; double fadeIn = 0, position = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s|ldd", &str, &str_len, &loops, &fadeIn, &position) == FAILURE) RETURN_FALSE;
	if (music != NULL) {
		Mix_HaltMusic();
		Mix_FreeMusic(music);
	}
	music = Mix_LoadMUS(str);
	Mix_FadeInMusicPos(music, loops, (int)(fadeIn) * 1000, position);
}

// Music::stop($fadeOut = 0)
PHP_METHOD_ARGS(Music, stop) ARG_INFO(fadeOut) ZEND_END_ARG_INFO()
PHP_METHOD(Music, stop)
{
	double fadeOut = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|d", &fadeOut) == FAILURE) RETURN_FALSE;
	Mix_FadeOutMusic((int)(fadeOut) * 1000);
}

// Music::playing()
PHP_METHOD_ARGS(Music, playing) ZEND_END_ARG_INFO()
PHP_METHOD(Music, playing)
{
	double fadeOut = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|d", &fadeOut) == FAILURE) RETURN_FALSE;
	Mix_FadeOutMusic((int)fadeOut);
}