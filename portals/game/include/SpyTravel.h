#ifndef SPYTRAVEL_H
#define SPYTRAVEL_H

void initMapGame();
int mapGameTick(int move);
void getDisplayStatusText( char *buffer, size_t size );
void getDossierText( int suspect, char *buffer, size_t size );
bool isBanditPresent();
bool isSuspectPresent();
void getClue();
int getLocalHospitality();

enum ELocation{
  kPorto,
  kLisbon,
  kMadrid,
  kBarcelona,
  kFrankfurt,
  kSaarbrucken,
  kLuxembourg,
  kCharleroi
};



enum EInterest {
  kCommerce,
  kReligion,
  kIntellectual,
  kMilitary,
  kArt,
  kNature
};

enum ERelationship {
  kNone,
  kWantsToKill,
  kKnowsIntention,
  kKnowsItsInnocent,
  kKnowsLocation,
  kClaimsToKnowNothing
};
/*
const EInterest placeInterests[24] = {
  kCommerce, kReligion, kIntellectual,
  kCommerce, kMilitary, kIntellectual,
  kReligion, kReligion, kArt,
  kCommerce, kReligion, kArt,
  kReligion, kIntellectual, kArt,
  kReligion, kReligion, kCommerce,
  kMilitary, kMilitary, kCommerce,
  kArt, kIntellectual, kCommerce
  };*/

struct Spy {
  const char* name;
  const char* codename;  
  ELocation origin;
  EInterest mainInterest;
  EInterest secundaryInterest;
  uint8_t friendlyness;
  ELocation location;
  ELocation destination;
  int distanceDue = 1;
  int turnsToStay = 16;
  Spy* target = nullptr;
  ERelationship relationshipWithTarget = ERelationship::kNone;
  //  uint8_t place = 0;
  bool alive = true;
  bool giveClue = false;
};




extern const char* placesNames[8];
extern Spy suspects[8];
extern int turnsToCatchBandit;
#endif
