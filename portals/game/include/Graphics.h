#ifndef GRAPHICS_H
#define GRAPHICS_H

struct Rect {
  int16_t x0 = 0;
  int16_t y0 = 0;
  int16_t x1 = 0;
  int16_t y1 = 0;  
};


uint8_t getPaletteEntry(uint32_t color);
void graphicsBlit( int16_t x, int16_t y, NativeBitmap* bitmap);
void graphicsFill( int16_t x, int16_t y, uint16_t dx, uint16_t dy, uint8_t pixel );
void graphicsInitFont(const char* path);
void graphicsDrawTextAt( int16_t x, int16_t y, const char* text, uint8_t colour );
void graphicsDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void graphicsPut( int16_t x0, int16_t y0, uint8_t color );
void graphicsDrawSprite( FixP x, FixP y, FixP x1, FixP y1, NativeBitmap* sprite, bool useAlpha = false, FixP mx = FixP{1}, FixP my = FixP{1} );
void graphicsDrawFloor(FixP y0, FixP y1, FixP x0y0, FixP x1y0, FixP x0y1, FixP x1y1, NativeBitmap* texture, FixP mx = FixP{1}, FixP my = FixP{1});
void graphicsDrawWall( FixP x0, FixP x1, FixP x0y0, FixP x0y1, FixP x1y0, FixP x1y1, NativeBitmap* texture, FixP mx = FixP{1}, FixP my = FixP{1});
void graphicsSetClipRect( int16_t x0, int16_t y0, int16_t x1, int16_t y1 );
void graphicsEncloseClipRect( int16_t x0, int16_t y0, int16_t x1, int16_t y1 );
Rect graphicsGetCurrentClipRect();
void graphicsSetClipRect( const Rect& rect);


extern uint8_t transparency ;
extern Rect clippingRect;
#endif
