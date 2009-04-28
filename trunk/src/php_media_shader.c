PM_OBJECTDELETE(Shader)
{
	glDeleteProgram(object->program);
	glDeleteShader(object->shader_fragment);
	glDeleteShader(object->shader_vertex);
	
	PM_OBJECTDELETE_STD;
}

PM_OBJECTCLONE_IMPL(Shader) {
	//CLONE_COPY_FIELD(chunk);
}

PM_OBJECTNEW(Shader);
PM_OBJECTCLONE(Shader);

typedef struct {
	zval *array;
	ShaderStruct *shader;
} ShaderEntry;

#define MAX_SHADER_STACK 128

ShaderEntry shader_stack[MAX_SHADER_STACK]; int shader_stack_pos = 0;
BitmapStruct *shader_bitmaps[16]; int shader_bitmaps_count = 1;


void shader_set(ShaderStruct *shader, zval *shader_params, TSRMLS_D) {
	if (glUseProgram == NULL) return;

	if (shader) {
		HashPosition pos;
		zval **element;
		char *key; int key_len;
		GLuint uni_id;
		GLenum uni_type;
		int n;
		char temp[256]; int temp_len, uni_len;
		double values_d[16];
		float  values_f[16];
		int    values_i[16];
		int    values_count = 0;
		shader_bitmaps_count = 1;

		glUseProgram(shader->program);

		if (shader_params) {
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

				glGetActiveUniform(shader->program, uni_id, sizeof(temp), &temp_len, &uni_len, &uni_type, temp);
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
					case IS_OBJECT:
						values_count = 1;
					break;
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
						BitmapStruct *bitmap;
						
						if ((Z_TYPE(**element) == IS_OBJECT) && instanceof_function(Z_OBJCE(**element), ClassEntry_Bitmap, TSRMLS_C)) {
							bitmap = (BitmapStruct *)zend_object_store_get_object(*element, TSRMLS_C);
							
							shader_bitmaps[shader_bitmaps_count++] = bitmap;

							glActiveTexture(GL_TEXTURE0 + shader_bitmaps_count);
							glBindTexture(GL_TEXTURE_2D, bitmap->gltex);
							glUniform1iv(uni_id, 1, &shader_bitmaps_count);

							//printf("%d: %d\n", uni_id, tex_count);
							glEnable(GL_TEXTURE_2D);
							
							{
								int filter = bitmap->smooth ? GL_LINEAR : GL_NEAREST;
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
							}
						} else {
							zend_error(E_WARNING, "Uniform '%s' can only process Bitmap objects", key);
						}							
					} break;
				}

				//glVertexAttrib1f(aid, v0);
			}
		}
	}
	// No Shader
	else {
		int n;
		glUseProgram(0);
		for (n = 0; n < 8; n++) {
			glActiveTexture(GL_TEXTURE1 + n);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}
		glActiveTexture(GL_TEXTURE0);
	}
}

void shader_begin(ShaderStruct *shader, zval *array, TSRMLS_D) {
	if (shader_stack_pos >= MAX_SHADER_STACK) {
		THROWF("Shader stack limit!! Maybe any Shader::begin without Shader::end?");
	}
	shader_stack[shader_stack_pos].shader = shader;
	shader_stack[shader_stack_pos].array = array;
	shader_stack_pos++;
	shader_set(shader, array, TSRMLS_C);
}

void shader_end(TSRMLS_D) {
	if (--shader_stack_pos > 0) {
		shader_set(shader_stack[shader_stack_pos].shader, shader_stack[shader_stack_pos].array, TSRMLS_C);
	} else {
		shader_set(NULL, NULL, TSRMLS_C);
	}
}

// Shader::__construct($shader_fragment, $shader_vertex)
PHP_METHOD_ARGS(Shader, __construct) ARG_INFO(shader_fragment) ARG_INFO(shader_vertex) ZEND_END_ARG_INFO()
PHP_METHOD(Shader, __construct)
{
	int str_shader_vertex_len = 0, str_shader_fragment_len = 0;
	//char *str_shader_vertex   = "void main(void) { gl_TexCoord[0] = gl_MultiTexCoord0; gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; }";
	//char *str_shader_fragment = "void main (void) { gl_FragColor = vec4 (0.0, 1.0, 0.0, 1.0); }";
	char *str_shader_vertex   = NULL;
	char *str_shader_fragment = NULL;
	int r;
	THIS_SHADER;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s|s", &str_shader_fragment, &str_shader_fragment_len, &str_shader_vertex, &str_shader_vertex_len) == FAILURE) RETURN_FALSE;

	shader->program = glCreateProgram();
	//printf("PROGRAM: %d\n", shader->program);

	//void main(void) { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; }

	if (str_shader_vertex && str_shader_vertex_len) {
		shader->shader_vertex   = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(shader->shader_vertex, 1, &str_shader_vertex, &str_shader_vertex_len);
		glCompileShader(shader->shader_vertex);
		glGetShaderiv(shader->shader_vertex, GL_INFO_LOG_LENGTH, &r);
		if (r) {
			glGetShaderInfoLog(shader->shader_vertex, sizeof(gl_error), &r, gl_error);
			if (strlen(gl_error)) {
				//THROWF("Can't load vertex shader ('%s')", gl_error);
				zend_error(E_WARNING, "Vertex Shader: '%s'", gl_error);
			}
		}
		glAttachShader(shader->program, shader->shader_vertex);
	}

	if (str_shader_fragment && str_shader_fragment_len) {
		shader->shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(shader->shader_fragment, 1, &str_shader_fragment, &str_shader_fragment_len);
		glCompileShader(shader->shader_fragment);
		glGetShaderiv(shader->shader_fragment, GL_INFO_LOG_LENGTH, &r);
		if (r) {
			glGetShaderInfoLog(shader->shader_fragment, sizeof(gl_error), &r, gl_error);
			if (strlen(gl_error)) {
				//THROWF("Can't load fragment shader ('%s')", gl_error);
				zend_error(E_WARNING, "Fragment Shader: '%s'", gl_error);
			}
		}
		glAttachShader(shader->program, shader->shader_fragment);
	}
	
	glLinkProgram(shader->program);
	glGetShaderiv(shader->program, GL_INFO_LOG_LENGTH, &r);
	if (r) {
		glGetShaderInfoLog(shader->program, sizeof(gl_error), &r, gl_error);
		if (strlen(gl_error)) {
			//THROWF("Can't link program ('%s')", gl_error);
			//zend_error(E_WARNING, "Shader Program: '%s'", gl_error);
		}
	}
}

// Shader::begin($params)
PHP_METHOD_ARGS(Shader, begin) ARG_INFO(params) ZEND_END_ARG_INFO()
PHP_METHOD(Shader, begin)
{
	zval *shader_params = NULL;
	THIS_SHADER;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "a", &shader_params) == FAILURE) RETURN_FALSE;
	shader_begin(shader, shader_params, TSRMLS_C);
}

// Shader::end()
PHP_METHOD_ARGS(Shader, end) ARG_INFO(params) ZEND_END_ARG_INFO()
PHP_METHOD(Shader, end)
{
	THIS_SHADER;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "") == FAILURE) RETURN_FALSE;
	shader_end(TSRMLS_C);
}

// Shader::__get()
PHP_METHOD_ARGS(Shader, __get) ARG_INFO(params) ZEND_END_ARG_INFO()
PHP_METHOD(Shader, __get)
{
	zval *array;
	char temp[256]; int temp_len = 0;
	char *key; int key_l;
	int n, count;
	int uni_type; int uni_len;
	THIS_SHADER;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "s", &key, &key_l) == FAILURE) RETURN_FALSE;

	if (strcmp(key, "params") == 0) {
		MAKE_STD_ZVAL(array);
		array_init(array);
		
		glGetProgramiv(shader->program, GL_ACTIVE_UNIFORMS, &count);
		for (n = 0; n < count; n++) {
			glGetActiveUniform(shader->program, n, sizeof(temp), &temp_len, &uni_len, &uni_type, temp);
			add_assoc_long(array, temp, 0);
		}

		RETURN_ZVAL(array, 0, 1);
	}
	
	RETURN_FALSE;
}