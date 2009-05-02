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
	 *
	 * @return Bitmap
	 */
	static public function init($w = 640, $h = 480, $fullscreen = false) { } // Bitmap $screen

	/**
	 * Swaps the buffers. Clears the offscreen buffer. Update events. Wait until the frame is finished.
	 */
	static public function frame() { }

	/**
	 * Set or retrieves the title of the window.
	 *
	 * @param string $title If passed, changes the title of the window.
	 *
	 * @return Bitmap
	 */
	static public function title($title = null) { }
}

/**
 * Channel class
 *
 */
class Channel {
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
class Sound {
	static public function fromFile($file) { }
	static public function fromString($data) { }
	public function slice($start = 0, $length = NULL) { }
	public function play() { }
}

/**
 * SoundEffect class
 *
 */
class SoundEffect {
}

/**
 * Class to control the music subsystem.
 */
final class Music {
	/**
	 * Starts playing a song. In WAVE, MOD, MIDI, OGG, MP3.
	 *
	 * @param string $file     Filename of the song.
	 * @param int    $loops    Number of loops.
	 * @param float  $fadeIn   Number seconds for the FadeIn transition.
	 * @param float  $position Number seconds to start playing the song.
	 */
	static public function play($file, $loops = -1, $fadeIn = 1, $position = 0) { }

	/**
	 * Stops the song with an optional FadeOut transition.
	 *
	 * @param float  $fadeOut   Number seconds for the FadeOut transition.
	 */
	static public function stop($fadeOut = 1) { }

	/**
	 * Returns if the song is playing.
	 *
	 * @return bool
	 */
	static public function playing() { }
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
class Bitmap {
	const FORMAT_BMP = 0;
	const FORMAT_TGA = 1;
	const FORMAT_PNG = 2;
	const FORMAT_JPG = 3;

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
	 * Saves this Bitmap as a file in the specified format.
	 *
	 * @param string $filename File name.
	 */
	static public function saveFile($filename, $format = Bitmap::FORMAT_BMP) { }

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
	 * Blits this bitmap in another.
	 *
	 * @param Bitmap $dest Bitmap to blit on.
	 * @param float  $x Red.
	 * @param float  $y Green.
	 * @param float  $size Size.
	 * @param float  $rotation Rotation.
	 * @param float  $alpha Alpha.
	 * @param Shader $shader Shader.
	 * @param array  $shader_params Parameters for the shader.
	 */
	public function blit(Bitmap $dest, $x, $y, $size = 1.0, $rotation = 0, $alpha = 1.0, Shader $shader = NULL, array $shader_params = NULL) { }
}

/**
 * Class to specify programs to execute for each pixel.
 */
class Shader {
	/**
	 * Creates a shader.
	 *
	 * @param string  $shader_fragment Shader fragment unit.
	 * @param string  $shader_vertex   Shader vertex unit.
	 */
	public function __construct($shader_fragment = NULL, $shader_vertex = NULL) { }
	
	/**
	 * Starts a shader program.
	 *
	 * @param array  $shader_params Parameters for the shader.
	 */
	public function begin($shader_params = array()) { }

	/**
	 * Ends a shader program.
	 */
	public function end() { }
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
	const LEFT       = 1;
	const MIDDLE     = 2;
	const RIGHT      = 3;
	const WHEEL_UP   = 4;
	const WHEEL_DOWN = 5;

	/**
	 * Shows the mouse cursor.
	 */
	static public function show() { }

	/**
	 * Hide the mouse cursor.
	 */
	static public function hide() { }

	/**
	 * Gets or sets the mouse position.
	 *
	 * @param int $x Sets x position of the mouse cursor.
	 * @param int $y Sets y position of the mouse cursor.
	 *
	 * @return array($x, $y)
	 */
	static public function position($x = NULL, $y = NULL) { }

	/**
	 * Returns if the button is currently pressed.
	 *
	 * @param int $but Button to check.
	 * @return bool
	 */
	static public function pressed($but) { }

	/**
	 * Returns if the button is currently down.
	 *
	 * @param int $but Button to check.
	 * @return bool
	 */
	static public function down($but) { }
}

/**
 * Class to draw texts on bitmaps.
 */
class Font {
	public $fixed;
	public $style; // read/write
	public $height;
	public $ascent;
	public $descent;
	public $lineSkip;
	public $faceName;
	public $styleName;

	static public function fromFile($filename, $size = 16, $index = 0) { }
	static public function fromString($filename, $size = 16, $index = 0) { }
	static public function fromName($names, $size = 16, $index = 0) { }

	public function width($text) { }
	public function height($text = '') { }
	public function metrics($char) { }
	
	public function blit(Bitmap $dest, $text, $x, $y, $color) { }
}

/**
 * Class for extra math operations.
 */
final class Math {
	/**
	 * Clamps a variable passed by reference by a minimum and maximum value.
	 *
	 * @param mixed $min Minimum value.
	 * @param mixed $max Maximum value.
	 * @param mixed &$input Variable to check and update.
	 */
	static public function clamp($min, $max, &$input) { }

	/**
	 * The function will return a number between 0 and 1 that represents the relationship of the $input value to the $min and $max values. 
	 *
	 * @param mixed $min Minimum value.
	 * @param mixed $max Maximum value.
	 * @param mixed &$input Variable to check and update.
	 */
	static public function smoothstep($min, $max, &$input) { }
}

/**
 * Class for compiling and executing fast C code. Using libtcc.
 *
 * @see http://bellard.org/tcc/
 */
class TCC {
	/**
	 * Creates a new TCC state.
	 */
	public function __construct() { }

	/**
	 * #define $key $value.
	 *
	 * @param string $key   Key.
	 * @param string $value Value.
	 */
	public function define($key, $value) { }

	/**
	 * Compiles C source from a string.
	 *
	 * @param string $string String with C source.
	 */
	public function sourceString($string) { }

	/**
	 * Compiles a C file.
	 *
	 * @param string $filename Filename to compile.
	 */
	public function sourceFile($filename) { }

	/**
	 * Lib to link. It must be a lib or exp file.
	 *
	 * @param string $filename Filename with the lib to link.
	 */
	public function libFile($filename) { }

	/**
	 * Adds a lib path to search for libs.
	 *
	 * @param string $path Path to search for libs.
	 */
	public function libPath($path) { }

	/**
	 * Adds an include path to search for includes.
	 *
	 * @param string $path Path to search for includes.
	 */
	public function includePath($path) { }

	/**
	 * Executes a main function.
	 */
	public function runMain() { }

	/**
	 * Calls a function.
	 *
	 * @param string $format name:params:retval.
	 * @param array  $params Array with params to pass to the function.
	 */
	public function call($format, array $params) { }
}
