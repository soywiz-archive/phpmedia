<?php
/**
 * Screen class
 *
 */
final class Screen {
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

	public function duplicate() { }
	public function __clone() { }
	public function slice($x, $y, $w, $h) { }
	public function split($w, $h) { } // Like slice
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