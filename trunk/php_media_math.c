// Math::clamp(&$var, $min, $max)
PHP_METHOD_ARGS(Math, clamp) ARG_INFO(var) ARG_INFO(min) ARG_INFO(max) ZEND_END_ARG_INFO()
PHP_METHOD(Math, clamp)
{
	zval **var, *min, *max;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "Zzz", &var, &min, &max) == FAILURE) RETURN_FALSE;
	switch (Z_TYPE(**var)) {
		case IS_DOUBLE: {
			double min, max;
			if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "Zdd", &var, &min, &max) == FAILURE) RETURN_FALSE;
			if (Z_DVAL(**var) < min) Z_DVAL(**var) = min;
			if (Z_DVAL(**var) > max) Z_DVAL(**var) = max;
		} break;
		case IS_LONG: {
			int min, max;
			if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "Zll", &var, &min, &max) == FAILURE) RETURN_FALSE;
			if (Z_LVAL(**var) < min) Z_LVAL(**var) = min;
			if (Z_LVAL(**var) > max) Z_LVAL(**var) = max;
		} break;
		default: {
			int min, max;
			if (zend_parse_parameters(ZEND_NUM_ARGS(), TSRMLS_C, "Zll", &var, &min, &max) == FAILURE) RETURN_FALSE;
			ZVAL_LONG(*var, min);
		} break;
	}
}