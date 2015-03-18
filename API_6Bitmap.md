## class Bitmap ##
Class to handle, modify and draw images.

<font face='Lucida Console'>
<ul><li><img src='http://phpmedia.googlecode.com/svn/www/icons/method.png' /><a href='API_6Bitmap11__construct.md'>__construct</a> ($w, $h) : <font color='#a88'><a href='API_6Bitmap.md'>Bitmap</a></font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/method_static.png' /><a href='API_6Bitmap8fromFile.md'>fromFile</a> ($filename) : <font color='#a88'><a href='API_6Bitmap.md'>Bitmap</a></font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/method_static.png' /><a href='API_6Bitmap10fromString.md'>fromString</a> ($string) : <font color='#a88'><a href='API_6Bitmap.md'>Bitmap</a></font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/method_static.png' /><a href='API_6Bitmap8saveFile.md'>saveFile</a> ($filename, $format<font color='#999'> = 0</font>) : <font color='#a88'>void</font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/method.png' /><a href='API_6Bitmap7__clone.md'>__clone</a> () : <font color='#a88'><a href='API_6Bitmap.md'>Bitmap</a></font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/method.png' /><a href='API_6Bitmap5slice.md'>slice</a> ($x, $y, $w, $h) : <font color='#a88'><a href='API_6Bitmap.md'>Bitmap</a></font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/method.png' /><a href='API_6Bitmap5split.md'>split</a> ($w, $h) : <font color='#a88'>Array</font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/method.png' /><a href='API_6Bitmap5clear.md'>clear</a> ($r<font color='#999'> = 0</font>, $g<font color='#999'> = 0</font>, $b<font color='#999'> = 0</font>, $a<font color='#999'> = 0</font>) : <font color='#a88'>void</font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/method.png' /><a href='API_6Bitmap4blit.md'>blit</a> (<a href='API_6Bitmap.md'>Bitmap</a> $dest, $x, $y, $size<font color='#999'> = 1</font>, $rotation<font color='#999'> = 0</font>, $alpha<font color='#999'> = 1</font>, <a href='API_6Shader.md'>Shader</a> $shader<font color='#999'> = NULL</font>, $shader_params<font color='#999'> = NULL</font>) : <font color='#a88'>void</font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/field.png' /><a href='API_6Bitmap1x.md'>$x</a>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/field.png' /><a href='API_6Bitmap1y.md'>$y</a>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/field.png' /><a href='API_6Bitmap1w.md'>$w</a>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/field.png' /><a href='API_6Bitmap1h.md'>$h</a>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/field.png' /><a href='API_6Bitmap2cx.md'>$cx</a>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/field.png' /><a href='API_6Bitmap2cy.md'>$cy</a>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/constant.png' /><a href='API_6Bitmap10FORMAT_BMP.md'>FORMAT_BMP</a> = <font color='#a88'>0</font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/constant.png' /><a href='API_6Bitmap10FORMAT_TGA.md'>FORMAT_TGA</a> = <font color='#a88'>1</font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/constant.png' /><a href='API_6Bitmap10FORMAT_PNG.md'>FORMAT_PNG</a> = <font color='#a88'>2</font>
</li><li><img src='http://phpmedia.googlecode.com/svn/www/icons/constant.png' /><a href='API_6Bitmap10FORMAT_JPG.md'>FORMAT_JPG</a> = <font color='#a88'>3</font>
</font>