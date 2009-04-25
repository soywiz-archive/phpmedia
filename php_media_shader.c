static void Shader__ObjectDelete(ShaderStruct *shader, TSRMLS_D)
{
	glDeleteProgram(shader->program);
	glDeleteShader(shader->shader_fragment);
	glDeleteShader(shader->shader_vertex);
	zend_object_std_dtor(&shader->std, TSRMLS_C);
	efree(shader);
}

static zend_object_value Shader__ObjectNew_ex(zend_class_entry *class_type, ShaderStruct **ptr, TSRMLS_D)
{
	ShaderStruct *intern;
	zend_object_value retval;
	zval *tmp;

	STRUCT_CREATE(ShaderStruct, intern);
	if (ptr != NULL) *ptr = intern;
	
	zend_object_std_init(&intern->std, class_type, TSRMLS_C);
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
	
	retval.handle = zend_objects_store_put(
		intern,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t) Shader__ObjectDelete,
		NULL,
		TSRMLS_C
	);

	retval.handlers = &Shader_Handlers;
	
	return retval;
}

static zend_object_value Shader__ObjectNew(zend_class_entry *class_type, TSRMLS_D)
{
	return Shader__ObjectNew_ex(class_type, NULL, TSRMLS_C);
}

PHP_METHOD_ARGS(Shader, __construct) ARG_INFO(str) ZEND_END_ARG_INFO()
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
		if (0) {
			glGetShaderInfoLog(shader->shader_vertex, sizeof(gl_error), &r, gl_error);
			if (strlen(gl_error)) {
				//THROWF("Can't load vertex shader ('%s')", gl_error);
				//zend_error(E_WARNING, "Vertex Shader: '%s'", gl_error);
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
				//zend_error(E_WARNING, "Fragment Shader: '%s'", gl_error);
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
			zend_error(E_WARNING, "Shader Program: '%s'", gl_error);
		}
	}
}