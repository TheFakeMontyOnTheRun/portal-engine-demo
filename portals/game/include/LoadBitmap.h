#ifndef LOADIMAGE_H
#define LOADIMAGE_H

typedef struct {
  uint8_t* data;
  uint16_t width;
  uint16_t height;
} NativeBitmap;

typedef struct {
  NativeBitmap *regular;
  NativeBitmap *rotated;
} Texture;

Texture *makeTextureFrom(NativeBitmap* bitmap);
NativeBitmap *loadBitmap(const char* filename);
void releaseBitmap(NativeBitmap* ptr);
void releaseTexture(Texture* ptr);
#endif
