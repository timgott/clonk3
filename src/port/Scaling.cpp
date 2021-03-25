#include <SDL2/SDL.h>
#include "../standard.h"
#include "../clonk_bs.h" // for color constants

#include "Scaling.h"

bool IsUiColor(Uint8 color) {
 return color >= CBlack && color <= CYellow;
}

void CalculateScale2x(Uint8* src, int x, int y, int width, int height, Uint8* e0, Uint8* e1, Uint8* e2, Uint8* e3) {
	int pitch = width;
	int pos = y*pitch + x;
	Uint8 e = src[pos];
	if (IsUiColor(e)) { // Do not smooth UI colors
		*e0 = e;
		*e1 = e;
		*e2 = e;
		*e3 = e;void BlitIndexedPixelsAndScale(int w, int h, Uint8 *sourcePixels, Uint32 *destPixels, SDL_Color *palette, int scaleFactor, UpscaleInterpolationType interpolationType);
	}
	else {
		bool topInBound = y > 0;
		bool leftInBound = x > 0;
		bool bottomInBound = y < height-1;
		bool rightInBound = x < width-1;
		Uint8 a = leftInBound && topInBound ? src[pos - pitch - 1] : e;
		Uint8 b = topInBound ? src[pos - pitch] : e;
		Uint8 c = rightInBound && topInBound ? src[pos - pitch + 1] : e;
		Uint8 d = leftInBound ? src[pos - 1] : e;
		Uint8 f = rightInBound ? src[pos + 1] : e;
		Uint8 g = leftInBound && bottomInBound ? src[pos + pitch - 1] : e;
		Uint8 h = bottomInBound ? src[pos + pitch] : e;
		Uint8 i = rightInBound && bottomInBound ? src[pos + pitch + 1] : e;
		
		if (b != h && d != f) {
			bool smoothD = !IsUiColor(d);
			bool smoothF = !IsUiColor(f);
			*e0 = d == b && smoothD ? d : e;
			*e1 = b == f && smoothF ? f : e;
			*e2 = d == h && smoothD ? d : e;
			*e3 = h == f && smoothF ? f : e;
		} else {
			*e0 = e;
			*e1 = e;
			*e2 = e;
			*e3 = e;
		}
	}
}

void CalculateScale2xHalf(Uint8* src, int x, int y, int width, int height, Uint8* e0, Uint8* e1, Uint8* e2, Uint8* e3) {
	int pitch = width;
	int pos = y*pitch + x;
	Uint8 e = src[pos];
	if (IsUiColor(e)) { // Do not smooth UI colors
		*e0 = e;
		*e1 = e;
		*e2 = e;
		*e3 = e;
	}
	else {
		bool topInBound = y > 0;
		bool leftInBound = x > 0;
		bool bottomInBound = y < height-1;
		bool rightInBound = x < width-1;
		Uint8 b = topInBound ? src[pos - pitch] : e;
		Uint8 d = leftInBound ? src[pos - 1] : e;
		Uint8 f = rightInBound ? src[pos + 1] : e;
		Uint8 h = bottomInBound ? src[pos + pitch] : e;
		
		if (d != f) {
			bool smoothD = !IsUiColor(d);
			bool smoothF = !IsUiColor(f);
			*e0 = d == b && smoothD ? d : e;
			*e1 = b == f && smoothF ? f : e;
		} else {
			*e0 = e;
			*e1 = e;
		}

		*e2 = e;
		*e3 = e;
	}
}

void ApplyScale2x(Uint8 *src, int w, int h, Uint8 *dest) {
	int destPitch = w*2;

	for (size_t x = 0; x < w; x++)
	{
		for (size_t y = 0; y < h; y++)
		{
			int destPos = y*2*destPitch + x*2;
			CalculateScale2x(src, x, y, w, h,
				&dest[destPos],
				&dest[destPos + 1],
				&dest[destPos + destPitch],
				&dest[destPos + destPitch + 1]
			);
		}
	}
}

void ApplyScale2xHalf(Uint8 *src, int w, int h, Uint8 *dest) {
	int destPitch = w*2;

	for (size_t x = 0; x < w; x++)
	{
		for (size_t y = 0; y < h; y++)
		{
			int destPos = y*2*destPitch + x*2;
			CalculateScale2xHalf(src, x, y, w, h,
				&dest[destPos],
				&dest[destPos + 1],
				&dest[destPos + destPitch],
				&dest[destPos + destPitch + 1]
			);
		}
	}
}

void DrawScaledPixel(Uint32 *pixels, Uint32 value, int destX, int destY, int pitch, int scale) {
	int pos = destX + destY * pitch;
	Uint32 *p = &pixels[pos];
	for (int yy = 0; yy < scale; yy++)
	{
		for (int xx = 0; xx < scale; xx++)
		{
			*p = value;
			p += 1;
		}
		p += pitch - scale;
	}
}

inline Uint32 ColorToUint32(SDL_Color col) {
	return (col.a << 8 * 3) + (col.r << 8 * 2) + (col.g << 8 * 1) + col.b;
}

void BlitIndexedPixelsAndScaleNN(int w, int h, Uint8 *sourcePixels, Uint32 *destPixels, SDL_Color *palette, int scaleFactor) {
	int destPixelSkip = scaleFactor;
	int destLineSkip = w * destPixelSkip;

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			Uint32 destCol = ColorToUint32(palette[sourcePixels[x + y*w]]);
			int destX = x * scaleFactor;
			int destY = y * scaleFactor;
			DrawScaledPixel(destPixels, destCol, destX, destY, destLineSkip, scaleFactor);
		}
	}
}

void BlitIndexedPixelsAndScale2x(int w, int h, Uint8 *sourcePixels, Uint32 *destPixels, SDL_Color *palette, int scaleFactor) {
	int halfScaleFactor = scaleFactor / 2;
	int destPitch = w * scaleFactor;

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			Uint8 e0, e1, e2, e3;
			CalculateScale2x(sourcePixels, x, y, w, h, &e0, &e1, &e2, &e3);

			int destX = x * scaleFactor;
			int destY = y * scaleFactor;
			DrawScaledPixel(destPixels, ColorToUint32(palette[e0]), destX, destY, destPitch, halfScaleFactor);
			DrawScaledPixel(destPixels, ColorToUint32(palette[e1]), destX + halfScaleFactor, destY, destPitch, halfScaleFactor);
			DrawScaledPixel(destPixels, ColorToUint32(palette[e2]), destX, destY + halfScaleFactor, destPitch, halfScaleFactor);
			DrawScaledPixel(destPixels, ColorToUint32(palette[e3]), destX + halfScaleFactor, destY + halfScaleFactor, destPitch, halfScaleFactor);
		}
	}
}

void BlitIndexedPixelsAndScale4x(int w, int h, Uint8 *sourcePixels, Uint32 *destPixels, SDL_Color *palette, int scaleFactor) {
	Uint8 buffer2x[w*2 * h*2]; // allocation on stack, could probably be avoided but who cares.
	ApplyScale2xHalf(sourcePixels, w, h, buffer2x);

	BlitIndexedPixelsAndScale2x(w*2, h*2, buffer2x, destPixels, palette, scaleFactor / 2);
}

void BlitIndexedPixelsAndScale(int w, int h, Uint8 *sourcePixels, Uint32 *destPixels, SDL_Color *palette, int scaleFactor, UpscaleInterpolationType interpolationType) {
	switch (interpolationType)
	{
	case INTERPOLATION_NONE:
		BlitIndexedPixelsAndScaleNN(w, h, sourcePixels, destPixels, palette, scaleFactor);
		break;
	
	case INTERPOLATION_SCALE2X:
		BlitIndexedPixelsAndScale2x(w, h, sourcePixels, destPixels, palette, scaleFactor);
		break;

	case INTERPOLATION_SCALE4X:
		BlitIndexedPixelsAndScale4x(w, h, sourcePixels, destPixels, palette, scaleFactor);
		break;
	}
}

int GetMinScaleFactor(UpscaleInterpolationType interpolationType) {
	switch (interpolationType)
	{
	case INTERPOLATION_NONE:
		return 1;
		break;
	
	case INTERPOLATION_SCALE2X:
		return 2;
		break;

	case INTERPOLATION_SCALE4X:
		return 4;
		break;
	}
}