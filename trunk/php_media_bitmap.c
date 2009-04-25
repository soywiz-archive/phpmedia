static void Bitmap__ObjectDelete(BitmapStruct *bitmap, TSRMLS_D)
{
	if (bitmap->surface) {
		if (bitmap->surface->refcount <= 1) glDeleteTextures(1, &bitmap->gltex);
		SDL_FreeSurface(bitmap->surface);
	}
	zend_object_std_dtor(&bitmap->std, TSRMLS_C);
	efree(bitmap);
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
		bitmap = zend_object_store_get_object(return_value, TSRMLS_C);bitmap = zend_object_store_get_object(return_value, TSRMLS_C);
		bitmap->surface = surface;
		bitmap->x = bitmap->y = bitmap->cx = bitmap->cy = 0;
		bitmap->w = surface->w;
		bitmap->h = surface->h;
		BitmapPrepare(bitmap);
	} else {
		THROWF("Can't load image '%s'", name);
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
		THROWF("Can't load image from string");
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

// Bitmap::blit($bmp, $x = 0, $y = 0, $size = 1, $rotation = 0, $alpha = 1,Shader $shader = null)
PHP_METHOD_ARGS(Bitmap, blit) ZEND_END_ARG_INFO()
PHP_METHOD(Bitmap, blit)
{
	zval *object_bitmap = NULL, *object_shader = NULL, *shader_params = NULL;
	double x = 0, y = 0;
	double size = 1, rotation = 0, alpha = 1;
	BitmapStruct *source;
	ShaderStruct *shader;
	BitmapStruct *sources[12];
	double w, h, cx, cy;
	double tx[12][2], ty[12][2];
	int tex_count = 0, n;
	THIS_BITMAP;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "O|dddddOa", &object_bitmap, Bitmap_ClassEntry, &x, &y, &size, &rotation, &alpha, &object_shader, Shader_ClassEntry, &shader_params) == FAILURE) RETURN_FALSE;
	
	if (glUseProgram) {
		if (object_shader) {
			shader = (ShaderStruct *)zend_object_store_get_object(object_shader, TSRMLS_C);
			glUseProgram(shader->program);

			if (shader_params) {
				HashPosition pos;
				zval **element;
				char *key; int key_len;
				GLuint uni_id;

				for (
					zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(shader_params), &pos);
					zend_hash_get_current_data_ex(Z_ARRVAL_P(shader_params), (void **) &element, &pos) == SUCCESS;
					zend_hash_move_forward_ex(Z_ARRVAL_P(shader_params), &pos)
				) {
					zend_hash_get_current_key_ex(Z_ARRVAL_P(shader_params), &key, &key_len, NULL, 0, &pos);
					uni_id = glGetUniformLocation(shader->program, key);

					switch (Z_TYPE(**element)) {
						case IS_DOUBLE: case IS_LONG:
							convert_to_double(*element);
							glUniform1f(uni_id, (float)Z_DVAL(**element));
						break;
						// A bitmap?
						case IS_OBJECT: {
							BitmapStruct *bitmap2;
							
							if (instanceof_function(Z_OBJCE(**element), Bitmap_ClassEntry, TSRMLS_C)) {
								bitmap2 = (BitmapStruct *)zend_object_store_get_object(*element, TSRMLS_C);
								glActiveTexture(GL_TEXTURE1 + tex_count++);
								glBindTexture(GL_TEXTURE_2D, bitmap2->gltex);
								glUniform1i(uni_id, tex_count);
								sources[tex_count] = bitmap2;
								//printf("%d: %d\n", uni_id, tex_count);
								glEnable(GL_TEXTURE_2D);
							} else {
								zend_error(E_WARNING, "Only can process Bitmap objects");
							}
						} break;
						case IS_ARRAY : {
							zval **cvalue[16];
							int n, count = zend_hash_num_elements(HASH_OF(*element));
							for (n = 0; n < count; n++) {
								zend_hash_index_find(HASH_OF(*element), n, (void **)&cvalue[n]);
								convert_to_double(&**cvalue[n]);
							}
							switch (count) {
								case 1: glUniform1f(uni_id, (float)Z_DVAL(**cvalue[0])); break;
								case 2: glUniform2f(uni_id, (float)Z_DVAL(**cvalue[0]), (float)Z_DVAL(**cvalue[1])); break;
								case 3: glUniform3f(uni_id, (float)Z_DVAL(**cvalue[0]), (float)Z_DVAL(**cvalue[1]), (float)Z_DVAL(**cvalue[2])); break;
								case 4: glUniform4f(uni_id, (float)Z_DVAL(**cvalue[0]), (float)Z_DVAL(**cvalue[1]), (float)Z_DVAL(**cvalue[2]), (float)Z_DVAL(**cvalue[3])); break;
							}
						} break;
					}
					//glVertexAttrib1f(aid, v0);
				}			
				//printf("TEST %d\n", zend_hash_num_elements(HASH_OF(shader_params)));
				//GLuint aid = glGetUniformLocation(id, std.string.toStringz(name));
				//glVertexAttrib1f(aid, v0);
			}
			
			//glVertexAttrib1d(glGetUniformLocation(shader->program, "mycol"), (GLfloat)0.3);
		} else {
			glUseProgram(0);
			for (n = 0; n < 8; n++) {
				glActiveTexture(GL_TEXTURE1 + n);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
			}
		}
	}
	
	tex_count++;
	
	sources[0] = source = (BitmapStruct *)zend_object_store_get_object(object_bitmap, TSRMLS_C);
	//printf("%d\n", source->gltex);
	BitmapPrepare(source);
	
	BitmapPrepareDraw(bitmap);

	glLoadIdentity();
	glTranslated(x, y, 0);
	glRotated(rotation, 0, 0, 1);
	//glScaled((double)source->surface->w, (double)source->surface->h, 1);
	glScaled(size, size, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, source->gltex);
	glTexParameterf(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, 16);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glColor4d(1, 1, 1, alpha);

	for (n = 0; n < tex_count; n++) {
		tx[n][0] = (double)(sources[n]->x + 0            ) / (double)sources[n]->surface->w;
		tx[n][1] = (double)(sources[n]->x + sources[n]->w) / (double)sources[n]->surface->w;
		ty[n][0] = (double)(sources[n]->y + 0            ) / (double)sources[n]->surface->h;
		ty[n][1] = (double)(sources[n]->y + sources[n]->h) / (double)sources[n]->surface->h;

		tx[n][0] *= (double)sources[0]->w / (double)sources[n]->w;
		tx[n][1] *= (double)sources[0]->w / (double)sources[n]->w;
		ty[n][0] *= (double)sources[0]->h / (double)sources[n]->h;
		ty[n][1] *= (double)sources[0]->h / (double)sources[n]->h;
		
		//printf("(%f, %f)-(%f, %f)\n", (float)tx[n][0], (float)ty[n][0], (float)tx[n][1], (float)ty[n][1]);
	}
	
	w = source->w;
	h = source->h;
	cx = source->cx;
	cy = source->cy;
	
	#define QUAD_POINT(X, Y) { \
		for (n = 0; n < tex_count; n++) glMultiTexCoord2d(GL_TEXTURE0 + n, tx[n][X], ty[n][Y]); \
		glVertex2d((X * w) - cx, (Y * h) - cy); \
	}
		
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
		QUAD_POINT(0, 0);
		QUAD_POINT(1, 0);
		QUAD_POINT(1, 1);
		QUAD_POINT(0, 1);
	glEnd();
	
	//SDL_BlitSurface(source->surface, NULL, bitmap->surface, NULL);

	RETURN_TRUE;
}
