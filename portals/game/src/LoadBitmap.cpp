#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "LoadBitmap.h"

NativeBitmap *loadBitmap(const char *filename) {

	NativeBitmap *toReturn = (NativeBitmap *) calloc(1, sizeof(NativeBitmap));

	FILE *src = fopen(filename, "rb");

	uint8_t width = 0;
	uint8_t height = 0;

	fread(&width, 1, 1, src);
	toReturn->width += (width & 0xFF) << 8;
	fread(&width, 1, 1, src);
	toReturn->width += width & 0xFF;

	fread(&height, 1, 1, src);
	toReturn->height += (height & 0xFF) << 8;
	fread(&height, 1, 1, src);
	toReturn->height += height & 0xFF;

	size_t size = toReturn->width * toReturn->height;

	uint8_t *buffer = (uint8_t *) calloc(1, size);

	fread(&buffer[0], size, 1, src);
	fclose(src);
	toReturn->data = buffer;

	return toReturn;
}


void releaseBitmap(NativeBitmap *ptr) {
	assert(ptr != NULL);

	free(ptr->data);
	free(ptr);
}

void releaseTexture(Texture *ptr) {
	releaseBitmap(ptr->regular);
	releaseBitmap(ptr->rotated);
	free(ptr);
}

Texture *makeTextureFrom(NativeBitmap *bitmap) {
	Texture *toReturn = (Texture *) calloc(1, sizeof(Texture));

	toReturn->regular = bitmap;
	toReturn->rotated = (NativeBitmap *) calloc(1, sizeof(NativeBitmap));
	toReturn->rotated->width = bitmap->height;
	toReturn->rotated->height = bitmap->width;

	size_t size = bitmap->width * bitmap->height;
	toReturn->rotated->data = (uint8_t *) calloc(1, size);


	for (int y = 0; y < bitmap->height; ++y) {
		auto sourceLine = &bitmap->data[y * bitmap->width];
		for (int x = 0; x < bitmap->width; ++x) {
			toReturn->rotated->data[(x * bitmap->width) + y] = *sourceLine;
			sourceLine++;
		}
	}

	return toReturn;
}
