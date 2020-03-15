<?php
	function snake_game() {
		global $screen;
		
		global $b_tileset;
	
		global $b_tiles;
		global $b_snake_head, $b_snake_body, $b_snake_tail;
		global $b_item;
		global $b_life, $b_nlife;
		
		global $font;
		
		global $map;
		
		load_tileset() or die('No se pudieron cargar los tiles');
		load_level($level = 0) or die('No se pudo cargar el mapa');
		$background = load_board() or die('No se pudo cargar la imagen de fondo');
		$sound = load_sound() or die('No se pudo cargar la música de fondo');
		$sound_c = 100;

				
		$str_snd_click = zip_load('snake_data.dat#effects/click.wav') or die('No se pudo cargar el efecto de sonido "click.wav"');
		$str_snd_death = zip_load('snake_data.dat#effects/death.wav') or die('No se pudo cargar el efecto de sonido "death.wav"');
		$str_snd_level = zip_load('snake_data.dat#effects/level.wav') or die('No se pudo cargar el efecto de sonido "level.wav"');
						
		$snd_click = sound_string_load($str_snd_click);
		$snd_death = sound_string_load($str_snd_death);
		$snd_level = sound_string_load($str_snd_level);

		////////////////////////////////////////////////////////////////////////////////////////

		$snake_head_dir = 3; $snake_body = array();

		$snake_head_x = $map['start_x']; $snake_head_y = $map['start_y'];
		$snake_tail_x = $snake_head_x; $snake_tail_y = $snake_head_y;
		$trans_min = 100; $trans_max = 230;
		$addbody_ = 5; $addbody = $addbody_;

		$itemneeded_ = 4;
		$itempoints = 3;
		$itemneeded = $itemneeded_;
		$points = 0;
		$penalization = 12;
		$lifes = 4;

		$lifepoints = 103;
		$lifecounter = 0;

		$lifestatus = bitmap_create(120, 9);


		do { $item_x = rand(0, $map['width'] - 1); $item_y = rand(0, $map['height'] - 1); } while($map['data'][$item_y * $map['width'] + $item_x] != ' ');

		////////////////////////////////////////////////////////////////////////////////////////

		$pausep = false;
		while(!key_pressed(KEY_ESC)) {				
			$_screen = next_vpage(); wait(18);

			if (!sound_is_playing($sound_c)) { sound_volume($sound_c = sound_play($sound), 255); }

			// IO:  input-move
			if (key_pressed(KEY_UP))    $snake_head_dir = 0;
			if (key_pressed(KEY_LEFT))  $snake_head_dir = 1;
			if (key_pressed(KEY_RIGHT)) $snake_head_dir = 2;
			if (key_pressed(KEY_DOWN))  $snake_head_dir = 3;			

			if (!key_pressed(KEY_P) && !$pausep) $pausep = true;
			if (key_pressed(KEY_P) && $pausep) {
				$pausep = false;
				$temp = bitmap_create(bitmap_width($screen), bitmap_height($screen));
				draw_clear($temp, color_make(0, 0, 0));
				draw_bitmap($temp, $screen, 0, 0, 128);			

				$temp = 0;
				do {
				if (!sound_is_playing($sound_c)) { sound_volume($sound_c = sound_play($sound), 128); }
				if (!key_pressed(KEY_P)) $temp = 1; } while(!key_pressed(KEY_ESC) && !($temp == 1 && key_pressed(KEY_P)));

				continue;
			}

			// DRAW: background
			draw_bitmap($background, $_screen, 0, 0);

			// DRAW: snake-tail		
			$snake_tail_dir = isset($snake_body[0]) ? $snake_body[0] : $snake_head_dir;
			draw_bitmap($b_snake_tail[$snake_tail_dir], $_screen, $snake_tail_x * 8, $snake_tail_y * 8, $trans_min);

			// DRAW: snake-body
			$lastdir = $snake_tail_dir; $ainc = (($ssb = sizeof($snake_body)) == 0) ? 0 : ($trans_max - $trans_min) / sizeof($snake_body); $x = $snake_tail_x; $y = $snake_tail_y;
			$apos = $trans_min;
			for ($n = 0; $n < $ssb; $n++) {						
				$t = gsb($e = &$snake_body[$n], ($n + 1 >= $ssb) ? $snake_head_dir : $snake_body[$n + 1]);

				switch($e) {
					case 0: $y--; break;
					case 1: $x--; break;
					case 2: $x++; break;
					case 3: $y++; break;
				}		

				draw_bitmap($b_snake_body[$t], $_screen, $x * 8, $y * 8, floor($apos += $ainc));
			}

			// MOVE: snake-head
			$map['data'][$snake_head_y * $map['width'] + $snake_head_x] = 'x';
			switch($snake_head_dir) {
				case 0: $snake_head_y--; break;
				case 1: $snake_head_x--; break;
				case 2: $snake_head_x++; break;
				case 3: $snake_head_y++; break;
			}		

			$snake_body[] = $snake_head_dir;

			// CHECK: get_item		
			if ($snake_head_x == $item_x && $snake_head_y == $item_y) {
				do { $item_x = rand(0, $map['width'] - 1); $item_y = rand(0, $map['height'] - 1); } while($map['data'][$item_y * $map['width'] + $item_x] != ' ');
				$addbody = $addbody_; sound_play($snd_click); $points += $itempoints; $lifecounter += $itempoints; $itemneeded--;			
			}

			// CHECK: next_level
			if ($itemneeded <= 0) {
				$temp = $background;

				sound_play($snd_level);
				load_level(++$level) or die('No se pudo cargar el mapa');
				$background = load_board() or die('No se pudo cargar la imagen de fondo');
				$sound2 = load_sound() or die('No se pudo cargar la música de fondo');		
				$itemneeded = $itemneeded_ + ($level * 1);
				$snake_head_dir = 3; $snake_tail_dir = 3; $snake_body = array();

				$snake_head_x = $map['start_x']; $snake_head_y = $map['start_y'];
				$snake_tail_x = $snake_head_x; $snake_tail_y = $snake_head_y;			
				$addbody = $addbody_;

				$sound_c2 = sound_play($sound2);			

				for ($n = 0; $n < 255; $n+=8) {
					$_screen = next_vpage(); wait(80);
					draw_bitmap($temp, $_screen, 0, 0);
					draw_bitmap($background, $_screen, 0, 0, $n);	

					font_size($font, 40);
					font_write($font, $_screen, -1, 99, $map['name'], color_make(0, 0, 0));
					font_write($font, $_screen, -1, 101, $map['name'], color_make(0, 0, 0));
					font_write($font, $_screen, 1, 99, $map['name'], color_make(0, 0, 0));
					font_write($font, $_screen, 1, 101, $map['name'], color_make(0, 0, 0));

					font_write($font, $_screen, 1, 100, $map['name'], color_make(255, rand(0, 64), rand(0, 64)));

					sound_volume($sound_c, 255 - $n);
					sound_volume($sound_c2, $n);
				}

				$sound = $sound2; unset($temp); unset($sound2);
				do { $item_x = rand(0, $map['width'] - 1); $item_y = rand(0, $map['height'] - 1); } while($map['data'][$item_y * $map['width'] + $item_x] != ' ');
				$points += 48; $lifecounter += 48;
				continue;
			}


			// CHECK: collision (death)
			if ($map['data'][$snake_head_y * $map['width'] + $snake_head_x] != ' ') {
				$temp = bitmap_create(bitmap_width($screen), bitmap_height($screen));
				draw_clear($temp, color_make(0, 0, 0)); draw_bitmap($temp, $screen, 0, 0, 128); unset($temp);


				sound_play($snd_death);
				wait(1);
				$snake_head_dir = 3; $snake_tail_dir = 3; $snake_body = array();

				$snake_head_x = $map['start_x']; $snake_head_y = $map['start_y'];
				$snake_tail_x = $snake_head_x; $snake_tail_y = $snake_head_y;
				$addbody = $addbody_;

				$map['data'] = $map['data_back'];
				$itemneeded = $itemneeded_ + ($level * 1);
				$points -= $penalization; $lifecounter -= $penalization; $lifes--;
				continue;
			}

			// CHECK: lifecounter
			if ($lifecounter >= $lifepoints) {
				sound_play($snd_level);
				$lifes++; $lifecounter = 0;
			}				

			// MOVE: snake-tail
			if ($addbody > 0) {			
				$addbody--;
			} else {
				//$snake_tail_dir = array_shift($snake_body);
				array_shift($snake_body);
				$map['data'][$snake_tail_y * $map['width'] + $snake_tail_x] = ' ';
				switch($snake_tail_dir) {
					case 0: $snake_tail_y--; break;
					case 1: $snake_tail_x--; break;
					case 2: $snake_tail_x++; break;
					case 3: $snake_tail_y++; break;
				}			
			}

			// DRAW: item
			draw_bitmap($b_item, $_screen, $item_x * 8 + rand(-3, 3), $item_y * 8 + rand(-3, 3), 50);
			draw_bitmap($b_item, $_screen, $item_x * 8 + rand(-3, 3), $item_y * 8 + rand(-3, 3), 50);
			draw_bitmap($b_item, $_screen, $item_x * 8 + rand(-2, 2), $item_y * 8 + rand(-2, 2), 100);
			draw_bitmap($b_item, $_screen, $item_x * 8 + rand(-2, 2), $item_y * 8 + rand(-2, 2), 100);
			draw_bitmap($b_item, $_screen, $item_x * 8 + rand(-1, 1), $item_y * 8 + rand(-1, 1), 150);
			draw_bitmap($b_item, $_screen, $item_x * 8 + rand(-1, 1), $item_y * 8 + rand(-1, 1), 150);

			draw_bitmap($b_item, $_screen, $item_x * 8, $item_y * 8, 160);

			// DRAW: snake-head
			draw_bitmap($b_snake_head[$snake_head_dir], $_screen, $snake_head_x * 8, $snake_head_y * 8, $trans_max);

			// DRAW: status-bar

			font_size($font, 12);

			font_write_aa($font, $_screen, 100, 0, 'Points: ' . $points, color_make(255, 255, 255));
			font_write_aa($font, $_screen, 170, 0, $map['name'], color_make(255, 255, 255));

			draw_bitmap($b_item, $_screen, 1, 1, 160); font_size($font, 10);
			font_write_aa($font, $_screen, 10, 0, 'x' . $itemneeded, color_make(255, 255, 255));

			draw_clear($lifestatus, color_make(255, 0, 255));
			for ($n = 0; $n < 6; $n++) {
				if ($lifes > $n) {
					draw_bitmap($b_life, $lifestatus, $n * 9, 0, 200);
				} else {
					draw_bitmap($b_nlife, $lifestatus, $n * 9, 0, 200);
				}
			}

			draw_bitmap($lifestatus, $_screen, 30, 1, 255, 0, rand(100, 110));

			if ($lifes == 0) { break; }
		}
		
		return snake_scores($points);
	}	
	
	function load_level($level) {
		global $map;
		
		if (!zip_exists('snake_data.dat#levels/' . (string)$level . '.lvl')) { $level = '0'; }
		if ($dta = @zip_load('snake_data.dat#levels/' . (string)$level . '.lvl')) {
			$_d = array_trim(explode("\n", $dta));
			list($map['name'], $map['image'], $map['song']) = array_splice($_d, 0, 3);
			$map['data'] = implode('', $_d); $map['width'] = 40; $map['height'] = 30;
			$_w = &$map['width']; $_h = &$map['height'];					
			
			$d = &$map['data']; $w = &$map['width']; $h = &$map['height']; $n = 0;
			for ($y = 0; $y < $h; $y++) {
				for ($x = 0; $x < $w; $x++) {
					if (strtolower($d[$n]) == 's') { $map['start_x'] = $x; $map['start_y'] = $y; break 2; }
					$n++;
				}
			}
			
			$map['data_back'] = $map['data'];
			
			return true;
		} else {
			return false;
		}		
	}
	
	function load_sound() { global $map; return sound_load($map['song']); }
	
	function load_board($alpha = 180) {
		global $screen;
		global $b_tiles, $map;
		
		$return = bitmap_create(bitmap_width($screen), bitmap_height($screen));
		draw_clear($return, color_make(0, 0, 0));				
		$temp = bitmap_string_load(zip_load('snake_data.dat#bg/' . $map['image']), pm_filetype(substr(strstr($map['image'], '.'), 1)));		
		draw_bitmap_tiled($temp, $return, 0, 0);
		
		
		$d = &$map['data']; $w = &$map['width']; $h = &$map['height'];
		$n = 0;
		for ($y = 0; $y < $h; $y++) {
			for ($x = 0; $x < $w; $x++) {
				if ($d[$n] != ' ' && $d[$n] != 'S') {
					$l = 0;
					$l |= (($y - 1 < 0)   ? 0 : (($d[$n - $w] != ' ') ? 1 : 0)) << 0;
					$l |= (($x - 1 < 0)   ? 0 : (($d[$n - 1]  != ' ') ? 1 : 0)) << 1;
					$l |= (($x + 1 >= $w) ? 0 : (($d[$n + 1]  != ' ') ? 1 : 0)) << 2;
					$l |= (($y + 1 >= $h) ? 0 : (($d[$n + $w] != ' ') ? 1 : 0)) << 3;
				} else {
					$l = 16;
				}
				
				draw_bitmap($b_tiles[$l], $return, $x * 8, $y * 8, $alpha);
				$n++;
			}
		}
		
		
		return $return;
	}
	
	function load_tileset() {
		global $screen;
		
		global $b_tileset;

		global $b_tiles;
		global $b_snake_head, $b_snake_body, $b_snake_tail;
		global $b_item;
		global $b_life, $b_nlife;
		
		if ($b_tileset = @bitmap_string_load(zip_load('snake_data.dat#tileset.gif'), FILE_GIF)) {		
			$b_tiles = array(); for ($n = 0; $n < 17; $n++) { $b_tiles[$n] = @bitmap_create_sub($b_tileset, $n * 8, (8 * 0), 8, 8); }
			$b_snake_head = array(); for ($n = 0; $n < 4; $n++) { $b_snake_head[$n] = @bitmap_create_sub($b_tileset, $n * 8, (8 * 1), 8, 8); }
			$b_snake_body = array(); for ($n = 0; $n < 8; $n++) { $b_snake_body[$n] = @bitmap_create_sub($b_tileset, $n * 8, (8 * 2), 8, 8); }
			$b_snake_tail = array(); for ($n = 0; $n < 4; $n++) { $b_snake_tail[$n] = @bitmap_create_sub($b_tileset, $n * 8, (8 * 3), 8, 8); }
			$b_item = @bitmap_create_sub($b_tileset, (8 * 0), (8 * 4), 8, 8);		
			$b_life = @bitmap_create_sub($b_tileset, (8 * 0), (8 * 5), 8, 8);
			$b_nlife = @bitmap_create_sub($b_tileset, (8 * 1), (8 * 5), 8, 8);
			return true;			
		} else {
			return false;
		}		
	}
	
	function pm_filetype($extension) {
		switch(trim(strtolower($extension))) {
			case 'jpg': return FILE_JPG; break;
			case 'gif': return FILE_GIF; break;
			case 'bmp': return FILE_BMP; break;
			case 'pcx': return FILE_PCX; break;
			case 'png': return FILE_PNG; break;
			case 'tga': return FILE_TGA; break;
			default: return FILE_BMP; break;
		}
	}
	
	function gsb($f, $t) {
		$f++; $t++; $v = ($f * 10) + $t;		
		switch($v) {
			case 11: $r = 0; break;  // U D
			case 12: $r = 6; break;  // U R
			case 13: $r = 7; break;  // U L
			case 14: $r = 0; break;  // U U
			
			case 21: $r = 5; break;  // L D
			case 22: $r = 1; break;  // L R
			case 23: $r = 1; break;  // L L			
			case 24: $r = 7; break;  // L U
			
			case 31: $r = 4; break;  // R D
			case 32: $r = 2; break;  // R R
			case 33: $r = 2; break;  // R L
			case 34: $r = 6; break;  // R U
			
			case 41: $r = 3; break;  // D D
			case 42: $r = 4; break;  // D R
			case 43: $r = 5; break;  // D L
			case 44: $r = 3; break;  // D U
			default: $r = 0; break;
		}
		//return 0;
		return $r;
	}	
?>