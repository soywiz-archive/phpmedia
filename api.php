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
 * Audio class
 *
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