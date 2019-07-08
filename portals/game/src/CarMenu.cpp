#include <stdio.h>
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
#include "SpyTravel.h"

const char** CarMenu_options;
const int32_t* CarMenu_nextStateNavigation;
const char *mainText = NULL;
const char *CarMenu_StateTitle;

const char* CarMenu_Main_options[4] = {
  "Leer los expedientes",
  "Investigar",
  "Viajar",
  //  "BASIC",
  "Salir del juego"
};

int32_t CarMenu_Main_nextStateNavigation[5] = {
  kDossiersMenu,
  kInvestigateMenu,
  kTravelMenu,
  //  kBASIC,
  kMainMenu
};

const char* CarMenu_Dossier_options[9] = {
  "Sofia",
  "Ricardo",
  "Juan",
  "Pau",
  "Lina",
  "Elias",
  "Carmen",
  "Jean",
  "Volver"
};

int32_t CarMenu_Travel_nextStateNavigation[9] = {
  kTravelPorto,
  kTravelLisbon,
  kTravelMadrid,
  kTravelBarcelona,
  kTravelFrankfurt,
  kTravelSaarbrucken,
  kTravelLuxembourg,
  kTravelCharleroi,
  kPlayGame
};

const char* CarMenu_Travel_options[9] = {
  "Porto",
  "Lisbon",
  "Madrid",
  "Barcelona",
  "Frankfurt",
  "Saarbrucken",
  "Luxembourg",
  "Charleroi",
  "Volver"
};

int32_t CarMenu_Dossier_nextStateNavigation[9] = {
  kReadDossier_Sofia,
  kReadDossier_Ricardo,
  kReadDossier_Juan,
  kReadDossier_Pau,
  kReadDossier_Lina,
  kReadDossier_Elias,
  kReadDossier_Carmen,
  kReadDossier_Jean, 
  kPlayGame  
};

const char* CarMenu_ReadDossier_options[9] = {
  "Back"
};

int32_t CarMenu_ReadDossier_nextStateNavigation[9] = {
  kDossiersMenu,
};


size_t CarMenu_optionsCount = 6;
uint8_t biggestOption;
char textBuffer[40 * 25];
int32_t CarMenu_substate;


int32_t CarMenu_initStateCallback(int32_t tag, void* data) {
  CarMenu_StateTitle = NULL;
  cursorPosition = 0;
  CarMenu_substate = tag;
  currentPresentationState = kAppearing;
  timeUntilNextState = 500;
  memset(&textBuffer[0], ' ', 40 * 25 );
  
  switch(tag) {

  case kPlayGame:
    CarMenu_StateTitle = "Investigacion:";
    getDisplayStatusText(&textBuffer[0], 40*10);
    mainText = &textBuffer[0];
    currentBackgroundBitmap = loadBitmap("res/gamebg.img");    
    CarMenu_optionsCount = 4;
    CarMenu_options = &CarMenu_Main_options[0];
    CarMenu_nextStateNavigation = &CarMenu_Main_nextStateNavigation[0];
    break;

  case kDossiersMenu:
    CarMenu_StateTitle = u8"Expedientes:";
    getDisplayStatusText(&textBuffer[0], 40*10);
    mainText = &textBuffer[0];        
    currentBackgroundBitmap = loadBitmap("res/gamebg.img");    
    CarMenu_optionsCount = 9;
    CarMenu_options = &CarMenu_Dossier_options[0];
    CarMenu_nextStateNavigation = &CarMenu_Dossier_nextStateNavigation[0];    
    break;

  case kTravelMenu:
    CarMenu_StateTitle = u8"Viajar:";
    getDisplayStatusText(&textBuffer[0], 40*10);
    mainText = &textBuffer[0];    
    currentBackgroundBitmap = loadBitmap("res/gamebg.img");    
    CarMenu_optionsCount = 9;
    CarMenu_options = &CarMenu_Travel_options[0];
    CarMenu_nextStateNavigation = &CarMenu_Travel_nextStateNavigation[0];    
    break;    

  case kReadDossier_Sofia:
  case kReadDossier_Ricardo:
  case kReadDossier_Juan:
  case kReadDossier_Pau:
  case kReadDossier_Lina:
  case kReadDossier_Elias:
  case kReadDossier_Carmen:
  case kReadDossier_Jean: {
    CarMenu_StateTitle = u8"Expediente:";
    currentBackgroundBitmap = loadBitmap("res/gamebg.img");    
    
    mainText = &textBuffer[0];
    getDossierText( tag - kReadDossier_Sofia, &textBuffer[0], 40 * 25 );
    
    CarMenu_optionsCount = 1;
    CarMenu_options = &CarMenu_ReadDossier_options[0];
    CarMenu_nextStateNavigation = &CarMenu_ReadDossier_nextStateNavigation[0];        
  }
  }
  
  biggestOption = strlen(CarMenu_StateTitle);
  
  for ( int c = 0; c < CarMenu_optionsCount; ++c ) {
    auto len = strlen(CarMenu_options[c]);

    if (len > biggestOption) {
      biggestOption = len;
    }

  }

  return 0;
}

void CarMenu_initialPaintCallback(void) {

  if ( currentBackgroundBitmap != NULL ) {
    graphicsBlit(0, 0, currentBackgroundBitmap);
  }

  
  if ( mainText != NULL ) {
    graphicsDrawTextAt( 1, 1, mainText, 200 );
  }
}

void CarMenu_repaintCallback(void) {

  uint8_t optionsHeight = 8 * (CarMenu_optionsCount);
  
  graphicsFill( 320 - ( biggestOption * 8) - 16,
		200 - optionsHeight - 16,
		(biggestOption * 8) + 16,
		optionsHeight + 16, 255);

  if (CarMenu_StateTitle != NULL) {
    graphicsDrawTextAt( 40 - biggestOption, (26 - CarMenu_optionsCount) - 2, CarMenu_StateTitle, 128 );  
  }

  for ( int c = 0; c < CarMenu_optionsCount; ++c ) {

    bool isCursor = (cursorPosition == c ) && ( (currentPresentationState == kConfirmInputBlink1) ||
							 (currentPresentationState == kConfirmInputBlink3) ||
							 (currentPresentationState == kConfirmInputBlink5) ||							 
							 (currentPresentationState == kWaitingForInput) );

    if (isCursor) {
      graphicsFill( 320 - ( biggestOption * 8) - 16,
		    ( 200 - optionsHeight ) + ( c * 8),
		    (biggestOption * 8) + 16,
		    8, 0 );
    }

    bool shouldGreyOut = (CarMenu_substate == kTravelMenu && (c < 8) && suspects[c].giveClue);
    
    graphicsDrawTextAt( 40 - biggestOption, (26 - CarMenu_optionsCount) + c, &CarMenu_options[c][0], isCursor ?
			( shouldGreyOut ? 128 : 200) : ( shouldGreyOut ? 16 : 0) );
  }
}

int32_t CarMenu_tickCallback(int32_t tag, void* data) {

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
      cursorPosition = (cursorPosition - 1);

      if ( cursorPosition >= CarMenu_optionsCount ) {
	cursorPosition = CarMenu_optionsCount - 1;
      }
      
      break;
    case kCommandDown:
      cursorPosition = (cursorPosition + 1) % CarMenu_optionsCount;
      break;
    case kCommandFire1:

      if ( CarMenu_substate == kTravelMenu && cursorPosition < 8) {
	mapGameTick( cursorPosition);
      }
      
      if ( CarMenu_substate == kInvestigateMenu ) {
	mapGameTick(' ');
      }      
      nextNavigationSelection = CarMenu_nextStateNavigation[cursorPosition];
      currentPresentationState = kConfirmInputBlink1;
      break;
    }
  }
  
  return -1;
}

void CarMenu_unloadStateCallback() {
  if (currentBackgroundBitmap != NULL ) {
    releaseBitmap(currentBackgroundBitmap);
    currentBackgroundBitmap = NULL;
  }
}
