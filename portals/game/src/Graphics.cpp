#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <cmath>
#include <algorithm>


#include "FixP.h"
#include "LoadBitmap.h"
#include "Renderer.h"
#include "Graphics.h"

uint8_t transparency = 0;

Rect clippingRect{0, 0, 320, 200};

NativeBitmap *font = NULL;
FixP zero{0};

void graphicsInitFont(const char *path) {
	font = loadBitmap(path);
}

using TextureFixP = FixP;

Rect graphicsGetCurrentClipRect() {
	return clippingRect;
}

void graphicsSetClipRect(const Rect &rect) {
	clippingRect = rect;
}

void graphicsSetClipRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
	clippingRect.x0 = x0;
	clippingRect.y0 = y0;
	clippingRect.x1 = x1;
	clippingRect.y1 = y1;
}

void graphicsEncloseClipRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
	clippingRect.x0 = std::max(x0, clippingRect.x0);
	clippingRect.y0 = std::max(y0, clippingRect.y0);
	clippingRect.x1 = std::min(x1, clippingRect.x1);
	clippingRect.y1 = std::min(y1, clippingRect.y1);
}

/*
  *         /|x1y0
  * x0y0   / |
  *       |  |
  *       |  |
  * x0y1  |  |
  *       \  |
  *        \ |
  *         \| x1y1
  */
void
graphicsDrawWall(FixP x0, FixP x1, FixP x0y0, FixP x0y1, FixP x1y0, FixP x1y1,
				 NativeBitmap *texture, FixP mx, FixP my) {

	if (x0 > x1) {
		//switch x0 with x1
		x0 = x0 + x1;
		x1 = x0 - x1;
		x0 = x0 - x1;

		//switch x0y0 with x1y0
		x0y0 = x0y0 + x1y0;
		x1y0 = x0y0 - x1y0;
		x0y0 = x0y0 - x1y0;

		//switch x0y1 with x1y1
		x0y1 = x0y1 + x1y1;
		x1y1 = x0y1 - x1y1;
		x0y1 = x0y1 - x1y1;
	}

	const auto x = static_cast<int16_t >(x0);
	const auto limit = static_cast<int16_t >(x1);

	if (x == limit) {
		return;
	}

	FixP upperY0 = x0y0;
	FixP lowerY0 = x0y1;
	FixP upperY1 = x1y0;
	FixP lowerY1 = x1y1;

	if (x0y0 > x0y1) {
		upperY0 = x0y1;
		lowerY0 = x0y0;
		upperY1 = x1y1;
		lowerY1 = x1y0;
	};

	const FixP upperDy = upperY1 - upperY0;
	const FixP lowerDy = lowerY1 - lowerY0;

	FixP y0 = upperY0;
	FixP y1 = lowerY0;

	const FixP dX = FixP{limit - x};
	const FixP upperDyDx = upperDy / dX;
	const FixP lowerDyDx = lowerDy / dX;

	uint8_t pixel = 0;
	TextureFixP u{0};

	//0xFF here acts as a dirty value, indicating there is no last value. But even if we had
	//textures this big, it would be only at the end of the run.
	uint8_t lastU = 0xFF;
	uint8_t lastV = 0xFF;

	//we can use this statically, since the textures are already loaded.
	//we don't need to fetch that data on every run.
	uint8_t *data = texture->data;
	const uint16_t textureWidth = texture->width;
	const uint16_t textureHeight = texture->height;
	const FixP texWidth{textureWidth};
	const FixP texHeight = FixP{textureHeight} * my;

	const TextureFixP du = (texWidth / dX) * mx;

	int_fast16_t ix = x;
	uint8_t *bufferData = &buffer[0];

	for (; ix < limit; ++ix) {
		if (ix >= clippingRect.x0 && ix < clippingRect.x1) {

			const FixP diffY = (y1 - y0);

			if (diffY == zero) {
				continue;
			}

			const TextureFixP dv = texHeight / diffY;
			TextureFixP v{0};
			uint8_t iu = static_cast<uint8_t >(u) % textureHeight;
			int_fast16_t iY0 = static_cast<int16_t >(y0);
			int_fast16_t iY1 = static_cast<int16_t >(y1);
			uint8_t *sourceLineStart = data + (iu * textureWidth);
			uint8_t *lineOffset = sourceLineStart;
			uint8_t *destinationLine = bufferData + (320 * iY0) + ix;

			lastV = 0;
			pixel = *(lineOffset);

			for (int_fast16_t iy = iY0; iy < iY1; ++iy) {

				if (iy < clippingRect.y1 && iy >= clippingRect.y0) {
					const uint8_t iv = static_cast<uint8_t >(v);

					if (iv != lastV) {
						pixel = *(lineOffset);
						lineOffset = ((iv & (textureWidth - 1)) +
									  sourceLineStart);
						lastU = iu;
						lastV = iv;
					}

					if (pixel != transparency) {
						*(destinationLine) = pixel;
					}
				}
				destinationLine += (320);
				v += dv;
			}
		}
		y0 += upperDyDx;
		y1 += lowerDyDx;
		u += du;
#ifdef PUT_A_FLIP
		videoFlip();
#endif
	}
}

/*
 *     x0y0 ____________ x1y0
 *         /            \
 *        /             \
 *  x0y1 /______________\ x1y1
 */
void
graphicsDrawFloor(FixP y0, FixP y1, FixP x0y0, FixP x1y0, FixP x0y1, FixP x1y1,
				  NativeBitmap *texture, FixP mx, FixP my) {

	//if we have a trapezoid in which the base is smaller
	if (y0 > y1) {
		//switch y0 with y1
		y0 = y0 + y1;
		y1 = y0 - y1;
		y0 = y0 - y1;

		//switch x0y0 with x0y1
		x0y0 = x0y0 + x0y1;
		x0y1 = x0y0 - x0y1;
		x0y0 = x0y0 - x0y1;

		//switch x1y0 with x1y1
		x1y0 = x1y0 + x1y1;
		x1y1 = x1y0 - x1y1;
		x1y0 = x1y0 - x1y1;
	}

	const int16_t y = static_cast<int16_t >(y0);
	const int_fast16_t limit = static_cast<int16_t >(y1);

	if (y == limit) {
		return;
	}

	FixP upperX0 = x0y0;
	FixP upperX1 = x1y0;
	FixP lowerX0 = x0y1;
	FixP lowerX1 = x1y1;

	//what if the trapezoid is flipped horizontally?
	if (x0y0 > x1y0) {
		upperX0 = x1y0;
		upperX1 = x0y0;
		lowerX0 = x1y1;
		lowerX1 = x0y1;
	};

	const FixP leftDX = lowerX0 - upperX0;
	const FixP rightDX = lowerX1 - upperX1;
	const FixP dY = y1 - y0;
	const FixP leftDxDy = leftDX / dY;
	const FixP rightDxDy = rightDX / dY;
	FixP x0 = upperX0;
	FixP x1 = upperX1;

	uint8_t pixel = 0;

	TextureFixP v{0};

	//0xFF here acts as a dirty value, indicating there is no last value. But even if we had
	//textures this big, it would be only at the end of the run.
	uint8_t lastU = 0xFF;
	uint8_t lastV = 0xFF;

	int_fast16_t iy = static_cast<int16_t >(y);

	uint8_t *bufferData = &buffer[0];
	uint8_t *data = texture->data;
	const int8_t textureWidth = texture->width;
	const int8_t textureHeight = texture->height;
	const FixP texWidth{textureWidth};
	const FixP texHeight{textureHeight};

	const TextureFixP dv = (texHeight / dY) * my;

	for (; iy < limit; ++iy) {

		if (iy < clippingRect.y1 && iy >= clippingRect.y0) {

			const FixP diffX = (x1 - x0);

			if (diffX == zero) {
				continue;
			}

			int_fast16_t iX0 = static_cast<int16_t >(x0);
			int_fast16_t iX1 = static_cast<int16_t >(x1);

			const TextureFixP du = texWidth / (diffX / mx);
			TextureFixP u{0};
			const uint8_t iv = static_cast<uint8_t >(v) % textureHeight;
			uint8_t *sourceLineStart = data + (iv * textureWidth);
			uint8_t *destinationLine = bufferData + (320 * iy) + iX0;
			lastU = 0;
			pixel = *(sourceLineStart);

			for (int_fast16_t ix = iX0; ix < iX1; ++ix) {

				if (ix >= clippingRect.x0 && ix < clippingRect.x1) {
					const uint8_t iu = static_cast<uint8_t >(u) % textureWidth;

					//only fetch the next texel if we really changed the u, v coordinates
					//(otherwise, would fetch the same thing anyway)
					if (iu != lastU) {
						pixel = *(sourceLineStart);
						sourceLineStart += (iu - lastU);
						lastU = iu;
						lastV = iv;
					}

					if (pixel != transparency) {
						*(destinationLine) = pixel;
					}
				}
				++destinationLine;
				u += du;
			}
		}

		x0 += leftDxDy;
		x1 += rightDxDy;
		v += dv;
#ifdef PUT_A_FLIP
		videoFlip();
#endif
	}
}

void graphicsPut(int16_t x, int16_t y, uint8_t color) {
	buffer[(320 * y) + x] = color;
}

void graphicsDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {

	if (x0 == x1) {

		int16_t _y0 = y0;
		int16_t _y1 = y1;

		if (y0 > y1) {
			_y0 = y1;
			_y1 = y0;
		}


		for (int16_t y = _y0; y <= _y1; ++y) {
			graphicsPut(x0, y, 128);
		}
		return;
	}

	if (y0 == y1) {
		int16_t _x0 = x0;
		int16_t _x1 = x1;

		if (x0 > x1) {
			_x0 = x1;
			_x1 = x0;
		}

		for (int16_t x = _x0; x <= _x1; ++x) {
			graphicsPut(x, y0, 128);
		}
		return;
	}

	//switching x0 with x1
	if (x0 > x1) {
		x0 = x0 + x1;
		x1 = x0 - x1;
		x0 = x0 - x1;

		y0 = y0 + y1;
		y1 = y0 - y1;
		y0 = y0 - y1;
	}

	FixP fy = FixP{y0};
	FixP fDeltatY = FixP{y1 - y0} / FixP{x1 - x0};

	for (int16_t x = x0; x <= x1; ++x) {
		graphicsPut(x, static_cast<int16_t >(fy), 128);
		fy += fDeltatY;
	}
}

void
graphicsDrawSprite(FixP x0, FixP y0, FixP x1, FixP y1, NativeBitmap *texture,
				   bool useAlpha, FixP mx, FixP my) {

	//if we have a quad in which the base is smaller
	if (y0 > y1) {
		//switch y0 with y1
		y0 = y0 + y1;
		y1 = y0 - y1;
		y0 = y0 - y1;
	}

	const int16_t y = static_cast<int16_t >(y0);
	const int_fast16_t limit = static_cast<int16_t >(y1);

	if (y == limit) {
		//degenerate
		return;
	}

	//what if the quad is flipped horizontally?
	if (x0 > x1) {
		x0 = x0 + x1;
		x1 = x0 - x1;
		x0 = x0 - x1;
	};

	const FixP dY = (y1 - y0);

	uint8_t pixel = 0;
	TextureFixP v{0};

	//0xFF here acts as a dirty value, indicating there is no last value. But even if we had
	//textures this big, it would be only at the end of the run.
	uint8_t lastU = 0xFF;
	uint8_t lastV = 0xFF;

	int_fast16_t iy = static_cast<int16_t >(y);

	uint8_t *data = texture->data;

	const uint16_t textureWidth = texture->width;
	const uint16_t textureHeight = texture->height;
	const FixP texWidth{textureWidth};
	const FixP texHeight{textureHeight};

	const TextureFixP dv = (texHeight / dY) * my;

	const FixP diffX = (x1 - x0);

	int_fast16_t iX0 = static_cast<int16_t >(x0);
	int_fast16_t iX1 = static_cast<int16_t >(x1);

	if (iX0 == iX1) {
		//degenerate case
		return;
	}

	const TextureFixP du = (mx * texWidth) / (diffX);

	uint8_t *bufferData = &buffer[0];

	for (; iy < limit; ++iy) {

		if (iy < clippingRect.y1 && iy >= clippingRect.y0) {
			TextureFixP u{0};
			const uint8_t iv = static_cast<uint8_t >(v);
			uint8_t *sourceLineStart =
					data + ((iv & (textureHeight - 1)) * textureWidth);
			uint8_t *destinationLine = bufferData + (320 * iy) + iX0;

			lastU = 0;
			/*
			if ( !useAlpha && iv == lastV ) {
		  v += dv;
		  destinationLine = bufferData + (320 * iy);
		  sourceLineStart = destinationLine - 320;
		  int16_t start = ( 0 >= iX0 ) ? 0 : iX0;
		  int16_t finish = ( (256 - 1) >= iX1 ) ? iX1 : (256 - 1);
		  memcpy( destinationLine + start, sourceLineStart + start, finish - start );
	  #ifdef PUT_A_FLIP
		  videoFlip();
	  #endif
		  continue;
		  }*/

			pixel = *(sourceLineStart);

			for (int_fast16_t ix = iX0; ix < iX1; ++ix) {

				if (ix < clippingRect.x1 && ix >= clippingRect.x0) {

					const uint8_t iu = static_cast<uint8_t >(u) % textureWidth;
					//only fetch the next texel if we really changed the u, v coordinates
					//(otherwise, would fetch the same thing anyway)
					if (iu != lastU) {
						pixel = *(sourceLineStart);
						sourceLineStart += (iu - lastU);
						lastU = iu;
						lastV = iv;
					}


					if (pixel != transparency) {
						*(destinationLine) = pixel;
					}
				}
				++destinationLine;
				u += du;
			}
		}
		v += dv;
#ifdef PUT_A_FLIP
		videoFlip();
#endif
	}

}

void
graphicsDrawTextAt(int16_t x, int16_t y, const char *text, uint8_t colour) {

	size_t len = strlen(text);
	int16_t dstX = (x - 1) * 8;
	int16_t dstY = (y - 1) * 8;
	uint8_t *dstBuffer = &buffer[0];
	uint16_t fontWidth = font->width;
	uint8_t *fontPixelData = font->data;

	for (size_t c = 0; c < len; ++c) {
		uint8_t ascii = text[c] -
						' '; //the font starts with space, thus it's interesting to base the ascii code from there.
		uint8_t line = ascii >> 5; //font divided in 32 chars per line
		uint8_t col = ascii & 31; //what char in the line itself
		uint8_t *letter = fontPixelData + (col * 8) + (fontWidth * (line * 8));

		if (text[c] == '\n' || dstX >= 320) {
			dstX = 0;
			dstY += 8;
			continue;
		}

		if (text[c] == ' ') {
			dstX += 8;

			if (dstX >= 320) {
				dstX = 0;
				dstY += 8;
			}

			continue;
		}

		for (uint_fast8_t srcY = 0; srcY < 8; ++srcY) {

			uint8_t *letterSrc = letter + (fontWidth *
										   srcY); //skip the remaining font line to read the next line of the font
			uint8_t *letterDst = dstBuffer + dstX + (320 * (dstY + srcY));

			for (uint_fast8_t srcX = 0; srcX < 8; ++srcX) {

				uint8_t index = *letterSrc;

				if (index != transparency) {
					*letterDst = colour;
				}

				++letterSrc;
				++letterDst;
			}
		}
		dstX += 8;
	}
}

void graphicsBlit(int16_t dx, int16_t dy, NativeBitmap *src) {

	uint8_t *destination = &buffer[0];
	uint8_t *sourceLine = &src->data[0];
	size_t height = src->height;
	size_t width = src->width;
	size_t offsetX = 0;
	size_t offsetY = 0;

	if (dx < 0) {
		offsetX = -dx;
		width += dx;
		dx = 0;
	}

	if ((dx + width) >= 319) {
		width = 319 - dx;
	}

	if (dy < 0) {
		offsetY = -dy;
		height += dy;
		dy = 0;
	}

	if ((dy + height) >= 199) {
		height = 199 - dy;
	}

	for (size_t y = 0; y < height; ++y) {

		if ((dy + y) < clippingRect.y0) {
			continue;
		}

		if ((dy + y) >= clippingRect.y1) {
			continue;
		}

		uint8_t *destinationLineStart = destination + (320 * (dy + y)) + dx;
		uint8_t *sourceLineStart =
				sourceLine + (src->width * (offsetY + y)) + offsetX;

		for (size_t x = 0; x < width; ++x) {
			uint8_t pixel = (*sourceLineStart);

			if (pixel != transparency) {
				*destinationLineStart = pixel;
			}

			++sourceLineStart;
			++destinationLineStart;
		}
	}
}


uint8_t getPaletteEntry(uint32_t origin) {

	if (!(origin & 0xFF000000)) {
		return transparency;
	}

	uint8_t shade = 0;

	shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
	shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
	shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

	return shade;
}

void
graphicsFill(int16_t x, int16_t y, uint16_t dx, uint16_t dy, uint8_t pixel) {

	if (pixel == transparency) {
		return;
	}

	uint8_t *destination = &buffer[0];

	for (int16_t py = 0; py < dy; ++py) {
		uint8_t *destinationLineStart = destination + (320 * (y + py)) + x;
		memset(destinationLineStart, pixel, dx);
	}
}


void
graphicsDrawRect(int16_t x, int16_t y, uint16_t dx, uint16_t dy, uint8_t pixel) {

    if (pixel == transparency) {
        return;
    }

    uint8_t *destination = &buffer[0];

    memset(destination + (320 * (y)) + x, pixel, dx);

    for (int16_t py = 0; py < dy; ++py) {
        *(destination + (320 * (y + py)) + x) = pixel;
        *(destination + (320 * (y + py)) + x + dx) = pixel;
    }

    memset(destination + (320 * (y + dy)) + x, pixel, dx);

}