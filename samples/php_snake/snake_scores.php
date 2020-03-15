<?php
	// Este sistema es muy facil de trucar evidentemente pero no hay ninguna forma de evitar
	// que se pueda o no trucar... porque el servidor no hace un seguimiento de la partida
	// y es muy facil porque está el codigo fuente :D
	function snake_scores($_points = NULL) {
		global $font, $screen;
		
		$strbg = zip_load('snake_data.dat#back_hightscores.gif');
		$bg = bitmap_string_load($strbg, FILE_GIF);
		
		$_screen = next_vpage();
		draw_bitmap($bg, $_screen, 0, 0);
		
		font_size($font, 20);
		font_write_aa($font, $_screen, 59, 59, 'Cargando...', color_black);		
		font_write_aa($font, $_screen, 60, 60, 'Cargando...', color_white);
		
		next_vpage();		
		
		$table = unserialize(strin('[code]', '[/code]', file_load('http://usuarios.lycos.es/phpwiz/snake_hightscores.php')));
		
		font_size($font, 17);
		$n = 0; $m = 0;
		$x = 40;
		
		
		$maxcol = array(255, 230, 98); $mincol = array(255, 255, 255);
				
		foreach($table as $row) {			
			list($name, $points) = $row;			
						
			$color = array();						
			for ($u = 0; $u < 3; $u++) { $color[$u] = (($maxcol[$u] - $mincol[$u]) / 20) * (20 - $m) + $mincol[$u]; }
			$color = color_make($color[0], $color[1], $color[2]);
			
			$m++;
			
			font_write_aa($font, $bg, $x + 1, 57 + $n * 16, "$m.", color_black);
			font_write_aa($font, $bg, $x, 58 + $n * 16, "$m.", color_black);
			font_write_aa($font, $bg, $x, 57 + $n * 16, "$m.", $color);
			
			font_write_aa($font, $bg, $x + 1 + 20, 57 + $n * 16, $name, color_black);
			font_write_aa($font, $bg, $x + 20, 58 + $n * 16, $name, color_black);
			font_write_aa($font, $bg, $x + 20, 57 + $n * 16, $name, $color);
			
			font_write_aa($font, $bg, $x + 71, 57 + $n * 16, $points, color_black);
			font_write_aa($font, $bg, $x + 70, 58 + $n * 16, $points, color_black);
			font_write_aa($font, $bg, $x + 70, 57 + $n * 16, $points, $color);
			$n++;
			if ($n >= 10) { $n = 0; $x += 155; }
		}
		
		$writemode = ($_points != NULL && $_points > $table[19][1]);		
		
		draw_clear($trans = bitmap_create(bitmap_width($screen), 80), 0);		
		
		$_name = '';
		
		$list = array(
			KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K,
			KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V,
			KEY_W, KEY_X, KEY_Y, KEY_Z		
		);
		
		$keys = array();
		for ($n = 0; $n < 26; $n++) {
			$keys[$list[$n]] = array(chr($n + 65), false);
		}
		
		$keyback = false;
		
		while(!key_pressed(KEY_ESC)) {
			$_screen = next_vpage(); wait(40);
			draw_bitmap($bg, $_screen, 0, 0);
			
			if ($writemode) {
				draw_bitmap($trans, $_screen, 0, bitmap_height($_screen) / 2 - bitmap_height($trans) / 2, 128);
				font_write_aa($font, $_screen, 4, bitmap_height($_screen) / 2 - bitmap_height($trans) / 2 + 5, "Felicidades has conseguido: $_points puntos!", color_white);
				font_write_aa($font, $_screen, 50, bitmap_height($_screen) / 2 - bitmap_height($trans) / 2 + 30, $_name . '_', color_white);				
				
				// NOTa para mi ... añadir al php media mas funciones para teclado ¬¬
				// tipo key_last en ascii... y key_clear_buffer y tal XD
				foreach($keys as $kn => $kv) {
					list($cadd, $cpress) = $kv;
					if (!key_pressed($kn) && $cpress) { $keys[$kn] = array($cadd, false); }
					if (key_pressed($kn) && !$cpress && strlen($_name) < 3) { $keys[$kn] == array($cadd, true); $_name .= $cadd; }					
				}
				
				if (key_pressed(KEY_BACKSPACE) && strlen($_name) > 0 && !$keyback) { $_name = substr($_name, 0, -1); $keyback = true; }
				if (!key_pressed(KEY_BACKSPACE) && $keyback) $keyback = false;
				if (key_pressed(KEY_ENTER)) {
					file_load('http://usuarios.lycos.es/phpwiz/snake_hightscores.php?a=add&n=' . $_name . '&p=' . $_points);
					break;
				}
			}
			
		}	
		return false;
	}
?>