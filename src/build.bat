@echo off
SET php_path=..\..\..\..\php-5.3.0RC1
SET sources=php_media.c
SET library=..\php_media.dll
SET MYLIBS=kernel32.lib ole32.lib user32.lib advapi32.lib shell32.lib ws2_32.lib Dnsapi.lib
SET MYLIBS=%MYLIBS% SDL.lib opengl32.lib SDL_image.lib SDL_mixer.lib SDL_ttf.lib
del php_media.dll 2> NUL
@"%VCINSTALLDIR%\bin\cl.exe" /nologo /FD /I "%php_path%" /I "%php_path%\main" /I "%php_path%\Zend" /I "%php_path%\TSRM" /I "%php_path%\ext" /D _WINDOWS /D ZEND_WIN32=1 /D PHP_WIN32=1 /D WIN32 /D _MBCS /wd4996 /D_USE_32BIT_TIME_T=1 /MP /LD /MD /W3 /Ox /D NDebug /D NDEBUG /D ZEND_WIN32_FORCE_INLINE /GF /D ZEND_DEBUG=0 /D ZTS=1 /I "no\include" /D FD_SETSIZE=256 /Fd.\ /Fp.\ /FR.\ %SOURCES% "%php_path%\php5ts.lib" %MYLIBS% /link /out:%library% /dll /nologo /version:5.3.0 /libpath:"\"no\lib\""