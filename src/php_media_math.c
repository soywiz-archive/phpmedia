// Math::clamp($min, $max, &$input)
PHP_METHOD_ARGS(Math, clamp) ARG_INFO(min) ARG_INFO(max) ARG_INFO(input) ZEND_END_ARG_INFO()
PHP_METHOD(Math, clamp)
{
	zval **input, *min, *max;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "zzZ", &min, &max, &input) == FAILURE) RETURN_FALSE;
	switch (Z_TYPE(**input)) {
		case IS_DOUBLE: {
			double min, max;
			if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ddZ", &min, &max, &input) == FAILURE) RETURN_FALSE;
			if (Z_DVAL(**input) < min) Z_DVAL(**input) = min;
			if (Z_DVAL(**input) > max) Z_DVAL(**input) = max;
		} break;
		case IS_LONG: {
			int min, max;
			if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "llZ", &min, &max, &input) == FAILURE) RETURN_FALSE;
			if (Z_LVAL(**input) < min) Z_LVAL(**input) = min;
			if (Z_LVAL(**input) > max) Z_LVAL(**input) = max;
		} break;
		default: {
			int min, max;
			if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "llZ", &min, &max, &input) == FAILURE) RETURN_FALSE;
			ZVAL_LONG(*input, min);
		} break;
	}
}

// Math::smoothstep($min, $max, $input)
PHP_METHOD_ARGS(Math, smoothstep) ARG_INFO(min) ARG_INFO(max) ARG_INFO(input) ZEND_END_ARG_INFO()
PHP_METHOD(Math, smoothstep)
{
	double min = 0.0, max = 1.0, input = 0.0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ddd", &min, &max, &input) == FAILURE) RETURN_FALSE;
	if (input <= min) {
		RETURN_DOUBLE(0.0);
	} else if (input >= max) {
		RETURN_DOUBLE(1.0);
	} else {
		RETURN_DOUBLE((input - min) / (max - min));
	}
}

// Math::sign($input)
PHP_METHOD_ARGS(Math, sign) ARG_INFO(min) ARG_INFO(max) ARG_INFO(input) ZEND_END_ARG_INFO()
PHP_METHOD(Math, sign)
{
	double value = 0.0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "d", &value) == FAILURE) RETURN_FALSE;
	if (value < 0.0) {
		RETURN_LONG(-1);
	} else if (value > 0.0) {
		RETURN_LONG(+1);
	} else {
		RETURN_LONG(0);
	}
}

// Math::swap(&$a, &$b)
PHP_METHOD_ARGS(Math, swap) ARG_INFO(a) ARG_INFO(b) ZEND_END_ARG_INFO()
PHP_METHOD(Math, swap)
{
	zval **a, **b, temp;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "ZZ", &a, &b) == FAILURE) RETURN_FALSE;
	temp = **a;
	**a = **b;
	**b = temp;
}
