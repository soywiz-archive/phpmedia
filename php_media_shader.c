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