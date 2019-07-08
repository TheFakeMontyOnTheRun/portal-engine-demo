#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "SpyTravel.h"

//general stuff below
int hoursLeftInDay = 0;
int daysLeft = 0;
int trainSpeed = 0xFF;
int turnsToCatchBandit = 64;
bool gotHim = false;

int8_t trail[8];
int8_t mercenaryCollaboration[8];
int turns = 0;

const char* placesNames[] = {
  "Porto",
  "Lisbon",
  "Madrid",
  "Barcelona",
  "Frankfurt",
  "Saarbrucken",
  "Luxembourg",
  "Charleroi"
};

int distances[8][8] = {
//   OPO   LIS   MAD   BCN   FRA   SAA   LUX   CHA
  {    0,  313,  560, 1158, 2162, 1985, 2000, 1864}, //Porto
  {  313,    0,  629, 1248, 2317, 2138, 2126, 2018}, //Lisboa
  {  560,  629,    0,  618, 1845, 1668, 1665, 1547}, //Madrid
  { 1158, 1248,  618,    0, 1330, 1159, 1663, 1274}, //Barcelona
  { 2162, 2317, 1845, 1330,    0,  187,  220,  397}, //Frankfurt
  { 1985, 2138, 1668, 1159,  187,    0,  109,  293}, //Saarbrucken
  { 2000, 2126, 1665, 1163,  220,  109,    0,  212}, //Luxemburgo
  { 1864, 2018, 1547, 1274,  397,  293,  212,    0}, //Charleroi
};

Spy suspects[8] = {
  {"Sofia", "Gregalle", ELocation::kPorto, kArt, kIntellectual, 80 },
  {"Ricardo", "Bora", ELocation::kLisbon, kCommerce, kReligion, 67 },
  {"Juan", "Marin", ELocation::kMadrid, kArt, kReligion, 35 },
  {"Pau", "Libeccio", ELocation::kBarcelona, kMilitary, kIntellectual, 53 },
  {"Lina", "Scirocco", ELocation::kFrankfurt, kReligion, kCommerce, 12 },
  {"Elias", "Ostro", ELocation::kSaarbrucken, kReligion, kCommerce, 41 },
  {"Carmen", "Tramontane", ELocation::kLuxembourg, kCommerce, kMilitary, 56 },
  {"Jean", "Levante", ELocation::kCharleroi, kArt, kCommerce, 74 },  
};


Spy playerSpy;
Spy *bandit = nullptr;
Spy *victim = nullptr;
/*
const char *places[24] = {
  "Shoreline harbour", "Clergymen's tower", "Majestic Cafe",
  "Sodre harbour", "Belem's tower", "The Brazilian Lady Cafe",
  "Plaza mayor", "Almudena Cathedral", "Teatro de la Zarzuela ruins",
  "Port Vell", "Sagrada Familia church's contruction site", "Parc Guell",
  "Paulskirche", "Hauptwache", "The Romerberg",
  "Halberg Mithras shrine", "Ludwigskirche", "Saarkran",
  "Fort Thungen", "The Bockfiels", "Adolphe Bridge",
  "Parc du Cinquantenaire", "Universite Libre de Bruxelles", "Grote Markt" 
};
*/
char dossiers[8][ 40 * 25 ];


void getFileTextContents( int c ) {
  char filename[16];
  snprintf(&filename[0], 16, "res/%s.txt", suspects[ c ].name );

  FILE* fileInput = fopen( filename, "r");
  fread(&dossiers[c][0], 40 * 25, 1, fileInput );
  fclose(fileInput);
}

int getLocalHospitality() {
  return mercenaryCollaboration[ playerSpy.location];
}

void initMapGame() {
  int randomSeed = rand();
  bandit = &suspects[(randomSeed % 8)];
  victim = &suspects[(randomSeed + 1) % 8];
  bandit->target = victim;
  turnsToCatchBandit = 64; 
  //  printf("Killer is %s, while victim is %s\n", bandit->name, victim->name);
  bandit->target = victim;
  bandit->relationshipWithTarget = ERelationship::kWantsToKill;
  gotHim = false;
  turns = 0;
  playerSpy.location = playerSpy.destination = static_cast<ELocation>( ( randomSeed + 4 ) % 8 );
  //  printf("bandit starting at %s\n", placesNames[ bandit->location ] );
  playerSpy.distanceDue = 0;
  bandit->destination = static_cast<ELocation>((static_cast<int>(bandit->location) + 1) % 8);
  bandit->distanceDue = 15;
  bandit->turnsToStay = 15;

  for ( int c = 0; c < 8; ++c ) {
    mercenaryCollaboration[c] = 8 + (rand() % 8);
    suspects[c].target = nullptr;
    suspects[c].giveClue = false;
    suspects[c].alive = true;    
    suspects[c].relationshipWithTarget = ERelationship::kClaimsToKnowNothing;
    getFileTextContents( c );
  }

  auto knowsLocation = &suspects[(randomSeed + 2) % 8];
  auto knowsIntention = &suspects[(randomSeed + 3) % 8];
  auto knowsInnocent = &suspects[(randomSeed + 4) % 8];
  auto knownToBeInnocent = &suspects[(randomSeed + 5) % 8];

  knowsLocation->relationshipWithTarget = ERelationship::kKnowsLocation;
  knowsLocation->target = victim;

  knowsIntention->relationshipWithTarget = ERelationship::kKnowsIntention;
  knowsIntention->target = bandit;

  knowsInnocent->relationshipWithTarget = ERelationship::kKnowsItsInnocent;
  knowsInnocent->target = knownToBeInnocent;

  memset( trail, 16, 8 * sizeof(int8_t));
}

void update() {
  
  if ( playerSpy.distanceDue > 0 ) {
    --playerSpy.distanceDue;
  }

  if ( playerSpy.distanceDue <= 0 ) {
    playerSpy.distanceDue = 0;
    playerSpy.location = playerSpy.destination;
  }  
  
  if (gotHim) {
    return;
  }
  
  --turnsToCatchBandit;
    
  if (turnsToCatchBandit < 0 ) {
    return;
  }
  
  if ( bandit->distanceDue > 0 ) {
    --bandit->distanceDue;
    
    if ( bandit->distanceDue <= 0 ) {
      bandit->turnsToStay = 16 -  mercenaryCollaboration[bandit->destination];   
    }   
    
  } else if ( bandit->turnsToStay >= 0 ) {

    --bandit->turnsToStay;
    //    printf("staying for %d\n", bandit->turnsToStay);
    
    if ( bandit->turnsToStay <= 0 ) {
      //      puts("bandit going for the kill!");
      bandit->location = bandit->destination;
      victim->alive = false;
      bandit->turnsToStay = 15;

      do {
	bandit->destination = static_cast<ELocation>(rand() % 8);	  
	victim = &suspects[bandit->destination];
      } while( bandit != victim  && !victim->alive);

      bandit->target = victim;
      bandit->distanceDue = (distances[bandit->location][bandit->destination] / trainSpeed) + 1;
    }
  }
  
  //  printf("bandit is at %s and going to %s in %d and staying for %d\n", placesNames[bandit->location], placesNames[bandit->destination], bandit->distanceDue, bandit->turnsToStay );
  
  for ( int c = 0; c < 8; ++c ) {
    if (c != bandit->location ) {
      ++trail[c];
    } 
  }
  
  trail[bandit->location] = 0;  
}

void getDossierText( int c, char *buffer, size_t size ) {
  size_t bufferUsage = 0;

  bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "Name: %s\n", suspects[ c ].name );

  bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "Current operation: %s\n", suspects[ c ].codename );	  
  
  bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "Location: %s\n", placesNames[ c ] );	  

  bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "Bio:\n%s\n", dossiers[c] );	  
  
  if (suspects[c].giveClue) {
    bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "Intel:\n" );	  
    switch(suspects[c].relationshipWithTarget) {
    case kNone:	     
    case kWantsToKill:
    case kClaimsToKnowNothing:
      bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "\"I have no idea what are you talking \nabout\"" );	  
      break;
    case kKnowsIntention:
      bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "\"The murderer must be %s\"", bandit->name );
      break;
    case kKnowsItsInnocent:
      bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "\"What I do know is that %s\n is innocent.\"", suspects[c].target->name );
      break;
    case kKnowsLocation:
      bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "\"For sure, %s must be the target!\"", victim->name );
      break;
    }
  }
}

void getDisplayStatusText( char *buffer, size_t size ) {

  if (bandit == nullptr || victim == nullptr ) {
    return;
  }						 
 
  size_t bufferUsage = 0;
  
  bufferUsage += snprintf( buffer + bufferUsage, size - bufferUsage, "Turns left: %d\n", turnsToCatchBandit);
  
  bufferUsage += snprintf( buffer + bufferUsage, size - bufferUsage,"Your location: %s\n", placesNames[playerSpy.location]);

  bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "Local hospitality: %d%%\n", (100 * mercenaryCollaboration[playerSpy.location]) / 16);

  if ( (bandit->destination == playerSpy.destination) && (playerSpy.distanceDue < 1) && (bandit->distanceDue < 1) ) {
    bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "Bandit is in town!\n");
  }

  if (turnsToCatchBandit < 0 ) {
    bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "Bandit won. You're dead.\n");
    return;
  }

  
  if (gotHim) {
    bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "You got him!\n");  
  } else {
    bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "Trail: %d turn%sago\n", trail[playerSpy.location],
			    trail[playerSpy.location] == 1? " " : "s " );      
  }

  bool wroteDeadAgents = false;
  
  for ( int c = 0; c < 8; ++c ) {

    if (!suspects[c].alive) {
      if (!wroteDeadAgents) {
	wroteDeadAgents = true;
	bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "Dead agents:\n");
      }

      bufferUsage += snprintf(buffer + bufferUsage, size - bufferUsage, "- %s\n", suspects[c].name);
    }
  }
}



void displayStatus() {
  
  char buffer[ 80 * 25 ];

  memset( buffer, 0, 25 * 80 );

  getDisplayStatusText( buffer, 80 * 25 );
}

void goTo( ELocation newLocation ) {
  playerSpy.destination = newLocation;
  playerSpy.distanceDue = (distances[playerSpy.location][playerSpy.destination] / trainSpeed) + 1;
}

int mapGameTick(int input) {

  if (turnsToCatchBandit < 0 ) {
    return 0;
  }
  
  if ( input >= 0 && input < 8 ) {
    
    ELocation newLocation = static_cast<ELocation>(input);
    
    if (newLocation != playerSpy.location ) {
      goTo(newLocation);
    }

    turns = 0;
    
    do {
      update();
      ++turns;
    } while ( playerSpy.distanceDue > 0 );    
  }
 
  return 0;
}



bool isBanditPresent() {
  return (bandit->destination == playerSpy.destination) && (playerSpy.distanceDue < 1) && (bandit->distanceDue < 1);
}


bool isSuspectPresent() {
  return suspects[playerSpy.location].alive;
}

void getClue() {
  if (turnsToCatchBandit < 0 ) {
    return;
  }

  if ( isBanditPresent() ) {
    gotHim = true;
    return;
  } else if ( suspects[playerSpy.location].alive) {
      suspects[playerSpy.location].giveClue = true;
  }  

  update();
  ++turns;
}
