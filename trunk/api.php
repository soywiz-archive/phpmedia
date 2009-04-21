<?php
/**
 * phpmedia API for autogenerating wiki pages.
 *
 * @link http://code.google.com/p/phpmedia/wiki/API
 */

/**
 * Screen class
 *
 */
final class Screen {
	/**
	 * Initializes a window to start drawing.
	 *
	 * @param int $w Width of the window to create.
	 * @param int $h Height of the window to create.
	 * @param boolean $fullscreen Determines wether to create a fullscreen windows or not.
	 * @return Bitmap
	 */
	static public function init($w = 640, $h = 480, $fullscreen = false) { } // Bitmap $screen

	/**
	 * Swaps the buffers. Clears the offscreen buffer. Update events. Wait until the frame is finished.
	 */
	static public function frame() { }

	/**
	 * Set or retrieves the title of the window.
	 */
	static public function title($title = null) { }
}

/**
 * Channel class
 *
 */
final class Channel {
	const FREE = -1;

	public $playing;
	public $position;
	public $length;
	public $sound;

	/**
	 * Obtains a channel of the current index. Use the constante FREE to obtain a free channel.
	 *
	 * @param int $index Channel index.
	 * @return Channel
	 */
	static public function get($index) { }

	/**
	 * Plays the specified sound in this channel.
	 *
	 * @param Sound $sound Sound to play.
	 * @param float $volume Volume to play the sound [0.0, 1.0].
	 * @param float $fadeIn Time in seconds for the fade in.
	 * @param float $fadeOut Time in seconds for the fade out.
	 * @return Channel
	 */
	public function play(Sound $sound, $volume = 1.0, $fadeIn = 0.0, $fadeOut = 0.0, SoundEffect $effect = NULL) { }
}

/**
 * Sound class
 *
 */
final class Sound {
	public function slice($start = 0, $length = NULL) { }
}

/**
 * SoundEffect class
 *
 */
final class SoundEffect {
}

/**
 * Class to control the audio system.
 */
final class Audio {
	/**
	 * Initializes the audio subsystem.
	 *
	 * @param int $frequency Frequency in hz for the sampling.
	 */
	static public function init($frequency = 22050) { }
}

/**
 * Class to handle, modify and draw images.
 */
final class Bitmap {
	//public $parent;
	public $x, $y;
	public $w, $h;
	public $cx, $cy;

	/**
	 * Creates an empty bitmap of the specified size.
	 *
	 * @param int $w Width.
	 * @param int $h Height.
	 * @return Bitmap
	 */
	public function __construct($w, $h) { }

	/**
	 * Creates a bitmap from a file.
	 *
	 * @param string $filename File name.
	 * @return Bitmap
	 */
	static public function fromFile($filename) { }
	
	/**
	 * Creates a bitmap from a image stored in the string.
	 *
	 * @param string $string Image data.
	 * @return Bitmap
	 */
	static public function fromString($string) { }

	/**
	 * Duplicates the bitmap and the texture.
	 *
	 * @return Bitmap
	 */
	//public function duplicate() { }

	/**
	 * Clones the Bitmap without cloning the texture. Useful for using the same texture with different center addresses.
	 *
	 * @return Bitmap
	 */
	public function __clone() { }

	/**
	 * Makes a slice of the bitmap without duplicating the texture.
	 *
	 * @param float $x Coordinate x.
	 * @param float $y Coordinate y.
	 * @param float $w Width.
	 * @param float $h Height.
	 * @return Bitmap
	 */
	public function slice($x, $y, $w, $h) { }

	/**
	 * Makes an array of slices of the bitmap without duplicating the texture.
	 *
	 * @param float $w Width.
	 * @param float $h Height.
	 * @return Array
	 */
	public function split($w, $h) { } // Like slice

	/**
	 * Clears the bitmap with the specified color.
	 *
	 * @param float $r Red.
	 * @param float $g Green.
	 * @param float $b Blue.
	 * @param float $a Alpha.
	 */
	public function clear($r = 0.0, $g = 0.0, $b = 0.0, $a = 0.0) { } // array(0.0, 0.0, 0.0, 0.0)

	/**
	 * Blits the specified bitmap in the current one.
	 *
	 * @param Bitmap $src Source bitmap.
	 * @param float $x Red.
	 * @param float $y Green.
	 * @param float $size Size.
	 * @param float $rotation Rotation.
	 * @param float $alpha Alpha.
	 * @param Shader $shader PixelShader.
	 */
	public function blit(Bitmap $src, $x, $y, $size = 1.0, $rotation = 0, $alpha = 1.0, Shader $shader = NULL) { }
}

/**
 * Class to specify programs to execute for each pixel.
 */
final class Shader {
	public $params = array();

	public function __construct($string) { }
}

/**
 * Class to control keyboard operations.
 */
final class Keyboard {
	/**
	 * Returns if the key is currently pressed.
	 *
	 * @param int $key Key to check.
	 * @return bool
	 */
	static public function pressed($key) { }

	/**
	 * Returns if the key is currently down.
	 *
	 * @param int $key Key to check.
	 * @return bool
	 */
	static public function down($key) { }
}

/**
 * Class to control mouse operations.
 */
final class Mouse {
	/**
	 * Shows the mouse cursor.
	 */
	static public function show() { }

	/**
	 * Hide the mouse cursor.
	 */
	static public function hide() { }
}

final class Font {
	static public function fromFile($filename, $size) { }
	
	public function render($text, $x, $y); // Bitmap
}
?>