<?php
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
}

/**
 * Channel class
 *
 */
final class Channel {
	public $playing;
	public $position;
	public $length;
	public $sound;
}

/**
 * Sound class
 *
 */
final class Sound {
	
}

/**
 * Audio class
 *
 */
final class Audio {
	/**
	* Initializes the audio subsystem.
	*
	* @param int $frequency Frequency in hz for the sampling.
	* @return Channel
	*/
	static public function init($frequency = 22050) { }
}

/**
 * Bitmap class
 *
 */
final class Bitmap {
	public $parent;
	public $x, $y;
	public $w, $h;
	public $cx, $cy;
	
	public function __construct($w, $h) { }
	static public function fromFile($filename) { }
	static public function fromString($string) { }

	/**
	* Duplicates the bitmap and the texture.
	*
	* @return Bitmap
	*/	
	public function duplicate() { }

	/**
	* Clones the Bitmap without cloning the texture. Useful for using the same texture with different center addresses.
	*
	* @return Bitmap
	*/	
	public function __clone() { }

	/**
	* Makes a slice of the bitmap without duplicating the texture.
	*
	* @return Bitmap
	*/	
	public function slice($x, $y, $w, $h) { }

	/**
	* Makes an array of slices of the bitmap without duplicating the texture.
	*
	* @return Array
	*/	
	public function split($w, $h) { } // Like slice

	/**
	* Clears the bitmap with the specified color.
	*/	
	public function clear($color = NULL) { } // array(0.0, 0.0, 0.0, 0.0)
	public function blit(Bitmap $src, $x, $y, $size = 1.0, $rotation = 0, $alpha = 1.0, Shader $shader = NULL) { }
}

/**
 * Shader class
 *
 */
final class Shader {
	public $params = array();

	public function __construct($string) { }
}
?>