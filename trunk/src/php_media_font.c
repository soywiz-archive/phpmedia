void glyph_free(FontGlyphCache *g) {
	if (g == NULL) return;
	if (g->surface != NULL) {
		if (g->surface->refcount <= 1) {
			glDeleteTextures(1, &g->gltex);
			if (g->list) glDeleteLists(g->list, 1);
		}
		SDL_FreeSurface(g->surface);
	}
	g->ch = 0;
	g->used = 0;
	g->gltex = 0;
	g->list = 0;
	g->surface = NULL;
}

PM_OBJECTDELETE(Font)
{
	int n;

	if (object->font) {
		for (n = 0; n < GLYPH_MAX_CACHE; n++) glyph_free(&object->glyphs[n]);
		TTF_CloseFont(object->font);
	}

	PM_OBJECTDELETE_STD;
}

PM_OBJECTCLONE_IMPL(Font) {
	CLONE_COPY_FIELD(font);
}

PM_OBJECTNEW(Font);
PM_OBJECTCLONE(Font);

void FontCheckInit() { if (!TTF_WasInit()) TTF_Init(); }

// Font::fromFile($file, $size = 16, $index = 0)
PHP_METHOD_ARGS(Font, fromFile) ARG_INFO(file) ARG_INFO(size) ARG_INFO(index) ZEND_END_ARG_INFO()
PHP_METHOD(Font, fromFile)
{
	char *name = NULL; int name_len = 0; int size = 16, index = 0;
	SDL_RWops *rw;
	FontStruct *object;
	FontCheckInit(); if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s|ll", &name, &name_len, &size, &index) == FAILURE) RETURN_FALSE;

	ObjectInit(EG(called_scope), return_value, TSRMLS_C); // Late Static Binding
	object = zend_object_store_get_object(return_value, TSRMLS_C);	
	object->font = NULL;

	if (rw = SDL_RWFromFile(name, "r")) object->font = TTF_OpenFontIndexRW(rw, 1, size, index);

	if (object->font == NULL) {
		THROWF("Can't load from from file('%s') with size(%d)", name, size);
	}
}

// Font::fromString($data, $size = 16, $index = 0)
PHP_METHOD_ARGS(Font, fromString) ARG_INFO(data) ARG_INFO(size) ARG_INFO(index) ZEND_END_ARG_INFO()
PHP_METHOD(Font, fromString)
{
}

// Font::width($text = '')
PHP_METHOD_ARGS(Font, width) ARG_INFO(text) ZEND_END_ARG_INFO()
PHP_METHOD(Font, width)
{
	char *text = NULL; int text_len = 0;
	THIS_FONT;
	FontCheckInit(); if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|s", &text, &text_len) == FAILURE) RETURN_FALSE;
}

// Font::__get($key)
PHP_METHOD_ARGS(Font, __get) ARG_INFO(key) ZEND_END_ARG_INFO()
PHP_METHOD(Font, __get)
{
	char *key; int key_l;
	THIS_FONT;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &key, &key_l) == FAILURE) RETURN_FALSE;

	if (strcmp(key, "fixed"    ) == 0) RETURN_BOOL(TTF_FontFaceIsFixedWidth(font->font));
	if (strcmp(key, "style"    ) == 0) RETURN_LONG(TTF_GetFontStyle(font->font));
	if (strcmp(key, "height"   ) == 0) RETURN_LONG(TTF_FontHeight(font->font));
	if (strcmp(key, "ascent"   ) == 0) RETURN_LONG(TTF_FontAscent(font->font));
	if (strcmp(key, "descent"  ) == 0) RETURN_LONG(TTF_FontDescent(font->font));
	if (strcmp(key, "lineSkip" ) == 0) RETURN_LONG(TTF_FontLineSkip(font->font));
	if (strcmp(key, "faceName" ) == 0) RETURN_STRING(TTF_FontFaceFamilyName(font->font), 1);
	if (strcmp(key, "styleName") == 0) RETURN_STRING(TTF_FontFaceStyleName(font->font), 1);
	
	RETURN_FALSE;
}

// Font::__set($key, $value)
PHP_METHOD_ARGS(Font, __set) ARG_INFO(key) ARG_INFO(value) ZEND_END_ARG_INFO()
PHP_METHOD(Font, __set)
{
	char *key; int key_l; int v;
	THIS_FONT;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "sl", &key, &key_l, &v) == FAILURE) RETURN_FALSE;

	if (strcmp(key, "style"   ) == 0) {
		TTF_SetFontStyle(font->font, v);
		RETURN_LONG(TTF_GetFontStyle(font->font));
	}
}

// Font::metrics($char)
PHP_METHOD_ARGS(Font, metrics) ARG_INFO(key) ZEND_END_ARG_INFO()
PHP_METHOD(Font, metrics)
{
	char *key; int key_l;
	zval *array;
	THIS_FONT;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &key, &key_l) == FAILURE) RETURN_FALSE;
	{
		int minx, miny;
		int maxx, maxy;
		int advance;
		Uint16 ch = utf8_decode(key, NULL);
		TTF_GlyphMetrics(font->font, ch, &minx, &maxx, &miny, &maxy, &advance);
		
		MAKE_STD_ZVAL(array);
		array_init(array);

		add_assoc_long(array, "minx", minx);
		add_assoc_long(array, "miny", miny);
		add_assoc_long(array, "maxx", maxx);
		add_assoc_long(array, "maxy", maxy);
		add_assoc_long(array, "width", maxx - minx);
		add_assoc_long(array, "height", maxy - miny);
		add_assoc_long(array, "advance", advance);
		
		RETURN_ZVAL(array, 0, 1);
	}
}

// Font::height($text = '')
PHP_METHOD_ARGS(Font, height) ARG_INFO(text) ZEND_END_ARG_INFO()
PHP_METHOD(Font, height)
{
	char *text = NULL; int text_len = 0, lines = 1, n;
	THIS_FONT;
	FontCheckInit(); if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "|s", &text, &text_len) == FAILURE) RETURN_FALSE;
	for (n = 0; n < text_len; n++) {
		if (text[n] == '\n') lines++;
	}
	RETURN_LONG(TTF_FontHeight(font->font) * lines);
}

/*
typedef struct {
	Uint16 ch;
	Uint16 used;
	GLuint gltex;
	SDL_Surface *surface;
} FontGlyphCache;

typedef struct {
	zend_object std;
	TTF_Font *font;
	FontGlyphCache[0x100] glyphs;
} FontStruct;
*/

// void qsort(void *base, size_t nmemb, size_t tamanyo, int (*comparar)(const void *, const void *));

int sort_glyph_usage(const void *_a, const void *_b) {
	FontGlyphCache *a = (FontGlyphCache *)_a, *b = (FontGlyphCache *)_b;
	if (a->ch == 0) return 1;
	if (b->ch == 0) return -1;
	return (int)(a->used) - (int)(b->used);
}

FontGlyphCache *glyph_get(FontStruct *font, Uint16 ch) {
	int n = 0;
	int w, h, x, y;
	SDL_Surface *surfaceogl = NULL;
	FontGlyphCache *g = NULL;
	SDL_Color color = {0xFF, 0xFF, 0xFF, 0xFF};
	qsort(font->glyphs, GLYPH_MAX_CACHE, sizeof(FontGlyphCache), sort_glyph_usage);
	
	// Try to locate an already cached glyph
	for (n = 0; n < GLYPH_MAX_CACHE; n++) {
		g = &font->glyphs[n];
		if (g->ch == ch) {
			g->used++;
			return g;
		}
		if (g->ch == 0) break;
	}
	glyph_free(g);
	g->ch = 0;
	g->used = 1;
	g->surface = TTF_RenderGlyph_Blended(font->font, ch, color);
	g->list = glGenLists(1);
	surfaceogl = __SDL_ConvertSurfaceForOpenGL(g->surface, 0);
	{
		glGenTextures(1, &g->gltex);
		glBindTexture(GL_TEXTURE_2D, g->gltex);

		glTexImage2D(GL_TEXTURE_2D, 0, 4, surfaceogl->w, surfaceogl->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surfaceogl->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glEnable(GL_CLAMP_TO_EDGE);
		glEnable(GL_TEXTURE_2D);
	}
	SDL_FreeSurface(surfaceogl);

	w = g->surface->w;
	h = g->surface->h;
	x = 0;
	y = 0;

	glNewList(g->list, GL_COMPILE);
	{
		int minx, miny;
		int maxx, maxy;
		int advance;
		TTF_GlyphMetrics(font->font, ch, &minx, &maxx, &miny, &maxy, &advance);
		x = minx;
		y = TTF_FontAscent(font->font) - maxy;
		
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScaled(1.0 / (double)w, 1.0 / (double)h, 1.0);

		glMatrixMode(GL_MODELVIEW);

		glBindTexture(GL_TEXTURE_2D, g->gltex);
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
			if (1) {
				glTexCoord2f((float)0, (float)0); glVertex2i(x + 0, y + 0);
				glTexCoord2f((float)w, (float)0); glVertex2i(x + w, y + 0);
				glTexCoord2f((float)w, (float)h); glVertex2i(x + w, y + h);
				glTexCoord2f((float)0, (float)h); glVertex2i(x + 0, y + h);
			} else {
				glTexCoord2f(0, 0); glVertex2i(x + 0, y + 0);
				glTexCoord2f(1, 0); glVertex2i(x + w, y + 0);
				glTexCoord2f(1, 1); glVertex2i(x + w, y + h);
				glTexCoord2f(0, 1); glVertex2i(x + 0, y + h);
			}
		glEnd();
		glTranslatef((float)advance, 0.0, 0.0);
	}
	glEndList();

	return g;
}

// Font::blit(Bitmap $dest, $text, $x, $y, $color)
PHP_METHOD_ARGS(Font, blit) ARG_INFO(text) ZEND_END_ARG_INFO()
PHP_METHOD(Font, blit)
{
	zval *object_bitmap = NULL;
	zval *color_array = NULL;
	char *str = NULL; int str_len = 0;
	int ptr_pos = 0; int ptr_inc = 0;
	double x = 0.0, y = 0.0;
	double color[4] = {1, 1, 1, 1};
	FontGlyphCache *g;
	Uint16 ch = 0;
	THIS_FONT;
	FontCheckInit(); if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "Os|dda", &object_bitmap, ClassEntry_Bitmap, &str, &str_len, &x, &y, &color_array) == FAILURE) RETURN_FALSE;
	
	if (color_array) {
		int n = 0;
		HashTable *ht = Z_ARRVAL_P(color_array);
	
		for (n = 0; n < 4; n++) {
			zval **v = NULL;
			if (0 == zend_hash_index_find(ht, n, (void **)&v)) { convert_to_double(*v); color[n] = Z_DVAL_PP(v); }
		}
	}

	glPushAttrib(GL_CURRENT_BIT);
	{
		glColor4dv(color);

		glLoadIdentity();
		glTranslated(x, y, 0.0);
		while (ptr_pos < str_len) {
			ch = utf8_decode(&str[ptr_pos], &ptr_inc); ptr_pos += ptr_inc;
			g = glyph_get(font, ch);
			glCallList(g->list);
		}
	}
	glPopAttrib();

	//glMatrixMode(GL_TEXTURE); glPopMatrix();
}
