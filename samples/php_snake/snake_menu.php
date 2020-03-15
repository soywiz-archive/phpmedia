<?php
	function snake_menu($menu) {		
		$m = $menu;
		global $screen;	
		global $menu;
		global $item;
		global $config;
		global $font;
		
		$menu = $m;
		$pos = array(); $ne = sizeof($menu); $inc = (2 * pi()) / $ne;
		
		$b_bg = bitmap_string_load(zip_load('snake_data.dat#menu/title.gif'), FILE_GIF);	
		$pi2 = pi() * 2;
		
		// Make positions of menu
		$n = 0;
		$menutmp = array();
		foreach($menu as $name => $value) {
			$angle = $inc * $n - (pi() / 2);
			$pos[$name] = array(cos($angle), 0, sin($angle));
			draw_clear($menutmp[$name] = bitmap_create(bitmap_width($menu[$name][0]), bitmap_height($menu[$name][0])), color_transparent);
			$n++;
		}
		
		$xmid = bitmap_width($screen) / 2; $ymid = bitmap_height($screen) / 2;
		
		$d = 4;
		$ppu = 40;
		$angle = 0;
		
		$transmin = 30;
		$transmax = 255;
		
		$item = 0;
		$keyleft = false; $keyright = false; $keyenter = false;	
		//while(!key_pressed(KEY_ESC)) {				
		
		$sound = sound_load('1.mp3'); $sound_c = 100;
		
		$str_snd_click = zip_load('snake_data.dat#effects/click.wav') or die('No se pudo cargar el efecto de sonido "click.wav"');
		$snd_click = sound_string_load($str_snd_click);
		
		
		while(true) {
			if (!sound_is_playing($sound_c)) $sound_c = sound_play($sound);
			// Obtiene datos de teclado
			if (key_pressed(KEY_LEFT) && !$keyleft) { $keyleft = true; sound_play($snd_click); $item = modm(0, $ne - 1, $item - 1); }
			if (key_pressed(KEY_RIGHT) && !$keyright) { $keyright = true; sound_play($snd_click); $item = modm(0, $ne - 1, $item + 1); }
			if (!key_pressed(KEY_LEFT) && $keyleft) $keyleft = false;
			if (!key_pressed(KEY_RIGHT) && $keyright) $keyright = false;
						
			$angleto = mod(0, pi() * 2, ($item * $inc));
						
			if (key_pressed(KEY_ENTER) && !$keyenter) {
				sound_play($snd_click);
				$keyenter = true;
				$func = $menu[$item][1];
				
				$background = bitmap_duplicate($screen);
				sound_pause($sound_c, true);
				
				if (!$func()) {				
					$temp = bitmap_duplicate($screen);

					sound_volume($sound_c, 0);
					sound_pause($sound_c, false);
					for ($n = 0; $n < 255; $n+=20) {
						sound_volume($sound_c, max(0, min($n, 255)));
						$_screen = next_vpage(); wait(80);
						draw_bitmap($temp, $_screen, 0, 0);
						draw_bitmap($background, $_screen, 0, 0, $n);	
					}
					
					unset($temp, $background);
				} else {
					sound_pause($sound_c, false);
				}
				$keyenter = false;
			}
			if (!key_pressed(KEY_ENTER) && $keyenter) $keyenter = false;
			
			if (abs($angle - $angleto) < 0.2) {
				$angle = $angleto;
			} else {							
				switch(menu_shordir(0, pi() * 2, $angle, $angleto)) {
					case -1: $angle = mod(0, pi() * 2, $angle - 0.2); break;
					case 1:  $angle = mod(0, pi() * 2, $angle + 0.2); break;
				}
			}			
		
			// Cambia de página
			$_screen = next_vpage(); wait(40);
			
			// Dibujar fondo			
			draw_bitmap($b_bg, $_screen, 0, 0);				
						
			// Recalcular puntos			
			$points = menu_rotateXZ($pos, -$angle);
			uasort($points, 'menu_point_sort');

			// Dibujar el menu						
			foreach($points as $name => $value) {
				list($x, $y, $z) = $value;
				
				$z+=2;
				$x1 = (($x * ($d / $z)) * $ppu); $y1 = (($y * ($d / $z)) * $ppu);								
				
				//draw_pixel($_screen, 160 + $x1, 120 + $y1, color_black);
								
				$bmp = &$menu[$name][0]; $bmpt = &$menutmp[$name];
				$z = 100 - ($z - 1) * 50;				 $size = 50 + ($z / 2);
				// 0-100								
				
				//draw_clear($bmpt, color_black);
				draw_clear($bmpt, color_transparent);
				draw_bitmap($bmp, $bmpt, 0, 0, 255, 0, $size);
				draw_bitmap($bmpt, $_screen, ($xmid + $x1) - (bitmap_width($bmp) * $size) / 200, ($ymid - $y1) - (bitmap_height($bmp) * $size) / 200, $transmin + ((($transmax - $transmin) * $z) / 100));
			}
			
			// Dibuja el título de la selección actual
			
			$txt_s = $menu[$item][2];
			
			font_size($font, 13);			
			$area = font_get_box($font, $txt_s);						
			$txt_m = (bitmap_width($screen) / 2) - (($area['x2'] - $area['x1']) / 2);
			
			font_write_aa($font, $_screen, $txt_m, 10, $txt_s, color_white);
			font_write_aa($font, $_screen, $txt_m - 1, 11, $txt_s, color_white);
			font_write_aa($font, $_screen, $txt_m + 1, 11, $txt_s, color_white);
			font_write_aa($font, $_screen, $txt_m, 12, $txt_s, color_white);
			font_write_aa($font, $_screen, $txt_m, 11, $txt_s, color_black);
		}
	}
	
	function menu_rotateXZ($points, $angle) {		
		$sp = sizeof($points);
		
		$cosb = cos($angle); $sinb = sin($angle);
		
		foreach($points as $n => $v) {
			list($x, $y, $z) = $v;
			
			$x1 = $x * $cosb - $z * $sinb;
			$z1 = $z * $cosb + $x * $sinb;			
						
			$points[$n] = array($x1, $y, $z1);
		}
		
		return $points;
	}
	
	function menu_point_sort($a1, $b1) {
		$a = &$a1[2]; $b = &$b1[2];
		return (($a == $b) ? 0 : (($a > $b) ? -1 : 1));
	}
	
	function menu_shordir($min, $max, $from, $to) {
		if ($from == $to) return 0;
		
		$from = mod($min, $max, $from);
		$to = mod($min, $max, $to);
		
		if ($from < $to) {			
			return abs($from - $to) < (abs($from - $min) + abs($max - $to)) ? 1 : -1;
		} else {
			return abs($from - $to) < (abs($max - $from) + abs($to - $min)) ? -1 : 1;
		}
	}
	
	function mod($min, $max, $num) {
		if ($num < $min) { return $max - ($min - $num); } 		
		if ($num > $max) { return $min + ($num - $max); }
		
		return $num;
	}
	
	function modm($min, $max, $num) {
		if ($num < $min) { return $max - ($min - $num) + 1; } 		
		if ($num > $max) { return $min + ($num - $max) - 1; }
		
		return $num;
	}	
?>