<?php
	require_files(
		'http://usuarios.lycos.es/phpwiz/',
		array(
			'php_media.' . PHP_SHLIB_SUFFIX,
			'php_zip.' . PHP_SHLIB_SUFFIX,
			'alleg41.' . PHP_SHLIB_SUFFIX,
			'alfont.' . PHP_SHLIB_SUFFIX,
			'fmod.' . PHP_SHLIB_SUFFIX,
			'snake_data.dat',
			array('snake_game.php', 'snake_game.dat'),
			array('snake_menu.php', 'snake_menu.dat'),
			array('snake_scores.php', 'snake_scores.dat'),
			array('snake_config.php', 'snake_config.dat'),
			array('0.mp3', '0.dat'),
			array('1.mp3', '1.dat')
		)
	);

	(file_exists('snake_data.dat') or die('No se pudo cargar el fichero snake_data.dat'));		
	(extension_loaded('media') or @dl('php_media.' . PHP_SHLIB_SUFFIX)) or die('No se pudo cargar la librería <PHP Media>');
	(extension_loaded('zip') or @dl('php_zip.' . PHP_SHLIB_SUFFIX)) or die('No se pudo cargar la librería <PHP Zip>');	

	// Selecciona modo de video
	
	config_load();
	
	if (!isset($config['fullscreen'])) $config['fullscreen'] = true;
	
	if ($config['fullscreen']) {
		videomode_set(320, 240, VIDEOMODE_FULLSCREEN);
	} else {
		videomode_set(320, 240, VIDEOMODE_WINDOWED);
	}
	
	
	// Define constantes de color
	
	define('color_transparent', color_make(255, 0, 255));
	define('color_black', color_make(0, 0, 0));
	define('color_white', color_make(255, 255, 255));
	
	// Carga los ficheros de las partes del juego
	
	require_once('snake_menu.php');
	require_once('snake_game.php');
	require_once('snake_scores.php');
	require_once('snake_config.php');
	
	
	// Carga del menú
	
	$fontstr = zip_load('snake_data.dat#nrkis.ttf') or die('No se pudo cargar la fuente "nrkis.ttf"');
	$font = font_string_load($fontstr);
			
	snake_menu(
		array(			
			array(bitmap_string_load(zip_load('snake_data.dat#menu/menu_start.gif'), FILE_GIF), 'snake_game', 'Start 1 player game'),
			array(bitmap_string_load(zip_load('snake_data.dat#menu/menu_exit.gif'), FILE_GIF), 'snake_exit', 'Exit game'),
			array(bitmap_string_load(zip_load('snake_data.dat#menu/menu_scores.gif'), FILE_GIF), 'snake_scores', 'See hight scores'),
			array(bitmap_string_load(zip_load($config['fullscreen'] ? 'snake_data.dat#menu/menu_fullscreen.gif' : 'snake_data.dat#menu/menu_windowed.gif'), FILE_GIF), 'snake_videoconfig', 'Change fullscreen/windowed videomode')
		)
	);	
	
	exitt();
	
	// Funciones comunes

	function wait($fps) { $inter = (1 / $fps); list($start) = explode(' ', microtime()); while(true) { list($mt) = explode(' ', microtime()); if ($mt < $start) $start--; if ($mt >= ($start + $inter)) break; } }
	function start_vpages() { global $vs, $vp, $screen, $vinit; if (!isset($vinit) || $vinit == false) { $vs = array(); draw_clear($vs[0] = bitmap_create_video(bitmap_width($screen), bitmap_height($screen)), 0);	draw_clear($vs[1] = bitmap_create_video(bitmap_width($screen), bitmap_height($screen)), 0); $vp = 0; $vinit = true; } }	
	function next_vpage() { global $vs, $vp, $vinit; if (!isset($vinit) || $vinit = false) { start_vpages(); } videomode_showbitmap($vs[$vp]); return $vs[($vp = $vp ? 0 : 1)]; }
	function file_load($url) { return @implode('', @file($url)); }
	function file_save($url, $string) { if ($fd = @fopen($url, 'wb+')) { @fwrite($fd, $string); @fclose($fd); return true; } return false; }
	function zip_list($file) { $return = array(); if (file_exists($file)) { if ($dir = @zip_open($file)) { while($file = @zip_read($dir)) { $return[] = @zip_entry_name($file); } @zip_close($dir); } } return $return; }
	function zip_exists($file) { if (strpos($file, '#') !== false) { list($zip, $file) = explode('#', $file); $file = str_replace('\\', '/', trim($file)); foreach(zip_list($zip) as $f) { if (strcasecmp(str_replace('\\', '/', trim($f)), $file) == 0) return true; } return false; } else { return file_exists($file); } }
	function zip_load($file) { if (strpos($file, '#') !== false) {  list($zip, $fn) = explode('#', $file); $fn = trim(strtolower($fn)); if ($dir = zip_open($zip)) { while($file = zip_read($dir)) { if (strtolower(trim(str_replace('\\', '/', zip_entry_name($file)))) == $fn) { if (zip_entry_open($dir, $file, 'rb')) { $content = zip_entry_read($file, @zip_entry_filesize($file)); zip_entry_close($file); return $content; } } } zip_close($dir); } return false; } else { return file_load($url); } }

	function dir_make($path) { $path = str_replace('\\', '/', $path); $nx = ''; foreach(explode('/', $path) as $e) { $nx .= $e . '/'; if (!file_exists($nx)) { mkdir($nx, 0777); } } }  
	function getAbsoluteUri($absuri, $uri) { if (strtolower(substr($uri, 0, 7)) == 'http://' || strtolower(substr($uri, 0, 8)) == 'https://' || strtolower(substr($uri, 0, 6)) == 'ftp://' || strtolower(substr($uri, 0, 7)) == 'ftps://') { return $uri; } $absuri = trim(str_replace('\\', '/', $absuri)); $uri = trim(str_replace('\\', '/', $uri));	$abspath = (substr($absuri, -1, 1) == '/') ? substr($absuri, 0, -1) : dirname($absuri); $res = explode('/', substr($uri, 0, strrpos($uri, '/'))); foreach($res as $e) { switch($e) { case '..': $abspath = dirname($abspath); break; case '.': break; default: $abspath .= '/' . $e; break; } } return $abspath . '/' . basename($uri); }


	function array_trim($list) { $return = array(); foreach($list as $n => $e) { if ($e = trim($e)) $return[$n] = $e; } return $return; }

	function require_files($url, $files) {
		echo "Comprobando ficheros...\n";
		foreach($files as $file) {
			if (is_array($file)) {
				list($file_save, $file_load) = $file;				
			} else {
				$file_save = $file_load = $file;
			}
			if (!file_exists($file_save)) {
				echo "Descargando '" . $file_save . "'...\n";				
				if ($data = @file_load(getAbsoluteUri($url, $file_load))) {
					@file_save($file_save, $data);
				} else {
					die("  ERROR: No se pudo descargar '" . $file_save . "'\n");
				}
			}
		}
	}		
	
	function snake_exit() { exit; }
	
	function config_load() { global $config; file_save('snakere.bat', ''); if (file_exists('config.dat')) { $config = unserialize(file_load('config.dat')); } }
	function config_save() { global $config; file_save('snakere.bat', '@call snake.bat'); file_save('config.dat', serialize($config)); }
	
	function strin($start, $end, $string) {
		if ($pos = strpos($string, $start) === false) return false;
		if ($pos2 = strpos($string, $end, $pos + strlen($start)) === false) $pos2 = strlen($string);
		return substr($string, $pos + strlen($start), $pos2 - $pos - strlen($start) - 1);
	}
?>