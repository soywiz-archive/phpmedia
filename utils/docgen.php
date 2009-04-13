<?php
	define('ROOT', str_replace('\\', '/', dirname(dirname(__FILE__))));
	
	$path_php_api  = ROOT . '/trunk/api.php';
	$path_wiki     = ROOT . '/wiki';
	$path_wiki_toc = $path_wiki . '/API_TOC.wiki';
	
	if (!is_readable($path_wiki)) {
		die("'" . ($path_wiki) . "' doesn't exists.");
	}

	if (!is_readable($path_php_api)) {
		die("'" . ($path_php_api) . "' doesn't exists.");
	}

	error_reporting(E_ALL | E_STRICT);
	$classes = get_declared_classes();
	require_once(ROOT . '/trunk/api.php');
	$classes = array_diff(get_declared_classes(), $classes);
	
	function mangle($path) {
		$c = explode('::', $path);
		@list($class, $component) = $c;
		switch (sizeof($c)) {
			case 1:
				return 'API_' . strlen($class) . $class;
			break;
			case 2:
				return 'API_' . strlen($class) . $class . strlen($component) . $component;
			break;
		}
	}
	
	function icon($name) {
		return "[http://phpmedia.googlecode.com/svn/www/icons/{$name}.png]";
	}
	
	if (!($ftoc = fopen($path_wiki_toc, 'wb'))) {
		die("Error\n");
	}
	foreach ($classes as $class_name) {
		$class = new ReflectionClass($class_name);
		//$doc = $class->getDocComment();
		fprintf($ftoc, "  * %s[%s %s]\n", icon('class'), mangle($class->getName()), $class->getName());

		foreach ($class->getMethods() as $method) {
			$icon = 'method';
			if ($method->isStatic()) $icon .= '_static';
			
			fprintf($ftoc, "    * %s[%s %s]\n", icon($icon), mangle($class->getName() . '::' . $method->getName()), $method->getName());
		}
		
		foreach ($class->getProperties() as $property) {
			$icon = 'field';
			fprintf($ftoc, "    * %s[%s %s]\n", icon($icon), mangle($class->getName() . '::' . $property->getName()), $property->getName());
		}
		
		foreach ($class->getConstants() as $constant) {
			$icon = 'constant';
			fprintf($ftoc, "    * %s[%s %s]\n", icon($icon), mangle($class->getName() . '::' . $constant->getName()), $constant->getName());
		}
	}
	fclose($ftoc);
?>