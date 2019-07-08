#ifndef __DJGPP__
const long UCLOCKS_PER_SEC = 10000;

long timeEllapsed = 0;


long uclock()  {
  timeEllapsed += (1000/60);
  return timeEllapsed;
}

#endif


#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <time.h>

#include "FixP.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "Renderer.h"
#include "Graphics.h"

bool isRunning = true;
InitStateCallback initStateCallback = NULL;
InitialPaintCallback initialPaintCallback = NULL;
RepaintCallback repaintCallback = NULL;
TickCallback tickCallback = NULL;
UnloadStateCallback unloadStateCallback = NULL;

long timeUntilNextState = 500;
EPresentationState currentPresentationState;
NativeBitmap* currentBackgroundBitmap = NULL;
uint8_t cursorPosition = 0;
int32_t	nextNavigationSelection = -1;
int32_t menuStateToReturn = -1;
int32_t currentGameMenuState = -1;

void enterState(int32_t newState ) {

  if (unloadStateCallback != NULL) {
    unloadStateCallback();
  }

  timeUntilNextState = 500;
  currentPresentationState = kAppearing;
  currentBackgroundBitmap = NULL;
  cursorPosition = 0;
  nextNavigationSelection = -1;

  switch( newState ) {
  default:
  case kMainMenu:
    initStateCallback = MainMenu_initStateCallback;
    initialPaintCallback = MainMenu_initialPaintCallback;
    repaintCallback = MainMenu_repaintCallback;
    tickCallback = MainMenu_tickCallback;
    unloadStateCallback = MainMenu_unloadStateCallback;
    break;

  case kPracticeCrawling:
    menuStateToReturn = kMainMenu;
    initStateCallback = Crawler_initStateCallback;
    initialPaintCallback = Crawler_initialPaintCallback;
    repaintCallback = Crawler_repaintCallback;
    tickCallback = Crawler_tickCallback;
    unloadStateCallback = Crawler_unloadStateCallback;    
    break;

  case kQuit:
    isRunning = false;
    break;
  }

  currentGameMenuState = newState;

  initStateCallback(newState, NULL);
  initialPaintCallback();

}

int main(int argc, char **argv ) {
  puts("The Mistral Report tech demo - 2018 - Daniel Monteiro");
  srand(time(NULL));
  graphicsInitFont("res/font.img");
  eventsInit();

  enterState( kMainMenu ); 

  clock_t diff = 0;

  
  while (isRunning) {


    clock_t t0 = 0;
    clock_t t1 = 0;
    t0 = uclock();
    
    eventsHandle();
    repaintCallback();  

    t1 = uclock();
    diff = (10000 * (t1 - t0)) / UCLOCKS_PER_SEC;

    if (diff == 0) {
      diff = 1;
    }
    
#ifdef __DJGPP
    /*    
    char buffer[80];
    graphicsFill( 0, 0, 64, 8, 255 );
    snprintf(buffer, 80, "t: %ld", diff);
    graphicsDrawTextAt( 1, 1, buffer, 0 );    
    */
#endif

    int32_t newState = tickCallback(lastCommand, &diff);
    
    videoFlip();

    if (newState != currentGameMenuState && newState != -1 ) {
      enterState( newState );
    }
  }

  unloadStateCallback();
  
  videoDestroy();


  return 0;
}
