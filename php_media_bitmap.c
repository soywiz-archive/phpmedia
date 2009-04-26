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

	switch (key_l) {
		case 2:
			if (strcmp(key, "cx") == 0) RETURN_LONG((bitmap) ? bitmap->cx = v : 0);
			if (strcmp(key, "cy") == 0) RETURN_LONG((bitmap) ? bitmap->cy = v : 0);
		break;
		case 6:
			if (strcmp(key, "smooth") == 0) RETURN_LONG((bitmap) ? bitmap->smooth = v : 0);
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
		case 6:
			if (strcmp(key, "smooth") == 0) RETURN_LONG((bitmap) ? bitmap->smooth : 0);
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
	ShaderStruct *shader;
	BitmapStruct *sources[12];
	double w, h, cx, cy;
	double tx[12][2], ty[12][2];
	int tex_count = 0, n;
	THIS_BITMAP;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "O|dddddOa", &object_bitmap, ClassEntry_Bitmap, &x, &y, &size, &rotation, &alpha, &object_shader, ClassEntry_Shader, &shader_params) == FAILURE) RETURN_FALSE;
	
	source = bitmap;
	dest = (BitmapStruct *)zend_object_store_get_object(object_bitmap, TSRMLS_C);
	
	BitmapPrepare(source);
	BitmapPrepareDraw(dest);
	
	sources[tex_count++] = source;

	if (source == dest) {
		THROWF("Can't draw a bitmap on itself.");
		RETURN_FALSE;
	}
	
	if (source->surface == screen) {
		THROWF("Can't blit the screen on a bitmap.");
		RETURN_FALSE;		
	}
	
	if (glUseProgram) {
		if (object_shader) {
			shader = (ShaderStruct *)zend_object_store_get_object(object_shader, TSRMLS_C);
			glUseProgram(shader->program);

			if (shader_params) {
				HashPosition pos;
				zval **element;
				char *key; int key_len;
				GLuint uni_id;
				GLenum uni_type;
				int dummy, n;
				double values_d[16];
				float  values_f[16];
				int    values_i[16];
				int    values_count = 0;
				
				for (n = 0; n < 16; n++) {
					values_d[n] = 0.0;
					values_f[n] = 0.0;
					values_i[n] = 0;
				}

				for (
					zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(shader_params), &pos);
					zend_hash_get_current_data_ex(Z_ARRVAL_P(shader_params), (void **) &element, &pos) == SUCCESS;
					zend_hash_move_forward_ex(Z_ARRVAL_P(shader_params), &pos)
				) {
					zend_hash_get_current_key_ex(Z_ARRVAL_P(shader_params), &key, &key_len, NULL, 0, &pos);
					uni_id = glGetUniformLocation(shader->program, key);
					
					if (uni_id == -1) {
						zend_error(E_WARNING, "Can't find uniform with name '%s'", key);
						continue;
					}

					glGetActiveUniform(shader->program, uni_id, 0, NULL, &dummy, &uni_type, NULL);
					//printf("%s: %d\n", key, uni_type);
					
					switch (Z_TYPE(**element)) {
						case IS_ARRAY: {
							zval **cvalue;
							int n;
							values_count = zend_hash_num_elements(HASH_OF(*element));
							for (n = 0; n < values_count; n++) {
								zend_hash_index_find(HASH_OF(*element), n, (void **)&cvalue);
								convert_to_double(&**cvalue);
								values_d[n] = (double)Z_DVAL(**cvalue);
								values_f[n] = (float )Z_DVAL(**cvalue);
								values_i[n] = (int   )Z_DVAL(**cvalue);								
							}
						}break;
						default:
							convert_to_double(*element);
							values_d[0] = (double)Z_DVAL(**element);
							values_f[0] = (float )Z_DVAL(**element);
							values_i[0] = (int   )Z_DVAL(**element);
							values_count = 1;
						break;
					}
					
					#define uni_check_length(EXPECTED) if (values_count != (EXPECTED)) { zend_error(E_WARNING, "Uniform '%s' expects %d values, obtained %d", key, (int)EXPECTED, (int)values_count); }
					
					#define GL_INT_VEC1 GL_INT
					#define GL_BOOL_VEC1 GL_BOOL
					#define GL_FLOAT_VEC1 GL_FLOAT
					#define UNIFORM_F(N) { case GL_FLOAT_VEC##N: glUniform##N##fv(uni_id, 1, values_f); uni_check_length(N); break; }
					#define UNIFORM_I(N) { case GL_INT_VEC##N: case GL_BOOL_VEC##N: glUniform##N##iv(uni_id, 1, values_i); uni_check_length(N); break; }
					
					switch (uni_type) {
						UNIFORM_F(1); UNIFORM_F(2); UNIFORM_F(3); UNIFORM_F(4); // FLOAT
						UNIFORM_I(1); UNIFORM_I(2); UNIFORM_I(3); UNIFORM_I(4); // INT/BOOL
						// MATRIX
						case GL_FLOAT_MAT2: case GL_FLOAT_MAT3: case GL_FLOAT_MAT4: case GL_FLOAT_MAT2x3: case GL_FLOAT_MAT2x4: case GL_FLOAT_MAT3x2: case GL_FLOAT_MAT3x4: case GL_FLOAT_MAT4x2: case GL_FLOAT_MAT4x3:
							zend_error(E_WARNING, "Matrix uniforms not implemented");
						break;
						case GL_SAMPLER_1D: case GL_SAMPLER_2D: case GL_SAMPLER_3D: case GL_SAMPLER_CUBE: case GL_SAMPLER_1D_SHADOW: case GL_SAMPLER_2D_SHADOW: {
							BitmapStruct *bitmap2;
							
							if ((Z_TYPE(**element) == IS_OBJECT) && instanceof_function(Z_OBJCE(**element), ClassEntry_Bitmap, TSRMLS_C)) {
								bitmap2 = (BitmapStruct *)zend_object_store_get_object(*element, TSRMLS_C);
								glActiveTexture(GL_TEXTURE0 + tex_count);
								glBindTexture(GL_TEXTURE_2D, bitmap2->gltex);
								glUniform1iv(uni_id, 1, &tex_count);
								sources[tex_count] = bitmap2;
								//printf("%d: %d\n", uni_id, tex_count);
								glEnable(GL_TEXTURE_2D);
								
								if (bitmap2->smooth) {
									glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
									glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								} else {
									glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
									glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
								}

								tex_count++;
							} else {
								zend_error(E_WARNING, "Only can process Bitmap objects");
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
	
	if (source->smooth) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	

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
	
	RETURN_TRUE;
}
