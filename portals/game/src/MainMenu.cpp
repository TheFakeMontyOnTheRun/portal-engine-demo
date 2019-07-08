#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "FixP.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "Graphics.h"

const char* MainMenu_options[2] = {
  "Test",
  "Quit"
};

int32_t MainMenu_nextStateNavigation[2] = {
  kPracticeCrawling,
  kQuit
};

int biggestOption;

int32_t MainMenu_initStateCallback(int32_t tag, void* data) {
  cursorPosition = 0;

  if (currentBackgroundBitmap != NULL) {
    releaseBitmap(currentBackgroundBitmap);
  }
  
  currentBackgroundBitmap = loadBitmap("res/title_bg.img");
  currentPresentationState = kAppearing;
  timeUntilNextState = 500;

  biggestOption = 0;
  
  for ( int c = 0; c < 2; ++c ) {
    auto len = strlen(MainMenu_options[c]);

    if (len > biggestOption) {
      biggestOption = len;
    }

  }
  return 0;
}

void MainMenu_initialPaintCallback(void) {
  graphicsBlit(0, 0, currentBackgroundBitmap);
}

void MainMenu_repaintCallback(void) {

  uint8_t optionsHeight = 8 * 2;
  
  graphicsFill( 320 - ( biggestOption * 8),
		200 - optionsHeight,
		(biggestOption * 8),
		optionsHeight, 255);

  for ( int c = 0; c < 2; ++c ) {

    bool isCursor = (cursorPosition == c ) && ( (currentPresentationState == kConfirmInputBlink1) ||
							 (currentPresentationState == kConfirmInputBlink3) ||
							 (currentPresentationState == kConfirmInputBlink5) ||						   
							 (currentPresentationState == kWaitingForInput) );

    if (isCursor) {
      graphicsFill( 320 - ( biggestOption * 8),
		    ( 200 - optionsHeight ) + ( c * 8),
		    (biggestOption * 8),
		    8, 0 );
    }
    
    graphicsDrawTextAt( 40 - biggestOption + 1, (26 - 2) + c, &MainMenu_options[c][0], isCursor ? 200 : 0 );
  }
}

int32_t MainMenu_tickCallback(int32_t tag, void* data) {

  long delta = *((long*)data);

  timeUntilNextState -= delta;

  if (timeUntilNextState <= 0 ) {

    switch( currentPresentationState ) {
    case kAppearing:
      timeUntilNextState = 500;
      currentPresentationState = kWaitingForInput;
      break;
    case kWaitingForInput:
      break;
    case kConfirmInputBlink1:
    case kConfirmInputBlink2:
    case kConfirmInputBlink3:
    case kConfirmInputBlink4:
    case kConfirmInputBlink5:
    case kConfirmInputBlink6:      
      timeUntilNextState = 250;
      currentPresentationState = (EPresentationState)((int)currentPresentationState + 1);
      break;
    case kFade:
      return nextNavigationSelection;
      break;
    }
  }

  if (currentPresentationState == kWaitingForInput ) {  
  
    switch (tag) {
    case kCommandUp:
      cursorPosition = cursorPosition - 1;

      if ( cursorPosition > 2 ) {
	cursorPosition = 2;
      }
      
      break;
    case kCommandDown:
      cursorPosition = (cursorPosition + 1) % 2;
      break;
    case kCommandFire1:      
      nextNavigationSelection = MainMenu_nextStateNavigation[cursorPosition];
      currentPresentationState = kConfirmInputBlink1;
      break;
    }
  }
  
  return -1;
}

void MainMenu_unloadStateCallback() {
  releaseBitmap(currentBackgroundBitmap);
  currentBackgroundBitmap = NULL;
}
