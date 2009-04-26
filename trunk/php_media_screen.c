#ifdef WIN32
HICON icon;
struct zWMcursor { void* curs; };
HWND hwnd;
SDL_SysWMinfo wminfo;
#endif

// Screen::init($w, $h, $fullscreen = false)
PHP_METHOD_ARGS(Screen, init) ARG_INFO(w) ARG_INFO(h) ARG_INFO(fullscreen) ZEND_END_ARG_INFO()
PHP_METHOD(Screen, init)
{
	int w, h, fullscreen = 0, flags = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ll|l", &w, &h, &fullscreen) == FAILURE) RETURN_FALSE;

	flags |= SDL_OPENGL;
	flags |= SDL_HWSURFACE;
	if (fullscreen) flags |= SDL_FULLSCREEN;

	#ifdef WIN32
	{
		SDL_Cursor *cursor = SDL_GetCursor();

		HINSTANCE handle = GetModuleHandle(NULL);
		//((struct zWMcursor *)cursor->wm_cursor)->curs = (void *)LoadCursorA(NULL, IDC_ARROW);
		((struct zWMcursor *)cursor->wm_cursor)->curs = (void *)LoadCursor(NULL, IDC_ARROW);
		SDL_SetCursor(cursor);

		icon = LoadIcon(handle, (char *)101);
		SDL_GetWMInfo(&wminfo);
		hwnd = (HANDLE)wminfo.window;
		SetClassLong(hwnd, GCL_HICON, (LONG)icon);
	}
	#endif

	SDL_putenv("SDL_VIDEO_WINDOW_POS=center");
	SDL_putenv("SDL_VIDEO_CENTERED=center");

	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	screen = SDL_SetVideoMode(w, h, 0, flags);

    #ifdef WIN32
    	SetClassLong(hwnd, GCL_HICON, (LONG)icon);
    #endif

	glEnable(GL_LINE_SMOOTH);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	SDL_GL_SwapBuffers();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glViewport(0, 0, w, h + 1);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_TEXTURE   ); glLoadIdentity();
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(0, w, h, 0, -1.0, 1.0);
	glTranslatef(0, 1, 0);

	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glShadeModel(GL_SMOOTH);

	glEnable(GL_SCISSOR_TEST);
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (screen) {
		ObjectInit(ClassEntry_Bitmap, return_value, TSRMLS_C);
		{
			BitmapStruct *bmp = zend_object_store_get_object(return_value, TSRMLS_C);
			bmp->surface = screen;
			bmp->w = screen->w;
			bmp->h = screen->h;
			bmp->cx = bmp->cy = bmp->x = bmp->y = 0;
		}
	} else {
		RETURN_FALSE;
	}

	glInitExsts();
}

// Screen::frame()
PHP_METHOD_ARGS(Screen, frame) ZEND_END_ARG_INFO()
PHP_METHOD(Screen, frame)
{
	SDL_Event event;
	static unsigned int ticks = 0;

	memset(keys_pressed, 0, sizeof(keys_pressed));

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				exit(0);
			break;
			case SDL_KEYDOWN:
				keys_pressed[event.key.keysym.sym] = 1;
				keys_status[event.key.keysym.sym] = 1;
			break;
			case SDL_KEYUP:
				keys_pressed[event.key.keysym.sym] = 0;
				keys_status[event.key.keysym.sym] = 0;
			break;
			case SDL_MOUSEMOTION:
			break;
			default:
				//printf("I don't know what this event is! (%d)\n", event.type);
			break;
		}
	}

	while (SDL_GetTicks() < ticks + 1000 / 60) SDL_Delay(1);
	ticks = SDL_GetTicks();

	SDL_GL_SwapBuffers();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	FrameProcess();

	return;
}

// Screen::title(string $title) : string
// Screen::title() : string
PHP_METHOD_ARGS(Screen, title) ARG_INFO(title) ZEND_END_ARG_INFO()
PHP_METHOD(Screen, title)
{
	char *str = NULL; int str_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|s", &str, &str_len) == FAILURE) RETURN_FALSE;

	if (str != NULL) {
		SDL_WM_SetCaption(str, NULL);
	}
	
	SDL_WM_GetCaption(&str, NULL);
	if (str != NULL) {
		RETURN_STRING(str, 1);
	} else {
		RETURN_FALSE;
	}
}
