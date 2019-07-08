#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <crt0.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>
#include <sys/types.h>

#include "FixP.h"
#include "LoadBitmap.h"
#include "Renderer.h"
#include "Graphics.h"
#include "Engine.h"

uint8_t buffer[320 * 200];
uint8_t lastCommand = kCommandNone;


void eventsInit(void) {

	__dpmi_regs reg;

	reg.x.ax = 0x13;

	__dpmi_int(0x10, &reg);

	outp(0x03c8, 0);

	for (int r = 0; r < 4; ++r) {
		for (int g = 0; g < 8; ++g) {
			for (int b = 0; b < 8; ++b) {
				outp(0x03c9, (r * (21)));
				outp(0x03c9, (g * (8)));
				outp(0x03c9, (b * (8)));
			}
		}
	}
	transparency = 199;
}


void eventsHandle() {
	lastCommand = kCommandNone;

	if (kbhit()) {
		switch (getch()) {

			case 27:
				lastCommand = kCommandBack;
				break;

			case 13:
			case 'z':
				lastCommand = kCommandFire1;
				break;

			case ' ':
			case 'x':

				lastCommand = kCommandFire2;
				break;

			case 224:
			case 0:
				switch (getch()) {
					case 75:
						lastCommand = kCommandLeft;
						break;
					case 72:
						lastCommand = kCommandUp;
						break;
					case 77:
						lastCommand = kCommandRight;
						break;
					case 80:
						lastCommand = kCommandDown;
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
}


void videoDestroy() {
	textmode(C80);
	clrscr();
}

void videoFlip() {
	dosmemput(&buffer[0], 320 * 200, 0xa0000);
}  
