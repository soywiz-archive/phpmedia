static void Bitmap__ObjectDelete(void *object, TSRMLS_D)
{
	BitmapStruct *bitmap = (BitmapStruct *)object;
	{
		if (bitmap->surface)
		{
			if (bitmap->surface->refcount <= 1)
			{
				glDeleteTextures(1, &bitmap->gltex);
			}
			SDL_FreeSurface(bitmap->surface);
		}
		zend_object_std_dtor(&bitmap->std, TSRMLS_C);
		//printf("free\n");
	}
	efree(object);
}

static zend_object_value Bitmap__ObjectNew_ex(zend_class_entry *class_type, BitmapStruct **ptr, TSRMLS_D)
{
	BitmapStruct *intern;
	zend_object_value retval;
	zval *tmp;

	//printf("Bitmap__ObjectNew\n");

	STRUCT_CREATE(BitmapStruct, intern);
	if (ptr != NULL) *ptr = intern;
	
	zend_object_std_init(&intern->std, class_type, TSRMLS_C);
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
	
	retval.handle = zend_objects_store_put(
		intern,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t) Bitmap__ObjectDelete,
		NULL,
		TSRMLS_C
	);

	retval.handlers = &Bitmap_Handlers;
	
	return retval;
}

static zend_object_value Bitmap__ObjectNew(zend_class_entry *class_type, TSRMLS_D)
{
	return Bitmap__ObjectNew_ex(class_type, NULL, TSRMLS_C);
}

static zend_object_value Bitmap__ObjectClone(zval *this_ptr, TSRMLS_D)
{
	BitmapStruct *new_obj = NULL;
	BitmapStruct *old_obj = (BitmapStruct *)zend_object_store_get_object(this_ptr, TSRMLS_C);

	zend_object_value new_ov = Bitmap__ObjectNew_ex(old_obj->std.ce, &new_obj, TSRMLS_C);
	
	zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr), TSRMLS_C);

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
	new_obj->surface->refcount++;
	
	return new_ov;
}


// Bitmap::__construct($w, $h)
PHP_METHOD_ARGS(Bitmap, __construct) ARG_INFO(w) ARG_INFO(h) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, __construct)
{
	int w, h;
	THIS_BITMAP;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ll", &w, &h) == FAILURE) RETURN_FALSE;

	bitmap->surface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
	if (bitmap->surface) {
		bitmap->x = 0;
		bitmap->y = 0;
		bitmap->w = w;
		bitmap->h = h;
		bitmap->cx = 0;
		bitmap->cy = 0;
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

	switch (key_l) {
		case 2:
			if (strcmp(key, "cx") == 0) RETURN_LONG((bitmap) ? bitmap->cx = v : 0);
			if (strcmp(key, "cy") == 0) RETURN_LONG((bitmap) ? bitmap->cy = v : 0);
		break;
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

	switch (key_l) {
		case 1:
			if (strcmp(key, "w") == 0) RETURN_LONG((bitmap && bitmap->surface) ? bitmap->w : 0);
			if (strcmp(key, "h") == 0) RETURN_LONG((bitmap && bitmap->surface) ? bitmap->h : 0);
		break;
		case 2:
			if (strcmp(key, "cx") == 0) RETURN_LONG((bitmap) ? bitmap->cx : 0);
			if (strcmp(key, "cy") == 0) RETURN_LONG((bitmap) ? bitmap->cy : 0);
		break;
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

	ObjectInit(Bitmap_ClassEntry, return_value, TSRMLS_C);
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
			object = ObjectInit(Bitmap_ClassEntry, NULL, TSRMLS_C);
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
			new_bitmap->surface->refcount++;
			add_next_index_zval(return_value, object);
		}
	}
}

// Bitmap::fromFile($name)
PHP_METHOD_ARGS(Bitmap, fromFile) ARG_INFO(name) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, fromFile)
{
	char *name; int name_len;
	SDL_Surface *surface;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &name, &name_len) == FAILURE) RETURN_FALSE;

	if (surface = IMG_Load(name)) {
		BitmapStruct *bitmap;
		ObjectInit(Bitmap_ClassEntry, return_value, TSRMLS_C);
		bitmap = zend_object_store_get_object(return_value, TSRMLS_C);
		bitmap->surface = surface;
		bitmap->x = bitmap->y = bitmap->cx = bitmap->cy = 0;
		bitmap->w = surface->w;
		bitmap->h = surface->h;
		BitmapPrepare(bitmap);
	} else {
		zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0, TSRMLS_C, "Can't load image '%s'", name);
		RETURN_FALSE;
	}
}

// Bitmap::fromString($data)
PHP_METHOD_ARGS(Bitmap, fromString) ARG_INFO(name) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, fromString)
{
	char *data; int data_len;
	SDL_Surface *surface;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &data, &data_len) == FAILURE) RETURN_FALSE;

	if (surface = IMG_Load_RW(SDL_RWFromConstMem(data, data_len), 1)) {
		BitmapStruct *bitmap;
		ObjectInit(Bitmap_ClassEntry, return_value, TSRMLS_C);
		bitmap = zend_object_store_get_object(return_value, TSRMLS_C);
		bitmap->surface = surface;
		bitmap->x = bitmap->y = bitmap->cx = bitmap->cy = 0;
		bitmap->w = surface->w;
		bitmap->h = surface->h;
		BitmapPrepare(bitmap);
	} else {
		zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0, TSRMLS_C, "Can't load image from string");
		RETURN_FALSE;
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

// Bitmap::blit($bmp, $x = 0, $y = 0, $size = 1, $rotation = 0)
PHP_METHOD_ARGS(Bitmap, blit) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, blit)
{
	zval *object;
	double x = 0, y = 0;
	double size = 1, rotation = 0, alpha = 1;
	BitmapStruct *source;
	double w, h, cx, cy;
	double tx[2], ty[2];
	THIS_BITMAP;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "O|ddddd", &object, Bitmap_ClassEntry, &x, &y, &size, &rotation, &alpha) == FAILURE) RETURN_FALSE;

	source = (BitmapStruct *)zend_object_store_get_object(object, TSRMLS_C);
	//printf("%d\n", source->gltex);
	BitmapPrepare(source);
	
	BitmapPrepareDraw(bitmap);

	glLoadIdentity();
	glTranslated(x, y, 0);
	glRotated(rotation, 0, 0, 1);
	//glScaled((double)source->surface->w, (double)source->surface->h, 1);
	glScaled(size, size, 1);

	glBindTexture(GL_TEXTURE_2D, source->gltex);
	glTexParameterf(GL_TEXTURE_2D, 0x84FF, 16);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glColor4d(1, 1, 1, alpha);
	w = source->w;
	h = source->h;
	cx = source->cx;
	cy = source->cy;
	tx[0] = (double)(source->x + 0) / (double)source->surface->w;
	tx[1] = (double)(source->x + w) / (double)source->surface->w;
	ty[0] = (double)(source->y + 0) / (double)source->surface->h;
	ty[1] = (double)(source->y + h) / (double)source->surface->h;
	
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
		glTexCoord2d(tx[0], ty[0]); glVertex2d(0 - cx, 0 - cy);
		glTexCoord2d(tx[1], ty[0]); glVertex2d(w - cx, 0 - cy);
		glTexCoord2d(tx[1], ty[1]); glVertex2d(w - cx, h - cy);
		glTexCoord2d(tx[0], ty[1]); glVertex2d(0 - cx, h - cy);
	glEnd();
	
	//SDL_BlitSurface(source->surface, NULL, bitmap->surface, NULL);

	RETURN_TRUE;
}
