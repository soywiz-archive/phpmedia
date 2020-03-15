#define ALLEGRO_USE_CONSOLE

#include <allegro.h>
#include "loadpng.h"
#include <stdio.h>

#define u32 unsigned int
#define u16 unsigned short
#define u8  char
#define inline __inline

/*
static u32 colorMask = 0xF7DEF7DE;
static u32 lowPixelMask = 0x08210821;
static u32 qcolorMask = 0xE79CE79C;
static u32 qlowpixelMask = 0x18631863;
static u32 redblueMask = 0xF81F;
static u32 greenMask = 0x7E0;
*/

static u32 colorMask     = 0xfefefefe;
static u32 lowPixelMask  = 0x01010101;
static u32 qcolorMask    = 0xfcfcfcfc;
static u32 qlowpixelMask = 0x03030303;

//static u32 alphaMask   = 0x000000ff;
/*
static u32 noalphaMask   = 0xffffff00;
static u32 alphaMask   = 0xff000000;
*/

//static u32 noalphaMask   = 0x00ffffff;
static u32 noalphaMask   = 0xffffffff; // NO DEBE SER ASÍ (DEBE SER COMO ARRIBA) (NOTA1)
static u32 alphaMask   = 0xff000000;

//static u32 noalphaMask   = 0x00ffffff;
//static u32 alphaMask   = 0xff000000;

/*
static u32 colorMask     = 0xfefefe;
static u32 lowPixelMask  = 0x010101;
static u32 qcolorMask    = 0xcffcfc;
static u32 qlowpixelMask = 0x000303;
*/

static int alphaBlack = 0;
static int usingPink = 0;

#define SE_DEBUG if (0)
//#define SE_DEBUG allegro_message

static inline int EQ_COLOR(u32 A, u32 B) {
	// NO DEBE SER ASÍ (NOTA1)
	//return (getr(A) == getr(B) && getg(A) == getg(B) && getb(A) == getb(B));
	return ((A & noalphaMask) == (B & noalphaMask));
	return A == B;
}

static inline int GetResult(u32 A, u32 B, u32 C, u32 D) {
	int x = 0, y = 0, r = 0;

	if (A == C) x += 1; else if (B == C) y += 1;
	if (A == D) x += 1; else if (B == D) y += 1;

	if (x <= 1) r += 1;
	if (y <= 1) r -= 1;

	return r;
}

static inline u32 INTERPOLATE(u32 A, u32 B) {
	//u32 cA, cB, cG, cR;
	// Si el color es el mismo
	if (A == B) return A;
	//return A;

	/*
	// ABGR

	cA = (((A & 0x000000ff) >>  0) + ((B & 0x000000ff)) >>  0) >> 1;
	cB = (((A & 0x0000ff00) >>  8) + ((B & 0x0000ff00)) >>  8) >> 1;
	cG = (((A & 0x00ff0000) >> 16) + ((B & 0x00ff0000)) >> 16) >> 1;
	cR = (((A & 0xff000000) >> 24) + ((B & 0xff000000)) >> 24) >> 1;

	return (cA << 0) | (cB << 8) | (cG << 16) | (cR << 24);
	*/

	return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask));
}

static inline u32 GET_CHECKED_COLOR(BITMAP *bmp, u32 x, u32 y, u32 w, u32 h) {
	if (x < 0 || y < 0 || x >= w || y >= h) {
		return 0;
	}

	{
		u32 c = getpixel(bmp, x, y);

		/*
		if (x == 340 && y == 300) {
			printf("DEBUG P(340, 300) : (%8x, %8x : %8x)\n", c, alphaMask, c & alphaMask);
		}
		*/

		//if ((c & alphaMask) == 0) return 0;

		if (alphaBlack && geta(c) == 0) return 0;

		return c;
	}
}

//#define T_CHECKED_COLOR(bmp, x, y, w, h) getpixel(bmp, x, y)
#define T_CHECKED_COLOR(bmp, x, y, w, h) GET_CHECKED_COLOR(bmp, x, y, w, h)

static inline u32 Q_INTERPOLATE (u32 A, u32 B, u32 C, u32 D) {
	//return A;

	register u32 x = ((A & qcolorMask) >> 2) + ((B & qcolorMask) >> 2) + ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
	register u32 y = (A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);
	y = (y >> 2) & qlowpixelMask;
	return x + y;
}

//void SuperEagle32(u8 *srcPtr, u32 srcPitch, u8 *deltaPtr, u8 *dstPtr, u32 dstPitch, int width, int height) {
BITMAP *SuperEagle32(BITMAP *src) {
	int width, height;

	int sX, sY;
	//int tX, tY;
	int dX, dY;
	BITMAP *dst;
	BITMAP *delta;
	u32 inc_bP;
	u32 finish;

	SE_DEBUG("STEP[40]\n");

	// Crea los bitmaps temporales
	width  = src->w;
	height = src->h;

	if (!(dst   = create_bitmap_ex(32, width * 2, height * 2))) {
		SE_DEBUG("ERROR [1]!\n");
		return NULL;
	} clear_bitmap(dst);

	/*
	if (!(delta = create_bitmap_ex(32, width, height))) {
		SE_DEBUG("ERROR [2]!\n");
		return NULL;
	} clear_bitmap(delta);
	*/

	SE_DEBUG("STEP[41]\n");

	sX = 0; sY = 0;
	//tX = 0; tY = 0;
	dX = 0; dY = 0;

	// Posición X, Y del SRC
	// Posición X, Y del TEMP (delta)
	// Posición X, Y del DST

	inc_bP = 1;

	SE_DEBUG("STEP[4a]\n");

	for (; height; height--) {
		for (finish = width; finish; finish -= inc_bP) {
			u32 color4,    color5,    color6;
			u32 color1,    color2,    color3;
			u32 colorA1,   colorA2,   colorB1,   colorB2, colorS1, colorS2;
			u32 product1a, product1b, product2a, product2b;

			// Obtiene los píxeles cercanos usados para el supereagle

			colorB1 = T_CHECKED_COLOR(src, sX    , sY - 1, src->w, src->h);
			colorB2 = T_CHECKED_COLOR(src, sX + 1, sY - 1, src->w, src->h);

			color4  = T_CHECKED_COLOR(src, sX - 1, sY    , src->w, src->h);
			color5  = T_CHECKED_COLOR(src, sX    , sY    , src->w, src->h);
			color6  = T_CHECKED_COLOR(src, sX + 1, sY    , src->w, src->h);
			colorS2 = T_CHECKED_COLOR(src, sX + 1, sY    , src->w, src->h);

			color1  = T_CHECKED_COLOR(src, sX - 1, sY + 1, src->w, src->h);
			color2  = T_CHECKED_COLOR(src, sX    , sY + 1, src->w, src->h);
			color3  = T_CHECKED_COLOR(src, sX + 1, sY + 1, src->w, src->h);
			colorS1 = T_CHECKED_COLOR(src, sX + 2, sY + 1, src->w, src->h);

			colorA1 = T_CHECKED_COLOR(src, sX    , sY + 2, src->w, src->h);
			colorA2 = T_CHECKED_COLOR(src, sX + 1, sY + 2, src->w, src->h);
			// /

			if (sY == 0) {
				colorB2 = colorB1 = 0;
			}

			if (noalphaMask)

			//if (color2 == color6 && color5 != color3) {
			if (EQ_COLOR(color2, color6) && !EQ_COLOR(color5, color3)) {
				product1b = product2a = color2;

				//if ((color1 == color2) || (color6 == colorB2)) {
				if (EQ_COLOR(color1, color2) || EQ_COLOR(color6, colorB2)) {
					product1a = INTERPOLATE(color2, color5);
					product1a = INTERPOLATE(color2, product1a);
				} else {
					product1a = INTERPOLATE(color5, color6);
				}

				//if ((color6 == colorS2) || (color2 == colorA1)) {
				if (EQ_COLOR(color6, colorS2) || EQ_COLOR(color2, colorA1)) {
					product2b = INTERPOLATE(color2, color3);
					product2b = INTERPOLATE(color2, product2b);
				} else {
					product2b = INTERPOLATE(color2, color3);
				}

			} else
			//if (color5 == color3 && color2 != color6) {
			if (EQ_COLOR(color5, color3) && !EQ_COLOR(color2, color6)) {

				product2b = product1a = color5;

				//if ((colorB1 == color5) || (color3 == colorS1)) {
				if (EQ_COLOR(colorB1, color5) || EQ_COLOR(color3, colorS1)) {
					product1b = INTERPOLATE(color5, color6);
					product1b = INTERPOLATE(color5, product1b);
				} else {
					product1b = INTERPOLATE (color5, color6);
				}

				//if ((color3 == colorA2) || (color4 == color5)) {
				if (EQ_COLOR(color3, colorA2) || EQ_COLOR(color4, color5)) {
					product2a = INTERPOLATE(color5, color2);
					product2a = INTERPOLATE(color5, product2a);
				} else {
					product2a = INTERPOLATE(color2, color3);
				}

			} else
			//if (color5 == color3 && color2 == color6) {
			if (EQ_COLOR(color5, color3) && EQ_COLOR(color2, color6)) {
				register int r = 0;

				r += GetResult(color6, color5, color1,  colorA1);
				r += GetResult(color6, color5, color4,  colorB1);
				r += GetResult(color6, color5, colorA2, colorS1);
				r += GetResult(color6, color5, colorB2, colorS2);

				if (r > 0) {
					product1b = product2a = color2;
					product1a = product2b = INTERPOLATE(color5, color6);
				} else if (r < 0) {
					product2b = product1a = color5;
					product1b = product2a = INTERPOLATE(color5, color6);
				} else {
					product2b = product1a = color5;
					product1b = product2a = color2;
				}
			} else {
				product2b = product1a = INTERPOLATE(color2, color6);
				product2b = Q_INTERPOLATE(color3, color3, color3, product2b);
				product1a = Q_INTERPOLATE(color5, color5, color5, product1a);
				product2a = product1b = INTERPOLATE (color5, color3);
				product2a = Q_INTERPOLATE(color2, color2, color2, product2a);
				product1b = Q_INTERPOLATE(color6, color6, color6, product1b);
			}

			if (usingPink) {
				u8 alpha;
				#define checkpink1a(color) alpha = geta(color); if (alpha < 0xff) { if (geta(color) < 200) color = 0xffff00ff; else color |= 0xff000000;  }
				//#define checkpink1a(color) alpha = (color & 0xff000000) >> 24; if (alpha < 200) { /*color = 0xffff00ff;*/ } else { color |= 0xff000000; }
				//#define checkpink1a(color) alpha = geta(color); if (alpha < 200) { /*color = 0xffff00ff;*/ } else { color |= 0xff000000; }

				checkpink1a(product1a);
				checkpink1a(product1b);
				checkpink1a(product2a);
				checkpink1a(product2b);
			}

			putpixel(dst, dX    , dY    , product1a);
			putpixel(dst, dX + 1, dY    , product1b);
			putpixel(dst, dX    , dY + 1, product2a);
			putpixel(dst, dX + 1, dY + 1, product2b);

			//putpixel(delta, tX, tY, color5);

			dX += 2;
			sX++;
			//tX++;
		}

		sY++;    sX = 0;
		//tY++;    tX = 0;
		dY += 2; dX = 0;
	}

	SE_DEBUG("STEP[4b]\n");

	//destroy_bitmap(delta);

	return dst;
}

int main(int argc, char *argv[]) {
	char *src_filename, *dst_filename;

	BITMAP *realBitmap;
	BITMAP *loadBitmap;
	BITMAP *saveBitmap;

	printf("SuperEagle Utility v0.1        Copyright (c) Carlos Ballesteros Velasco 2005\n");
	printf("                               Based on 2xSai Engine: Derek Liauw Kie Fa, 1999\n");
	printf("This program is free for non-commercial use.\n\n");

	if (argc < 3) {
		printf("Usage: sa <input.png> <output.png>\n");
		printf("<input.png> and <output.png> must be the first two parameters!\n");
		printf("       <input.png> must be an 8/16/24/32 bit PNG file.\n");
		printf("       <ouput.png> will be a 32 bit PNG file.\n\n");

		return 1;
	}

	//printf("\n");

	src_filename = argv[1];  //first parameter is source file
	dst_filename = argv[2];  //second parameter is destination file

	//allegro_init();

	//SE_DEBUG("STEP[1]\n");

	if (install_allegro(SYSTEM_NONE, &errno, atexit)) {
		printf("Allegro failed to initialise.\n");
		return 1;
	}

	//SE_DEBUG("STEP[2]\n");

	//at_exit(allegro_exit);

	//set_color_conversion(COLORCONV_NONE);
	set_color_depth(32);

	//SE_DEBUG("STEP[3]\n");

	//loadBitmap = load_bitmap("test.tga", NULL);
	//loadBitmap = load_bitmap("test.bmp", NULL);
	loadBitmap = load_png(src_filename, NULL);
	if (!loadBitmap) {
		allegro_exit();
		printf("\nError reading source file");
		return 2;
	}

	{
		int x, y;
		for (y = 0; y < loadBitmap->h; y++) {
			for (x = 0; x < loadBitmap->w; x++) {
				// Hack to Pink -> Transparent
				u32 cpix = getpixel(loadBitmap, x, y);
				if (getr(cpix) == 0xff && getg(cpix) == 0x00 && getb(cpix) == 0xff) {
					putpixel(loadBitmap, x, y, 0);
					if (!usingPink) {
						printf("\nNow using pink color for keycolor");
					}
					usingPink = 1;
				}

				if (!alphaBlack) {
					if (geta(getpixel(loadBitmap, x, y)) != 0) {
						if (!alphaBlack) {
							printf("\nMap with transparencies");
						}
						alphaBlack = 1;
						//y = loadBitmap->h;
						//break;
					}
				}
			}
		}
	}

	save_png("mytest.png", loadBitmap, NULL);

	//realBitmap = create_bitmap_ex(32, loadBitmap->w, loadBitmap->h);

	//blit(loadBitmap, realBitmap, 0, 0, 0, 0, loadBitmap->w, loadBitmap->h);

	//allegro_message("%x", getpixel(realBitmap, 30, 30));
	//allegro_message("%x", getpixel(loadBitmap, 30, 30));

	//SE_DEBUG("STEP[4]\n");

	//saveBitmap = SuperEagle32(realBitmap);

	saveBitmap = SuperEagle32(loadBitmap);

	SE_DEBUG("STEP[5]\n");

	save_png(dst_filename, saveBitmap, NULL);
	//save_tga("eagle.tga", saveBitmap, NULL);
	//save_tga("eagle.tga", loadBitmap, NULL);

	destroy_bitmap(saveBitmap);
	destroy_bitmap(loadBitmap);

	SE_DEBUG("STEP[6]\n");

	allegro_exit();

	SE_DEBUG("STEP[7]\n");

	return 0;
} END_OF_MAIN()