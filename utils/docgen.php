<?php
	define('ROOT', str_replace('\\', '/', dirname(dirname(__FILE__))));
	
	$path_php_api  = ROOT . '/trunk/api.php';
	$path_wiki     = ROOT . '/wiki';
	
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
				$component = str_replace('$', '', $component);
				return 'API_' . strlen($class) . $class . strlen($component) . $component;
			break;
		}
	}
	
	function mangle_link($path, $long = false) {
		if ($long) {
			return '[' . mangle($path) . ' ' . $path . ']';
		} else {
			$a = explode('::', $path);
			return '[' . mangle($path) . ' ' . array_pop($a) . ']';
		}
	}
	
	function icon($name) {
		return "[http://phpmedia.googlecode.com/svn/www/icons/{$name}.png]";
	}
	
	function ParseDoc($doc) {
		$lines = array();
		foreach (preg_split("/[\n\r]+/", $doc) as $line) {
			$line = ltrim($line, " \t*/");
			if (strlen($line)) $lines[] = $line;
		}
		return implode("\n", $lines);
	}
	
	if (!($ftoc = fopen("{$path_wiki}/API_TOC.wiki", 'wb'))) {
		die("Error\n");
	}
	foreach ($classes as $class_name) {
		$class = new ReflectionClass($class_name);
		//$doc = $class->getDocComment();
		fprintf($ftoc, "  * %s[%s %s]\n", icon('class'), mangle($class->getName()), $class->getName());
		
		$fclass = fopen("{$path_wiki}/" . mangle($class->getName()) . ".wiki", 'wb');
		fprintf($fclass, "== class %s ==\n", $class->getName());
		fprintf($fclass, "%s\n\n", ParseDoc($class->getDocComment()));
		fprintf($fclass, "<font face=\"Lucida Console\">\n");

		foreach ($class->getMethods() as $method) {
			$icon = 'method';
			if ($method->isStatic()) $icon .= '_static';
			
			$doc = $method->getDocComment();
			$return = 'void';
			if (preg_match('/@return\\s+(\\w+)/si', $doc, $matches)) {
				$return = class_exists($matches[1]) ? mangle_link($matches[1]) : $matches[1];
			}
			$params = array();
			foreach ($method->getParameters() as $parameter) {
				$param = '';
				$cclass = $parameter->getClass();
				if ($cclass) {
					$param .= mangle_link($cclass->getName()) . ' ';
				}
				if ($parameter->isPassedByReference()) $param .= '&';
				$param .= '$' . $parameter->getName();
				if ($parameter->isDefaultValueAvailable()) {
					$param .= '<font color="#999"> = ' . str_replace("\n", '', var_export($parameter->getDefaultValue(), true) . '</font>');
				}
				$params[] = $param;
			}
			
			//print_r($method->getParameters());
			
			fprintf($ftoc, "    * %s%s\n", icon($icon), mangle_link($class->getName() . '::' . $method->getName(), false));
			fprintf($fclass, "  * %s%s (%s) : <font color=\"#a88\">%s</font>\n", icon($icon), mangle_link($class->getName() . '::' . $method->getName(), false), implode(', ', $params), $return);
		}
		
		foreach ($class->getProperties() as $property) {
			$icon = 'field';
			fprintf($ftoc, "    * %s%s\n", icon($icon), mangle_link($class->getName() . '::$' . $property->getName(), false));
			fprintf($fclass, "  * %s%s\n", icon($icon), mangle_link($class->getName() . '::$' . $property->getName(), false));
		}
		
		foreach ($class->getConstants() as $constant => $constant_value) {
			$icon = 'constant';
			fprintf($ftoc, "    * %s%s\n", icon($icon), mangle_link($class->getName() . '::' . $constant));
			fprintf($fclass, "  * %s%s = <font color=\"#a88\">%s</font>\n", icon($icon), mangle_link($class->getName() . '::' . $constant), $constant_value);
		}
		
		fprintf($fclass, "</font>\n");
	}
	fclose($ftoc);
?>