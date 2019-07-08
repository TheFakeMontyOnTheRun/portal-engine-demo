#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "FixP.h"
#include "LoadBitmap.h"
#include "Renderer.h"
#include "Graphics.h"
#include "Engine.h"


#include "SDL.h"

SDL_Window *window;
SDL_Renderer *renderer;

uint32_t palette[256];
uint8_t buffer[320 * 200];
uint8_t lastCommand = kCommandNone;

void eventsInit(void) {
	transparency = 199;
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow( "Portals",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                640,
                                400,
                                SDL_WINDOW_SHOWN );

    renderer = SDL_CreateRenderer( window, -1, 0 );

	for (int r = 0; r < 256; r += 16) {
		for (int g = 0; g < 256; g += 8) {
			for (int b = 0; b < 256; b += 8) {
				uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
				uint8_t paletteEntry = getPaletteEntry(pixel);
				palette[paletteEntry] = pixel;
			}
		}
	}
}


void eventsHandle() {
	SDL_Event event;

	lastCommand = kCommandNone;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT) {
			isRunning = false;
		}

		if (event.type == SDL_KEYDOWN) {

			switch (event.key.keysym.sym) {

				case SDLK_UP:
					lastCommand = kCommandUp;
					break;

				case SDLK_RIGHT:
					lastCommand = kCommandRight;
					break;

				case SDLK_DOWN:
					lastCommand = kCommandDown;
					break;

				case SDLK_LEFT:
					lastCommand = kCommandLeft;
					break;

				case SDLK_z:
				case SDLK_RETURN:
					lastCommand = kCommandFire1;
					break;

				case SDLK_x:
				case SDLK_SPACE:
					lastCommand = kCommandFire2;
					break;

				case SDLK_ESCAPE:
					lastCommand = kCommandBack;
					break;

				default:
					lastCommand = kCommandNone;
			}
		}
	}
}


void videoDestroy() {
	SDL_Quit();
}

void videoFlip() {
	SDL_Rect rect;

	for (int y = 0; y < 200; ++y) {
		for (int x = 0; x < 320; ++x) {

			rect.x = 2 * x;
			rect.y = 2 * y;
			rect.w = 2;
			rect.h = 2;

			uint32_t pixel = palette[buffer[(320 * y) + x]];

            SDL_SetRenderDrawColor( renderer, (pixel & 0x000000FF) - 0x38,
                                    ((pixel & 0x0000FF00) >> 8) - 0x18,
                                    ((pixel & 0x00FF0000) >> 16) - 0x10,
                                    255 );
            SDL_RenderFillRect( renderer, &rect );

		}
	}

    SDL_RenderPresent( renderer );
}
