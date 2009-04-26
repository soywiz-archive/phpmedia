static void Sound__ObjectDelete(SoundStruct *sound, TSRMLS_D)
{
	if (sound->chunk != NULL)
	{
		Mix_FreeChunk(sound->chunk);
	}
	zend_object_std_dtor(&sound->std, TSRMLS_C);
	efree(sound);
}

static zend_object_value Sound__ObjectNew_ex(zend_class_entry *class_type, SoundStruct **ptr, TSRMLS_D)
{
	SoundStruct *intern;
	zend_object_value retval;
	zval *tmp;

	STRUCT_CREATE(SoundStruct, intern);
	if (ptr != NULL) *ptr = intern;
	
	zend_object_std_init(&intern->std, class_type, TSRMLS_C);
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
	
	retval.handle = zend_objects_store_put(
		intern,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t) Sound__ObjectDelete,
		NULL,
		TSRMLS_C
	);

	retval.handlers = &Handlers_Sound;
	
	return retval;
}

static zend_object_value Sound__ObjectNew(zend_class_entry *class_type, TSRMLS_D)
{
	return Sound__ObjectNew_ex(class_type, NULL, TSRMLS_C);
}

static zend_object_value Sound__ObjectClone(zval *this_ptr, TSRMLS_D)
{
	SoundStruct *new_obj = NULL;
	SoundStruct *old_obj = (SoundStruct *)zend_object_store_get_object(this_ptr, TSRMLS_C);

	zend_object_value new_ov = Sound__ObjectNew_ex(old_obj->std.ce, &new_obj, TSRMLS_C);
	
	zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr), TSRMLS_C);
	
	return new_ov;
}

int Sound_fromRW(zval **return_value, SDL_RWops *rw, TSRMLS_D)
{
	Mix_Chunk *chunk;
	SoundStruct *sound;

	if ((chunk = Mix_LoadWAV_RW(rw, 1)) == NULL) return 0;
		
	ObjectInit(ClassEntry_Sound, *return_value, TSRMLS_C);
	sound = zend_object_store_get_object(*return_value, TSRMLS_C);
	sound->chunk = chunk;

	return 1;
}

// Sound::fromFile($file)
PHP_METHOD_ARGS(Sound, fromFile) ARG_INFO(file) ZEND_END_ARG_INFO()
PHP_METHOD(Sound, fromFile)
{
	char *name = NULL;
	int name_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &name, &name_len) == FAILURE) RETURN_FALSE;

	if (!Sound_fromRW(&return_value, SDL_RWFromFile(name, "r"), TSRMLS_C)) THROWF("Can't load sound from file: '%s'", name);
}

// Sound::fromString($data)
PHP_METHOD_ARGS(Sound, fromString) ARG_INFO(data) ZEND_END_ARG_INFO()
PHP_METHOD(Sound, fromString)
{
	char *data = NULL;
	int data_len = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &data, &data_len) == FAILURE) RETURN_FALSE;

	if (!Sound_fromRW(&return_value, SDL_RWFromConstMem(data, data_len), TSRMLS_C)) THROWF("Can't load sound from string");
}

// Sound::play()
PHP_METHOD_ARGS(Sound, play) ZEND_END_ARG_INFO()
PHP_METHOD(Sound, play)
{
	THIS_SOUND;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "") == FAILURE) RETURN_FALSE;

	Mix_PlayChannel(-1, sound->chunk, 0);
	//Mix_FadeInChannelTimed(-1, sound->chunk, 0, 0, 0);
}

// Audio::init($frequency = 22050)
PHP_METHOD_ARGS(Audio, init) ARG_INFO(frequency) ZEND_END_ARG_INFO()
PHP_METHOD(Audio, init)
{
	int frequency = 22050;
	//int frequency = 44100;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|l", &frequency) == FAILURE) RETURN_FALSE;
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
		THROWF("Can't initialize audio");
	}
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
		music = NULL;
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