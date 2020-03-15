
	loadpng: glue for Allegro and libpng
	

This wrapper is mostly a copy and paste job from example.c in the
libpng docs, stripping out the useless transformations and making it
use Allegro BITMAP and PALETTE structures.  It is placed in the public
domain.


Requirements:

	Allegro		http://alleg.sourceforge.net/
	libpng 		http://www.libpng.org/pub/png/
	zlib 		http://www.info-zip.org/pub/infozip/zlib/

The Allegro version will to be 4.0.x, or 4.1.x.


Usage:

See loadpng.h for functions and their descriptions.  There is a
simple example program called example.c, a program demonstrating
alpha translucency in exalpha.c, and a program demonstrating how to
load a PNG object from a datafile in exdata.c.  The makefile will
show you how to compile.


Gotchas:

Grayscale images will be loaded in as 24 bit images, or 32 bit
images if they contain an alpha channel.  These will then be
converted as usual, according to Allegro's conversion semantics.  Be
wary of this if you have disabled automatic colour depth conversion.


Help:

If you have a BGR video card, please contact me as I cannot test if
I have got BGR handling correctly.  Also, if colour ever comes out
incorrectly (e.g. brown becomes blue), please contact me as well.


Enjoy!

Peter Wang (tjaden@users.sf.net)
http://tjaden.strangesoft.net/

--
PS. I recommend you copy the loadpng files into your program source
trees rather than building and installing it as a library.
