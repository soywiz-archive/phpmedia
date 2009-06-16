PM_OBJECTDELETE(Bitmap)
{
	if (object->surface) {
		if (object->surface->refcount <= 1) glDeleteTextures(1, &object->gltex);
		SDL_FreeSurface(object->surface);
	}
	PM_OBJECTDELETE_STD;
}

PM_OBJECTCLONE_IMPL(Bitmap) {
	//CLONE_COPY_FIELD(surface->w);
	//CLONE_COPY_FIELD(surface->h);
	CLONE_COPY_FIELD(parent);
	CLONE_COPY_FIELD(surface);
	CLONE_COPY_FIELD(gltex);
	CLONE_COPY_FIELD(cx);
	CLONE_COPY_FIELD(cy);
	CLONE_COPY_FIELD(x);
	CLONE_COPY_FIELD(y);
	CLONE_COPY_FIELD(w);
	CLONE_COPY_FIELD(h);
	CLONE_COPY_FIELD(smooth);
	new_obj->surface->refcount++;
}

PM_OBJECTNEW(Bitmap);
PM_OBJECTCLONE(Bitmap);

// Bitmap::__construct($w, $h)
PHP_METHOD_ARGS(Bitmap, __construct) ARG_INFO(w) ARG_INFO(h) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, __construct)
{
	int w, h;
	THIS_BITMAP;

	sdl_load(TSRMLS_C);

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ll", &w, &h) == FAILURE) RETURN_FALSE;

	bitmap->surface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
	if (bitmap->surface) {
		bitmap->x = 0;
		bitmap->y = 0;
		bitmap->w = w;
		bitmap->h = h;
		bitmap->cx = 0;
		bitmap->cy = 0;
		bitmap->smooth = 1;
		BitmapPrepare(bitmap);
	}
}

// Bitmap::__set($key, $value)
PHP_METHOD_ARGS(Bitmap, __set) ARG_INFO(key) ARG_INFO(value) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, __set)
{
	char *key; int key_l; int v;
	THIS_BITMAP;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "sl", &key, &key_l, &v) == FAILURE) RETURN_FALSE;
	
	if (bitmap) {
		switch (key_l) {
			case 2:
				if (strcmp(key, "cx") == 0) RETURN_LONG(bitmap->cx = v);
				if (strcmp(key, "cy") == 0) RETURN_LONG(bitmap->cy = v);
			break;
			case 6:
				if (strcmp(key, "smooth") == 0) RETURN_LONG(bitmap->smooth = v);
			break;
		}
	}

	RETURN_FALSE;
}

// Bitmap::__get($key)
PHP_METHOD_ARGS(Bitmap, __get) ARG_INFO(key) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, __get)
{
	char *key; int key_l;
	THIS_BITMAP;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &key, &key_l) == FAILURE) RETURN_FALSE;

	if (bitmap) {
		switch (key_l) {
			case 1:
				if (strcmp(key, "w") == 0) RETURN_LONG(bitmap->w);
				if (strcmp(key, "h") == 0) RETURN_LONG(bitmap->h);
			break;
			case 2:
				if (strcmp(key, "cx") == 0) RETURN_LONG(bitmap->cx);
				if (strcmp(key, "cy") == 0) RETURN_LONG(bitmap->cy);
			break;
			case 6:
				if (strcmp(key, "smooth") == 0) RETURN_LONG(bitmap->smooth);
			break;
		}
	}

	RETURN_FALSE;
}

// Bitmap::slice($x, $y, $w, $h)
PHP_METHOD_ARGS(Bitmap, slice) ARG_INFO(x) ARG_INFO(y) ARG_INFO(w) ARG_INFO(h) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, slice)
{
	int x = 0, y = 0, w = 0, h = 0;
	BitmapStruct *new_bitmap;
	THIS_BITMAP;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "llll", &x, &y, &w, &h) == FAILURE) RETURN_FALSE;
	
	clamp(&x, 0, bitmap->w);
	clamp(&y, 0, bitmap->h);
	clamp(&w, 0, bitmap->w - x);
	clamp(&h, 0, bitmap->h - y);

	//ObjectInit(ClassEntry_Bitmap, return_value, TSRMLS_C);
	ObjectInit(EG(called_scope), return_value, TSRMLS_C);
	new_bitmap = zend_object_store_get_object(return_value, TSRMLS_C);
	new_bitmap->parent = bitmap;
	new_bitmap->surface = bitmap->surface;
	new_bitmap->gltex = bitmap->gltex;
	new_bitmap->x = x + bitmap->x;
	new_bitmap->y = y + bitmap->y;
	new_bitmap->cx = 0;
	new_bitmap->cy = 0;
	new_bitmap->w = w;
	new_bitmap->h = h;
	new_bitmap->smooth = bitmap->smooth;
	new_bitmap->surface->refcount++;
}

// Bitmap::split($w, $h)
PHP_METHOD_ARGS(Bitmap, split) ARG_INFO(w) ARG_INFO(h) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, split)
{
	int w = 0, h = 0;
	int x, y;
	zval *object;
	BitmapStruct *new_bitmap;
	THIS_BITMAP;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ll", &w, &h) == FAILURE) RETURN_FALSE;
	
	array_init(return_value);
	
	for (y = 0; y < bitmap->h; y += h) {
		for (x = 0; x < bitmap->w; x += w) {
			object = ObjectInit(ClassEntry_Bitmap, NULL, TSRMLS_C);
			new_bitmap = zend_object_store_get_object(object, TSRMLS_C);
			new_bitmap->parent = bitmap;
			new_bitmap->surface = bitmap->surface;
			new_bitmap->gltex = bitmap->gltex;
			new_bitmap->x = x + bitmap->x;
			new_bitmap->y = y + bitmap->y;
			new_bitmap->cx = 0;
			new_bitmap->cy = 0;
			new_bitmap->w = w;
			new_bitmap->h = h;
			new_bitmap->smooth = bitmap->smooth;
			new_bitmap->surface->refcount++;
			add_next_index_zval(return_value, object);
		}
	}
}

int Bitmap_fromRW(zval **return_value, SDL_RWops *rw, TSRMLS_D)
{
	SDL_Surface *surface;
	BitmapStruct *bitmap;

	sdl_load(TSRMLS_C);

	if ((surface = IMG_Load_RW(rw, 1)) == NULL) return 0;
	
	//ObjectInit(ClassEntry_Bitmap, *return_value, TSRMLS_C);
	ObjectInit(EG(called_scope), *return_value, TSRMLS_C);
	bitmap = zend_object_store_get_object(*return_value, TSRMLS_C);

	bitmap->surface = surface;
	bitmap->x = bitmap->y = bitmap->cx = bitmap->cy = 0;
	bitmap->w = surface->w;
	bitmap->h = surface->h;
	bitmap->smooth = 1;

	BitmapPrepare(bitmap);

	return 1;
}

// Bitmap::fromFile($name)
PHP_METHOD_ARGS(Bitmap, fromFile) ARG_INFO(name) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, fromFile)
{
	char *name;
	int name_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &name, &name_len) == FAILURE) RETURN_FALSE;

	if (!Bitmap_fromRW(&return_value, SDL_RWFromFile(name, "r"), TSRMLS_C)) THROWF("Can't load image from file: '%s'", name);
}

// Bitmap::fromString($data)
PHP_METHOD_ARGS(Bitmap, fromString) ARG_INFO(name) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, fromString)
{
	char *data;
	int data_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &data, &data_len) == FAILURE) RETURN_FALSE;

	if (!Bitmap_fromRW(&return_value, SDL_RWFromConstMem(data, data_len), TSRMLS_C)) THROWF("Can't load image from string");
}

// Bitmap::saveFile($name, $format = 0)
PHP_METHOD_ARGS(Bitmap, saveFile) ARG_INFO(name) ARG_INFO(format) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, saveFile)
{
	char *name = NULL; int name_len = 0;
	int format = 0;
	SDL_Surface *temp = NULL;
	int flip = 0;
	THIS_BITMAP;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s|l", &name, &name_len, &format) == FAILURE) RETURN_FALSE;
	

	if (bitmap->surface == screen) {
		temp = SDL_CreateRGBSurface(0, bitmap->surface->w, bitmap->surface->h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
		SDL_LockSurface(temp);
		{
			glReadPixels(0, 0, screen->w, screen->h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, temp->pixels);
		}
		SDL_UnlockSurface(temp);
		flip = 1;
	} else {
		int rw = 0, rh = 0;
		glBindTexture(GL_TEXTURE_2D, bitmap->gltex);

		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH , &rw);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &rh);

		temp = SDL_CreateRGBSurface(0, rw, rh, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
		
		SDL_LockSurface(temp);
		{
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, temp->pixels);
		}
		SDL_UnlockSurface(temp);
	}

	if (temp) {
		if (flip) {
			SDL_LockSurface(temp);
			{
				int n, width = temp->pitch, height = temp->h, height2 = temp->h >> 1;
				char *row = malloc(width);
				if (row) {
					for (n = 0; n < height2; n++) {
						char *row1 = (char *)temp->pixels + (n) * width;
						char *row2 = (char *)temp->pixels + (height - n - 1) * width;
						memcpy(row, row1, width);
						memcpy(row1, row2, width);
						memcpy(row2, row, width);
					}
					free(row);
				}
			}
			SDL_UnlockSurface(temp);
		}
		switch (format) {
			case 0: // BMP
				SDL_SaveBMP(temp, name);
				SDL_FreeSurface(temp);
			break;
			case 1: // TGA
			case 2: // PNG
			case 3: // TGA
				SDL_FreeSurface(temp);
				THROWF("Not supported format for Bitmap::saveFile");
			break;
			default:
				SDL_FreeSurface(temp);
				THROWF("Invalid format for Bitmap::saveFile");
			break;
		}
	}
}

// Bitmap::clear($r = 0, $g = 0, $b = 0, $a = 0)
PHP_METHOD_ARGS(Bitmap, clear) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, clear)
{
	double r = 0, g = 0, b = 0, a = 0;
	THIS_BITMAP;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|dddd", &r, &g, &b, &a) == FAILURE) RETURN_FALSE;
	
	BitmapPrepareDraw(bitmap);

	glClearColor((float)r, (float)g, (float)b, (float)a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RETURN_TRUE;
}

// Bitmap::center($x, $y)
// Bitmap::center()
PHP_METHOD_ARGS(Bitmap, center) ARG_INFO(cx) ARG_INFO(cy) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, center)
{
	int cx = 0, cy = 0;
	THIS_BITMAP;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|ll", &cx, &cy) == FAILURE) RETURN_FALSE;
	
	if (ZEND_NUM_ARGS() < 2) {
		bitmap->cx = bitmap->surface->w;
		bitmap->cy = bitmap->surface->h;
	} else {
		bitmap->cx = cx;
		bitmap->cy = cy;
	}
}

// Bitmap::blit($dest, $x = 0, $y = 0, $size = 1, $rotation = 0, $alpha = 1,Shader $shader = null)
PHP_METHOD_ARGS(Bitmap, blit) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, blit)
{
	zval *object_bitmap = NULL, *object_shader = NULL, *shader_params = NULL;
	double x = 0, y = 0;
	double size = 1, rotation = 0, alpha = 1;
	BitmapStruct *source, *dest;
	ShaderStruct *shader = NULL;
	double w, h, cx, cy;
	double tx[16][2], ty[16][2];
	int n;
	THIS_BITMAP;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "O|dddddOa", &object_bitmap, ClassEntry_Bitmap, &x, &y, &size, &rotation, &alpha, &object_shader, ClassEntry_Shader, &shader_params) == FAILURE) RETURN_FALSE;
	
	source = bitmap;
	dest   = (BitmapStruct *)zend_object_store_get_object(object_bitmap, TSRMLS_C);

	if (source->surface == screen) { THROWF("Can't blit the screen on a bitmap."); RETURN_FALSE; }
	
	BitmapPrepare(source);
	BitmapPrepareDraw(dest);
	
	if (glUseProgram && object_shader) {
		shader = (ShaderStruct *)zend_object_store_get_object(object_shader, TSRMLS_C);
		shader_begin(shader, shader_params, TSRMLS_C);
	}

	shader_bitmaps[0] = source;
	
	glLoadIdentity();
	glTranslated(x, y, 0);
	glRotated(rotation, 0, 0, 1);
	glScaled(size, size, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, source->gltex);
	glTexParameterf(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, 16);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glColor4d(1, 1, 1, alpha);
	
	{
		int filter = source->smooth ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	}

	for (n = 0; n < shader_bitmaps_count; n++) {
		/*
		tx[n][0] = (double)(shader_bitmaps[n]->x + 0                   ) / (double)shader_bitmaps[n]->surface->w;
		tx[n][1] = (double)(shader_bitmaps[n]->x + shader_bitmaps[n]->w) / (double)shader_bitmaps[n]->surface->w;
		ty[n][0] = (double)(shader_bitmaps[n]->y + 0                   ) / (double)shader_bitmaps[n]->surface->h;
		ty[n][1] = (double)(shader_bitmaps[n]->y + shader_bitmaps[n]->h) / (double)shader_bitmaps[n]->surface->h;

		tx[n][0] *= (double)shader_bitmaps[0]->w / (double)shader_bitmaps[n]->w;
		tx[n][1] *= (double)shader_bitmaps[0]->w / (double)shader_bitmaps[n]->w;
		ty[n][0] *= (double)shader_bitmaps[0]->h / (double)shader_bitmaps[n]->h;
		ty[n][1] *= (double)shader_bitmaps[0]->h / (double)shader_bitmaps[n]->h;
		*/
		tx[n][0] = (double)(shader_bitmaps[n]->x + 0                   );
		tx[n][1] = (double)(shader_bitmaps[n]->x + shader_bitmaps[n]->w);
		ty[n][0] = (double)(shader_bitmaps[n]->y + 0                   );
		ty[n][1] = (double)(shader_bitmaps[n]->y + shader_bitmaps[n]->h);

		glActiveTexture(GL_TEXTURE0 + n);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		{
			float tex_w, tex_h;
			
			glGetTexLevelParameterfv(GL_TEXTURE_2D, 0,  GL_TEXTURE_WIDTH, &tex_w);
			glGetTexLevelParameterfv(GL_TEXTURE_2D, 0,  GL_TEXTURE_HEIGHT, &tex_h);
			glScaled(1.0 / (double)tex_w, 1.0 / (double)tex_h, 1.0);
		}
	}
	
	glActiveTexture(GL_TEXTURE0);
	
	w  = source->w;
	h  = source->h;
	cx = source->cx;
	cy = source->cy;
	
	#define QUAD_POINT(X, Y) { \
		for (n = 0; n < shader_bitmaps_count; n++) glMultiTexCoord2d(GL_TEXTURE0 + n, tx[n][X], ty[n][Y]); \
		glVertex2d((X * w) - cx, (Y * h) - cy); \
	}

	glMatrixMode(GL_MODELVIEW);
		
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
		QUAD_POINT(0, 0);
		QUAD_POINT(1, 0);
		QUAD_POINT(1, 1);
		QUAD_POINT(0, 1);
	glEnd();

	if (shader) shader_end(TSRMLS_C);	
	
	RETURN_TRUE;
}

PHP_METHOD_ARGS(Draw, line) ARG_INFO(bitmap) ARG_INFO(x1) ARG_INFO(y1) ARG_INFO(x2) ARG_INFO(y2) ARG_INFO(border) ARG_INFO(color) ZEND_END_ARG_INFO()
PHP_METHOD(Draw, line)
{
	zval *object_bitmap = NULL;
	zval *color_array = NULL;
	BitmapStruct *dest = NULL;
	double colorv[4] = {1, 1, 1, 1};
	double x1 = 0, y1 = 0, x2 = 0, y2 = 0, border = 1;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "Odddd|da", &object_bitmap, ClassEntry_Bitmap, &x1, &y1, &x2, &y2, &border, &color_array) == FAILURE) RETURN_FALSE;
	dest = (BitmapStruct *)zend_object_store_get_object(object_bitmap, TSRMLS_C);
	BitmapPrepareDraw(dest);

	extract_color(color_array, colorv);
	
	glLoadIdentity();
	glColor4dv(colorv);
	glLineWidth((GLfloat)border);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
	glEnd();
}