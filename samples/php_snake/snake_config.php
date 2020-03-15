<?php
	function snake_videoconfig() {
		global $item, $menu, $config;
		if ($config['fullscreen']) {
			$menu[$item][0] = bitmap_string_load(zip_load('snake_data.dat#menu/menu_windowed.gif'), FILE_GIF);
			$config['fullscreen'] = false;			
		} else {
			$menu[$item][0] = bitmap_string_load(zip_load('snake_data.dat#menu/menu_fullscreen.gif'), FILE_GIF);
			$config['fullscreen'] = true;									
		}
		config_save(); exit;
	}
?>