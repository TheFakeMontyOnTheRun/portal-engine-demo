
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <cmath>
#include <algorithm>
#include <string.h>

#include "FixP.h"
#include "3D.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "Graphics.h"
#include "Vec2i.h"
#include "VisibilityStrategy.h"
#include "SpyTravel.h"

const auto kAgentsInBase = 16;

struct CrawlerAgent {
  Vec2i position = {0, 0 };
  EDirection direction = EDirection::kNorth;
  uint8_t life = 10;
  Vec2i target = {0,0};
  uint8_t id = 0;
  uint8_t firePower = 1;
  uint8_t ammo = 16;
  bool seen = false;
  bool showMuzzleflash = false;
};

struct Room {
  Vec2i p0;
  Vec2i p1;
  Texture *texture;
  int link[6];
  int height0;
  int height1;
  uint8_t lastRenderedFrame = 255;
};

CrawlerAgent enemies[kAgentsInBase];

int16_t enemiesScreenOffsets[kAgentsInBase];

FixP heightBias = FixP{4} / FixP{10};
CrawlerAgent playerCrawler;
VisMap visMap;
DistanceDistribution distancesDistribution;
bool playerHasClue = false;
uint8_t enemyBaseMap[ kMapSize ][ kMapSize ];
uint8_t enemiesInBase[ kMapSize ][ kMapSize ];
bool seenEnemyBase[ kMapSize ][ kMapSize ];

Texture *wallTexture = NULL;
Texture *doorOpenTexture = NULL;
Texture *doorClosedTexture = NULL;
Texture *tableTexture = NULL;
Texture *clueTexture = NULL;
Texture *noClueTexture = NULL;
Texture *floorTexture = NULL;
Texture *ceilingTexture = NULL;
Texture *targetTexture = NULL;

NativeBitmap *portableBitmap = NULL;
NativeBitmap *enemySprite0 = NULL;
NativeBitmap *enemySprite1 = NULL;
NativeBitmap *pistolBitmap = NULL;
NativeBitmap *pistolFiringBitmap = NULL;
NativeBitmap *muzzleFlashBitmap = NULL;


P3D camera{ FixP{-1}, FixP{0}, FixP{-5}};
Room* rooms;
int numRooms = 7;
int playerRoom = 1;

const uint8_t kDrawFaceFront =  0b00000001;
const uint8_t kDrawFaceRight = 0b00000010;
const uint8_t kDrawFaceBack = 0b00000100;
const uint8_t kDrawFaceLeft =  0b00001000;
const uint8_t kDrawFaceDown =  0b00010000;
const uint8_t kDrawFaceUp =    0b00100000;

bool practiceCrawling = false;
int crawlerTurn = 0;
int timeToCloseDoor = 4;
bool cachedVisibility = false;
bool needRedrawView = true;
int gunSpeedY = 0;
int gunPositionY = 64;
int gunPositionX = 0;
uint8_t currentTarget = 0;
uint8_t crawlerFrame = 0;

void hideGun() {
  gunSpeedY = 1;
}

void showGun() {
  gunSpeedY = -4;
}
void toggleGun() {
  if ( gunPositionY == 64 ) {
    hideGun();
  }

  if ( gunPositionY == 84 ) {
    showGun();
  }  
}

Vec2i findRandomValidPosition() {
  int tries = 10;
  do {
    int8_t x = (rand() % (kMapSize - 2)) + 1;
    int8_t y = (rand() % (kMapSize - 2)) + 1;
    if ( enemyBaseMap[y][x] == 0 && enemiesInBase[y][x] == 0) {
      return Vec2i{ x, y};
    }
  } while( --tries >= 0 );

  return Vec2i{0, 0};
}

void addRoom( int8_t x0, int8_t y0, int8_t x1, int8_t y1 ) {

  for ( int y = y0; y < y1; ++y ) {
    for ( int x = x0; x < x1; ++x ) {
      enemyBaseMap[y][x] = 0;
    }
  }


  Room room{Vec2i{x0, static_cast<int8_t>(y0)},  Vec2i{x1, static_cast<int8_t>(y1)}, ((numRooms % 2) == 0) ? noClueTexture : tableTexture,  {0,0,0,0,0,0},  1, 3};      

  numRooms++;
  rooms = (Room*)realloc(rooms, sizeof(Room) * numRooms );
  memcpy( &rooms[numRooms -1 ], &room, sizeof(Room));  
}

bool compilerPass() {
  int xSlab;
  int ySlab;
  //  int zSlab;

  for (int c = 1; c < numRooms; ++c ) {
    Room *roomC = &rooms[c];

    ySlab = roomC->p0.y;

    for (int d = 1; d < numRooms; ++d ) {

      if ( c == d ) {
	continue;
      }
      
      Room *roomD = &rooms[d];
      
      if ( roomD->p0.y < ySlab && ySlab < roomD->p1.y ) {
	addRoom( roomD->p0.x, ySlab, roomD->p1.x, roomD->p1.y );
	roomD->p1.y = ySlab;	
	return false;
      }      
    }

    
    
    ySlab = roomC->p1.y;

    for (int d = 1; d < numRooms; ++d ) {
      Room *roomD = &rooms[d];
      
      if ( roomD->p0.y < ySlab && ySlab < roomD->p1.y ) {
	addRoom( roomD->p0.x, ySlab, roomD->p1.x, roomD->p1.y );
	roomD->p1.y = ySlab;	
	return false;
      }      
    }

    xSlab = roomC->p0.x;

    for (int d = 1; d < numRooms; ++d ) {
      Room *roomD = &rooms[d];
      
      if ( roomD->p0.x < xSlab && xSlab < roomD->p1.x ) {
	addRoom( xSlab, roomD->p0.y, roomD->p1.x, roomD->p1.y );
	roomD->p1.x = xSlab;	
	return false;
      }      
    }

    xSlab = roomC->p1.x;

    for (int d = 1; d < numRooms; ++d ) {
      Room *roomD = &rooms[d];
      
      if ( roomD->p0.x < xSlab && xSlab < roomD->p1.x ) {
	addRoom( xSlab, roomD->p0.y, roomD->p1.x, roomD->p1.y );
	roomD->p1.x = xSlab;	
	return false;
      }      
    } 

    

    
    /*

    zSlab = room->h0;

    for (int d = 1; d < numRooms; ++d ) {
      Room *roomD = rooms[d];
      
      if ( roomD->p0.x < xSlab && xSlab < roomD->p1.x ) {
	roomD->p1.x = xSlab;
	addRoom( xSlab, roomD->p0.y, roomD->p1.x, roomD->p1.y );
	return false;
      }      
    }        
    */


  }

  return true;
}

void relinkRooms() {

  int xSlab;
  int ySlab;
  //  int zSlab;

  for (int c = 1; c < numRooms; ++c ) {
    Room *roomC = &rooms[c];

    ySlab = roomC->p0.y;

    for (int d = 1; d < numRooms; ++d ) {

      if ( c == d ) {
	continue;
      }
      
      Room *roomD = &rooms[d];
      
      if ( roomD->p0.x == roomC->p0.x && roomD->p1.x == roomC->p1.x  && ySlab == roomD->p1.y ) {
	roomC->link[0] = d;
	roomD->link[2] = c;	
      }      
    }

    ySlab = roomC->p1.y;

    for (int d = 1; d < numRooms; ++d ) {

      if ( c == d ) {
	continue;
      }
      
      Room *roomD = &rooms[d];
      
      if ( roomD->p0.x == roomC->p0.x && roomD->p1.x == roomC->p1.x  && ySlab == roomD->p0.y ) {
	roomC->link[2] = d;
	roomD->link[0] = c;
      }      
    }    
    
    
    xSlab = roomC->p0.x;

    for (int d = 1; d < numRooms; ++d ) {

      if ( c == d ) {
	continue;
      }
      
      Room *roomD = &rooms[d];
      
      if ( roomD->p0.y == roomC->p0.y && roomD->p1.y == roomC->p1.y  && xSlab == roomD->p1.x ) {
	roomC->link[3] = d;
	roomD->link[1] = c;
      }      
    }    
    

    xSlab = roomC->p1.x;

    for (int d = 1; d < numRooms; ++d ) {

      if ( c == d ) {
	continue;
      }
      
      Room *roomD = &rooms[d];
      
      if ( roomD->p0.y == roomC->p0.y && roomD->p1.y == roomC->p1.y  && ySlab == roomD->p0.x ) {
	roomC->link[1] = d;
	roomD->link[3] = c;
      }
      
    }        
  }
}

void compileMap() {
  while (!compilerPass());

  relinkRooms();
}

int32_t Crawler_initStateCallback(int32_t tag, void* data) {

  practiceCrawling = ( tag == kPracticeCrawling );
  cachedVisibility = false;

  wallTexture = makeTextureFrom(loadBitmap("res/wall.img"));
  floorTexture = makeTextureFrom(loadBitmap("res/ceiling.img"));
  ceilingTexture = makeTextureFrom(loadBitmap("res/floor.img"));
  targetTexture = makeTextureFrom(loadBitmap("res/target.img"));  
  tableTexture = makeTextureFrom(loadBitmap("res/table.img"));    
  doorOpenTexture = makeTextureFrom(loadBitmap("res/dooro.img"));
  doorClosedTexture = makeTextureFrom(loadBitmap("res/doorc.img"));        
  clueTexture = makeTextureFrom(loadBitmap("res/clue.img"));
  noClueTexture = makeTextureFrom(loadBitmap("res/noclue.img"));  

  pistolBitmap = loadBitmap("res/pistol.img");
  pistolFiringBitmap = loadBitmap("res/pistolf.img");
  portableBitmap = loadBitmap("res/portable.img");
  enemySprite0 = loadBitmap("res/enemy0.img");
  enemySprite1 = loadBitmap("res/enemy1.img");

  muzzleFlashBitmap = loadBitmap("res/muzzle.img");  

  memset( enemyBaseMap, 1, kMapSize * kMapSize );
  memset( enemiesInBase, 0, kMapSize * kMapSize );
  memset( seenEnemyBase, 0, kMapSize * kMapSize );  

  numRooms = 7;
  rooms = (Room*)malloc(sizeof(Room) * numRooms );  
  Room room0{Vec2i{-128, -128}, Vec2i{127, 127}, tableTexture, {0,0,0,0,0,0}, -128, 128};
  memcpy( &rooms[0], &room0, sizeof(Room));
  


  Room room1{Vec2i{-2, 14},  Vec2i{3, 9}, tableTexture,  {2,0,0,0,0,0}, -1, 1};
  Room room2{Vec2i{-2, 15},  Vec2i{3, 14}, noClueTexture, {3,4,1,0,0,0}, -1, 1};
  Room room3{Vec2i{-2, 16},  Vec2i{3, 15}, tableTexture , {0,0,2,5,0,0}, -1, 1};
  Room room4{Vec2i{ 3, 15},  Vec2i{4, 14}, clueTexture,   {0,0,0,2,0,0}, -1, 1};
  Room room5{Vec2i{-4, 16}, Vec2i{-2, 15}, clueTexture,   {0,3,0,0,0,0}, -1, 1};
  Room room6{Vec2i{-2, 14},  Vec2i{3, 9}, tableTexture,  {0,0,6,0,1,0},  1, 3};  

  memcpy( &rooms[1], &room1, sizeof(Room));
  memcpy( &rooms[2], &room2, sizeof(Room));
  memcpy( &rooms[3], &room3, sizeof(Room));
  memcpy( &rooms[4], &room4, sizeof(Room));
  memcpy( &rooms[5], &room5, sizeof(Room));
  memcpy( &rooms[6], &room6, sizeof(Room));          

  
  
  auto ptr = &enemyBaseMap[0][0];

  for ( auto y = 1; y < kMapSize -1; ++y ) {

    for ( auto x = 1; x < kMapSize -1 ; ++x ) {
      auto pos = &ptr[ (y * kMapSize) + x];

      *pos = 0;
           
      if ( (x == 16 || y == 16) ) {
	*pos = 1;  
      }
      
      if ( (x == 8 && y == 16) ) {
	*pos = 4;  
      }

      if ( (x == 24 && y == 16) ) {
	*pos = 4;  
      }

      if ( (x == 16 && y == 8) ) {
	*pos = 4;  
      }

      if ( (x == 16 && y == 24) ) {
	*pos = 4;  
      }

    }
    }
  /*
  //  addRoom(3, 3, 10, 17 );
  addRoom(5, 17, 8, 20 );  
  addRoom(2, 20, 28, 27 );
  addRoom(20, 27, 28, 30 );  
  
  */
  /*
  addRoom(3, 3, 10, 12 );
  addRoom(5, 12, 8, 15 );  
  addRoom(2, 15, 28, 30 );

  addRoom(20, 2, 23, 15 );
  addRoom(23, 6, 27, 9 );
  addRoom(27, 2, 30, 12 );        
  */
    compileMap();
  

  enemyBaseMap[3][3] = 1;


  Vec2i randomPos = findRandomValidPosition();
  
  enemyBaseMap[randomPos.y][randomPos.x] = 2;

  enemyBaseMap[13][3] = 1;
  
  auto agentsToSpawn = practiceCrawling ? kAgentsInBase - 1 : getLocalHospitality();

  for ( int c = 0; c < kAgentsInBase; ++c ) {
    
    auto p = &enemies[c];
    p->life = (agentsToSpawn >= 0) ? 10 : 0;
    p->ammo = 10;
    p->direction = kNorth;
    p->position = findRandomValidPosition();
    p->id = c + 1;
    p->seen = false;    
    p->showMuzzleflash = false;
    enemiesInBase[p->position.y][p->position.x] = p->id;
    agentsToSpawn--;
  }

  {
    auto p = &playerCrawler;
    p->life = 20;
    p->ammo = 20;
    p->direction = kNorth;

    auto randomPos = findRandomValidPosition();

//    camera.x = FixP{randomPos.x};
//    camera.z = FixP{31 - randomPos.y};
//    camera.y = FixP{(rooms[1].height0 + rooms[1].height1) / 2};

    enemyBaseMap[randomPos.y][randomPos.x] = 5;
    p->position = randomPos; 
    p->seen = false;
    p->showMuzzleflash = false;
  }

  cachedVisibility = false;  
  playerHasClue = false;
  crawlerTurn = 0;
  currentTarget = 0;
  hideGun();
  return 0;
}

void Crawler_initialPaintCallback(void) {
  graphicsFill( 256, 0, 64, 128, 0 );
  graphicsBlit( 0, 128, portableBitmap );

  graphicsDrawTextAt( 34, 1, "Map:", 128 );
}


void drawRoomAt( const P3D& camera, const Vec2i& v0, const Vec2i& v1, int h0, int h1 , uint8_t drawFacesFlags, Texture *texture) {

  FixP zero{0};
  auto kMinZCull{0};
  
  if (v1.y <= kMinZCull) {
    puts("behind frustum");    
    return;
  }
  
  const FixP _dx = FixP{v1.x - v0.x};
  const FixP _dy = FixP{h1 - h0};
  const FixP one{1};
  const FixP two{2};
  const FixP three{3};  
  FixP cameraToFarEnd = ( abs( (FixP{v0.y} - (camera.z ) ) ) );
  FixP sectorLength = FixP{abs(v1.y - v0.y)};
  const FixP _dz = min( sectorLength, cameraToFarEnd);

  if (_dz == zero ) {
    puts("slim room");
    return;
  }
  
  {
    const FixP x0 = FixP{v0.x} - camera.x;
    const FixP y0 = FixP{h0} - camera.y;
    const FixP z0 = FixP{v0.y} - camera.z - FixP{8};
    
    const FixP x1 = FixP{v1.x} - camera.x;
    const FixP y1 = FixP{h1} - camera.y;
    FixP z1 = FixP{v1.y} - camera.z - FixP{8};

    if ( z1 < zero ) {
      //      z1 = zero;
    }
 
    toProject[ 0 ] = P3D{ x0, y1, z1 };
    toProject[ 1 ] = P3D{ x0, y1, z0 };
    toProject[ 2 ] = P3D{ x1, y0, z0 };
    toProject[ 3 ] = P3D{ x1, y0, z1 };
  }

  projectPoints(4);
  
  const auto p1 = &projected[0];
  const auto p2 = &projected[1];
  const auto p3 = &projected[2];
  const auto p4 = &projected[3];
  

  
  if ( drawFacesFlags & kDrawFaceDown ) {
    
    graphicsDrawFloor(p4->y, p3->y,
		      p1->x, p4->x,
		      p2->x, p3->x,
		      texture->regular, _dx, _dz);
    
  }

  if ( drawFacesFlags & kDrawFaceUp ) {
    graphicsDrawFloor(p2->y, p1->y,
		      p2->x, p3->x,
		      p1->x, p4->x,
		      texture->regular, _dx, _dz);
    
  }  
  
  
  if ( drawFacesFlags & kDrawFaceRight) {

    graphicsDrawWall( p3->x, p4->x,
		      p2->y, p3->y,
		      p1->y, p4->y,
		      texture->rotated, _dz, _dy );		      

  }

  if ( drawFacesFlags & kDrawFaceLeft) {

    graphicsDrawWall(p1->x, p2->x,
		     p1->y, p4->y,
		     p2->y, p3->y,
		     texture->rotated, _dz, _dy);

  }

  if ( drawFacesFlags & kDrawFaceBack ) {

    graphicsDrawSprite( p1->x, p1->y,
			p4->x, p4->y,
			texture->regular,
			false, _dx, _dy);

  }

  
  
  if (drawFacesFlags & kDrawFaceFront ) {
    /*
    graphicsDrawSprite( p2->x, p2->y,
			p3->x, p3->y,
			texture->regular,
			false, _dx, _dy);
    */
  }
    
}



void drawCrateAt( int x, int y, int z, uint8_t drawFacesFlags, Texture *texture, bool useAlpha = false) {

  P3D center{ x, y, z };
  FixP zero{0};
  FixP kMinZCull{1};
  
  if (center.z <= kMinZCull) {
    return;
  }

  const FixP one{ 1 };
  
  toProject[ 0 ] = P3D{ center.x - one, center.y - heightBias - one, center.z + one };
  toProject[ 1 ] = P3D{ center.x - one, center.y - heightBias - one, center.z - one };
  toProject[ 2 ] = P3D{ center.x + one, center.y - heightBias + one, center.z - one };
  toProject[ 3 ] = P3D{ center.x + one, center.y - heightBias + one, center.z + one };

  projectPoints(4);
  
  const auto p1 = &projected[0];
  const auto p2 = &projected[1];
  const auto p3 = &projected[2];
  const auto p4 = &projected[3];

  if (static_cast<int>(p4->x) >= 256 ) {
    return;
  }

  if (static_cast<int>(p1->x) < 0) {
    return;	     
  }
  
  if ( ( drawFacesFlags & kDrawFaceRight ) && center.x <= zero ) {
    graphicsDrawWall( p3->x, p4->x,
		      p2->y, p3->y,
		      p1->y, p4->y,
		      texture->rotated );		      
  }

  if ( ( drawFacesFlags & kDrawFaceLeft) && center.x >= zero ) {
    graphicsDrawWall(p1->x, p2->x,
		     p1->y, p4->y,
		     p2->y, p3->y,
		     texture->rotated);
  }

  if ( drawFacesFlags & kDrawFaceFront ) {
    graphicsDrawSprite( p2->x, p2->y,
			p3->x, p3->y,
			texture->regular,
			useAlpha);
  }
}

bool drawEnemyAt( int x, int y, int z, NativeBitmap *texture, bool target, bool showMuzzleflash ) {

  P3D center{ x, y, z };
  FixP zero{0};
  FixP kMinZCull{1};
  
  if (center.z <= kMinZCull) {
    return false;
  }

  const FixP one{ 1 };
  
  toProject[ 0 ] = P3D{ center.x - one, center.y - one - heightBias, center.z };
  toProject[ 1 ] = P3D{ center.x + one, center.y + one - heightBias, center.z };

  projectPoints(2);
  
  const auto p1 = &projected[0];
  const auto p2 = &projected[1];


  
  graphicsDrawSprite( p1->x, p1->y,
		      p2->x, p2->y,
		      texture,
		      true);

  if (static_cast<int>(p1->x) >= 256 ) {
    return false;
  }

  if (static_cast<int>(p2->x) < 0) {
    return false;
  }
  
  if (showMuzzleflash) {
    graphicsDrawSprite( p1->x, p1->y,
			p2->x, p2->y,
			muzzleFlashBitmap,
			true);
  }

  if (target) {
    graphicsDrawSprite( p1->x, p1->y,
			p2->x, p2->y,
			targetTexture->regular,
			true);
  }


  return true;
}

void drawFloorAndCeilingAt( int x, int y, int z, Texture *floorTex, Texture *ceilingTex, int ceilHeight = 3 ) {
  
  P3D center{ x, y, z };
  FixP zero{0};
  FixP kMinZCull{1};
  
  if (center.z <= kMinZCull) {
    return;
  }

  const FixP one{ 1 };
  const FixP ceilingHeight{ ceilHeight };  
  
  toProject[ 0 ] = P3D{ center.x - one, center.y - one - heightBias, center.z - one };
  toProject[ 1 ] = P3D{ center.x - one, center.y - one - heightBias, center.z + one };
  toProject[ 2 ] = P3D{ center.x + one, center.y + ceilingHeight - heightBias, center.z + one };
  toProject[ 3 ] = P3D{ center.x + one, center.y + ceilingHeight - heightBias, center.z - one };

  projectPoints(4);
  
  const auto p1 = &projected[0];
  const auto p2 = &projected[1];
  const auto p3 = &projected[2];
  const auto p4 = &projected[3];

  if (static_cast<int>(p3->x) >= 256 ) {
    return;
  }

  if (static_cast<int>(p2->x) < 0) {
    return;	     
  }
  
  if ( ceilingTex != NULL && center.y >= zero ) {
    graphicsDrawFloor(p1->y, p2->y,
		      p1->x, p4->x,
		      p2->x, p3->x,
		      ceilingTex->regular);
  }

  if ( floorTex != NULL && center.y <= zero ) {
    graphicsDrawFloor(p3->y, p4->y,
		      p2->x, p3->x,
		      p1->x, p4->x,
		      floorTex->regular);
  }
}


void setClippingRectForLink(int roomNumber, int link, const P3D& camera) {

  if (roomNumber == playerRoom ) {
    graphicsSetClipRect( 0, 0, 256, 128 );
    return;
  }

  Room* room = &rooms[roomNumber];
  auto v0 = room->p0;
  auto v1 = room->p1;
  auto h0 = room->height0;
  auto h1 = room->height1;

  int16_t cx0 = 0;
  int16_t cy0 = 0;
  int16_t cx1 = 0;
  int16_t cy1 = 0;
  
  FixP zero{0};
  auto kMinZCull{0};
  
  if (v1.y <= kMinZCull) {
    return;
  }
  
  {

    const FixP x0 = FixP{v0.x} - camera.x;
    const FixP y0 = FixP{h0} - camera.y;
    const FixP z0 = FixP{v0.y} - camera.z - FixP{8};
    const FixP x1 = FixP{v1.x} - camera.x;
    const FixP y1 = FixP{h1} - camera.y;
    const FixP z1 = FixP{v1.y} - camera.z - FixP{8};
    toProject[ 0 ] = P3D{ x0, y1, z1 };
    toProject[ 1 ] = P3D{ x0, y1, z0 };
    toProject[ 2 ] = P3D{ x1, y0, z0 };
    toProject[ 3 ] = P3D{ x1, y0, z1 };
  }

  projectPoints(4);
  
  const auto p1 = &projected[0];
  const auto p2 = &projected[1];
  const auto p3 = &projected[2];
  const auto p4 = &projected[3];

  switch (link) {
  case 0: {

    cx0 = std::max<int>(0, std::min( static_cast<int16_t>(p1->x), static_cast<int16_t>(p4->x)));
    cy0 = std::max<int>(0, std::min( static_cast<int16_t>(p1->y), static_cast<int16_t>(p4->y)));
    cx1 = std::min<int>(256, std::max( static_cast<int16_t>(p1->x), static_cast<int16_t>(p4->x)));
    cy1 = std::min<int>(128, std::max( static_cast<int16_t>(p1->y), static_cast<int16_t>(p4->y)));
  }
    break;
    
  case 5: {

    cx0 = std::max<int>(0, std::min( static_cast<int16_t>(p2->x), static_cast<int16_t>(p3->x))); //
    cy0 = std::min<int>(0, std::min( static_cast<int16_t>(p1->y), static_cast<int16_t>(p2->y))); //
    cx1 = std::min<int>(256, std::max( static_cast<int16_t>(p2->x), static_cast<int16_t>(p3->x))); //
    cy1 = std::max<int>(128, std::max( static_cast<int16_t>(p1->y), static_cast<int16_t>(p2->y))); //
    
  }
    break;
    
  case 4: {

    cx0 = std::max<int>(0, std::min( static_cast<int16_t>(p2->x), static_cast<int16_t>(p3->x))); //
    cy0 = std::min<int>(0, std::min( static_cast<int16_t>(p4->y), static_cast<int16_t>(p3->y))); //
    cx1 = std::min<int>(256, std::max( static_cast<int16_t>(p2->x), static_cast<int16_t>(p3->x))); //
    cy1 = std::max<int>(128, std::max( static_cast<int16_t>(p4->y), static_cast<int16_t>(p3->y))); //

  }  
    break;
    
  case 1: {

    cx0 = std::max<int>(0, std::min( static_cast<int16_t>(p3->x), static_cast<int16_t>(p4->x)));
    cy0 = std::max<int>(0, std::min( static_cast<int16_t>(p3->y), static_cast<int16_t>(p2->y)));
    cx1 = std::min<int>(256, std::max( static_cast<int16_t>(p3->x), static_cast<int16_t>(p4->x)));
    cy1 = std::min<int>(128, std::max( static_cast<int16_t>(p3->y), static_cast<int16_t>(p2->y)));

  }
    break;
    
  case 3: {
    cx0 = std::max<int>(0, std::min( static_cast<int16_t>(p2->x), static_cast<int16_t>(p1->x)));
    cy0 = std::max<int>(0, std::min( static_cast<int16_t>(p2->y), static_cast<int16_t>(p3->y)));
    cx1 = std::min<int>(256, std::max( static_cast<int16_t>(p2->x), static_cast<int16_t>(p1->x)));
    cy1 = std::min<int>(128, std::max( static_cast<int16_t>(p2->y), static_cast<int16_t>(p3->y)));
  }
    break;
    
    
  case 2: {
    cx0 = std::max<int>(0, std::min( static_cast<int16_t>(p2->x), static_cast<int16_t>(p3->x)));
    cy0 = std::max<int>(0, std::min( static_cast<int16_t>(p2->y), static_cast<int16_t>(p3->y)));
    cx1 = std::min<int>(256, std::max( static_cast<int16_t>(p2->x), static_cast<int16_t>(p3->x)));
    cy1 = std::min<int>(128, std::max( static_cast<int16_t>(p2->y), static_cast<int16_t>(p3->y)));
  }
    break;
  }

  graphicsEncloseClipRect( cx0, cy0, cx1, cy1 );
}

void renderRooms(int roomNumber, int fromLink, const P3D& camera) {

  Room* room = &rooms[roomNumber];

  if (room->lastRenderedFrame == crawlerFrame ) {
    return;
  }

  auto flags = 0xFF;

  room->lastRenderedFrame = crawlerFrame;
  
  for (int link = 0; link < 6; ++link ) {
    if (room->link[link]) {
      flags = flags & ~(1 << link );
    }
  }

  setClippingRectForLink(roomNumber,
			 oppositeOf(static_cast<EDirection>(fromLink)),
			 camera
			 );

  if (clippingRect.x0 == clippingRect.x1 || clippingRect.y0 == clippingRect.y1 ) {
    puts("null portal");
    return;
  }
  
  auto clipRect = graphicsGetCurrentClipRect();
  if (roomNumber == playerRoom ) {
    flags = flags & ~kDrawFaceFront;
  }  
  
  drawRoomAt(camera,
	     room->p0,
	     room->p1,
	     room->height0,
	     room->height1,
	     flags, room->texture );


//  graphicsFill( 256 + (  2* room->p0.x ),
//		2* ( room->p0.y ),
//		2* ( room->p1.x - room->p0.x ),
//		2* ( room->p1.y - room->p0.y ),
//		(roomNumber == playerRoom ) ? 255 : 128
//		);

  for (int link = 0; link < 6; ++link ) {
    
    if (link == 0) {
      continue;
    }
    
    auto roomLinked = room->link[link];
    
    if (roomLinked) {

      if (rooms[roomLinked].lastRenderedFrame == crawlerFrame ) {
	continue;
      }
      
      renderRooms(roomLinked, link, camera);
      graphicsSetClipRect( clipRect );
      
    }
  }
}



void Crawler_repaintCallback(void) {

  ++crawlerFrame;
	graphicsFill( 0, 0, 320, 200, 255 );
  graphicsSetClipRect( 0, 0, 256, 128 );

	renderRooms( playerRoom, 2, camera );
  return;

  if (!cachedVisibility) {
    visibilityCastVisibility(visMap, enemyBaseMap, playerCrawler.position, playerCrawler.direction, true, distancesDistribution);
    needRedrawView = true;
    cachedVisibility = true;
  }

  if (gunSpeedY != 0 ) {
    gunPositionY += gunSpeedY;
    needRedrawView = true;
  }
  
  bool shotsFired = false;
  
  if (needRedrawView) {    
    for ( int c = 0; c < kAgentsInBase; ++c ) {
      auto p = &enemies[c];
      p->seen = false;    
    }
    
    uint8_t flags = 0;
    int tile;
    
    for ( auto dist = ( kMapSize + kMapSize - 1); dist >= 0; --dist ) {
      
      auto line = &distancesDistribution[dist][0];
      
      for (int x = 0; x < kMapSize; ++x ) {
	
	if ( line->x == INT8_MIN || line->y == INT8_MIN ) {
	  x = kMapSize;
	  continue;
	}
	
	flags = 0;
	Vec2i transformed;
	seenEnemyBase[line->y][line->x] = true;
	switch ( playerCrawler.direction ) {
	case kUp:
	case kDown:
	case kNorth:
	  transformed.x = line->x - playerCrawler.position.x;
	  transformed.y = line->y - playerCrawler.position.y;
	  
	  tile = (enemyBaseMap[line->y][line->x + 1]);
	  
	  if ( line->x < kMapSize && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) {
	    flags += kDrawFaceRight;
	  }
	  
	  tile = enemyBaseMap[line->y][line->x - 1];
	  
	  if ( line->x > 0 && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) {
	    flags += kDrawFaceLeft;
	  }
	  
	  tile = enemyBaseMap[line->y + 1][line->x];
	  
	  if ( line->y < kMapSize && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) {
	    flags += kDrawFaceFront;
	  }	
	  
	  break;
	case kSouth:
	  transformed.x = (- line->x + playerCrawler.position.x);
	  transformed.y = (- line->y + playerCrawler.position.y);	
	  
	  tile = enemyBaseMap[line->y][line->x + 1];
	  
	  if ( line->x < kMapSize && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) {
	    flags += kDrawFaceLeft;
	  }
	  
	  
	  tile = enemyBaseMap[line->y][line->x - 1];
	  
	  if ( line->x > 0 && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) {
	    flags += kDrawFaceRight;
	  }
	  
	  tile = enemyBaseMap[line->y - 1][line->x];
	  
	  if ( line->y > 0 && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) { 
	    flags += kDrawFaceFront;
	  }	
	  
	  break;	
	case kWest:
	  transformed.x = (- line->y + playerCrawler.position.y);
	  transformed.y = (line->x - playerCrawler.position.x);		
	  
	  tile = enemyBaseMap[line->y - 1][line->x];
	  
	  if ( line->y > 0 && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) { 
	    flags += kDrawFaceRight;
	  }
	  
	  tile = enemyBaseMap[line->y + 1][line->x];
	  
	  if ( line->y < kMapSize && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) {
	    flags += kDrawFaceLeft;
	  }
	  
	  tile = enemyBaseMap[line->y][line->x + 1];
	  
	  if ( line->x < kMapSize && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) {
	    flags += kDrawFaceFront;
	  }	
	  
	  break;
	case kEast:
	  transformed.x = (line->y - playerCrawler.position.y);
	  transformed.y = (- line->x + playerCrawler.position.x);		
	  
	  tile = enemyBaseMap[line->y + 1][line->x];
	  
	  if ( line->x < kMapSize && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) {
	    flags += kDrawFaceRight;
	  }
	  
	  tile = enemyBaseMap[line->y - 1][line->x];
	  
	  if ( line->x > 0 && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) {
	    flags += kDrawFaceLeft;
	  }
	  
	  tile = enemyBaseMap[line->y][line->x - 1];
	  
	  if ( line->y > 0  && ( tile == 0 || tile == 3 || tile == 5 || tile == 2) ) {
	    flags += kDrawFaceFront;
	  }	
	  
	  break;	
	}
	
	auto tile = enemyBaseMap[line->y][line->x];
	
	if ( tile == 1 || tile == 4 || tile == 3) {
	  
	  Texture* texture = nullptr;
	  bool alpha = false;
	  switch (tile) {
	  case 1:
	    texture = wallTexture;
	    break;
	  case 3:
	    drawFloorAndCeilingAt( 2 * transformed.x , 0, -2 * transformed.y, floorTexture, ceilingTexture );	  
	    texture = doorOpenTexture;
	    alpha  = true;
	    break;
	  case 4:
	    texture = doorClosedTexture;
	    break;
	  }
	  
	  drawCrateAt( 2 * transformed.x , 2, -2 * transformed.y, flags, wallTexture);	  
	  drawCrateAt( 2 * transformed.x , 0, -2 * transformed.y, flags, texture, alpha);
	} else if ( tile == 2 ) {
	  drawCrateAt( 2 * transformed.x , -1, -2 * transformed.y, flags, tableTexture);	
	  drawCrateAt( 2 * transformed.x , 3, -2 * transformed.y, flags, tableTexture);  
	  drawFloorAndCeilingAt( 2 * transformed.x , 1, -2 * transformed.y, NULL, playerHasClue ? noClueTexture : clueTexture, 0 );
	  drawFloorAndCeilingAt( 2 * transformed.x , 0, -2 * transformed.y, tableTexture, NULL, 2 );
	} else {
	  drawFloorAndCeilingAt( 2 * transformed.x , 0, -2 * transformed.y, floorTexture, tile == 5 ? tableTexture : ceilingTexture );

	  auto enemyId = enemiesInBase[line->y][line->x];

	  if (enemyId) {
	    auto enemy = &enemies[enemyId -1 ];

	    if ( enemy->life > 0 ) {
	      enemiesScreenOffsets[enemyId-1] = transformed.x;
	      enemy->seen = drawEnemyAt( 2 * transformed.x , 0, -2 * transformed.y, ((crawlerTurn + enemyId) & 1) ? enemySprite0 : enemySprite1, enemyId == currentTarget, enemy->showMuzzleflash );

	      if (enemy->seen) {

		if (enemy->showMuzzleflash) {
		  shotsFired = true;
		}		
	      }

	      enemy->showMuzzleflash = false;
	    }
	  }
	}
	
	line++;
      }
    }
    
    if ( !enemies[currentTarget -1 ].seen ) {
      currentTarget = 0;
      hideGun();
    }   

    graphicsFill(256, 16, 64, 64, 0);

    for ( int y = 0; y < 32; ++y ) {
      for ( int x = 0; x < 32; ++x ) {
	auto tile = (enemyBaseMap[y][x] );
	
	if (seenEnemyBase[y][x] && ( tile == 1 || tile == 4 || tile == 2 )) {
	  graphicsFill( 256 + (2 * x), 16 + (2 * y), 2, 2, 128 );
	}

	auto id = enemiesInBase[y][x];
	if (seenEnemyBase[y][x] && ( id != 0 ) ) {
	  
	  graphicsFill( 256 + (2 * x), 16 + (2 * y), 2, 2, ( id == currentTarget ) ? 64 : 255 );
	  
	}	


      }
    }


    graphicsFill( 256 + (2 * playerCrawler.position.x), 16 + (2 * playerCrawler.position.y), 2, 2, 37 );
    
  }
  clippingRect.y1 = 128;

  if ( gunSpeedY > 0 && gunPositionY >= 84) {
    gunSpeedY = 0;
  } else if ( gunSpeedY < 0 && gunPositionY <= 64 ) {
    gunSpeedY = 0;
  }


  graphicsFill( 40, 140, 214, 50, 37 );

  char healthBuffer[16];

  snprintf(healthBuffer, 16, "HP: %u", playerCrawler.life);
  graphicsDrawTextAt( 7, 19, healthBuffer, 0 );

  snprintf(healthBuffer, 16, "Ammo: %u", playerCrawler.ammo);
  graphicsDrawTextAt( 7, 20, healthBuffer, 0 );

  graphicsDrawTextAt( 7, 21, "Plastic handgun.", 0 );

  if (playerHasClue) {
    graphicsDrawTextAt( 7, 22, "Got the secret document.", 0 );
  }

  
  
  if (playerCrawler.showMuzzleflash) {
    shotsFired = true;
    playerCrawler.showMuzzleflash = false;

    //why the strange numbers? Dunno. No time to figure out.
    graphicsDrawSprite( gunPositionX + pistolFiringBitmap->width,
			gunPositionY - 32,
			gunPositionX + pistolFiringBitmap->width + 64,
			gunPositionY + 64 - 32,
			muzzleFlashBitmap,
			true);
  }
  
  graphicsBlit( 32 + gunPositionX, gunPositionY, playerCrawler.showMuzzleflash ? pistolFiringBitmap : pistolBitmap );  

  clippingRect.y1 = 200;
  
  needRedrawView = false;

  if (shotsFired) {
    needRedrawView = true;
  }

  graphicsSetClipRect( 0, 0, 320, 200 );
}


int32_t Crawler_success() {
  if (practiceCrawling) {
    return kMainMenu;
  } else {
    
    if (playerCrawler.life > 0 ) {      
      getClue();
    } else {
      turnsToCatchBandit = -1;
    }
    return kPlayGame;
  }
  
  return -1;
}

void pursueSpy( CrawlerAgent* spy, const Vec2i& target ) {

  int dx = (spy->position.x - target.x);
  int dy = (spy->position.y - target.y);
  FixP x{spy->position.x};
  FixP y{spy->position.y};    
  FixP one{1};
  FixP zero{0};
  FixP incX;
  FixP incY;

  if ( std::abs(dx) >= std::abs(dy) ) {
    FixP inc = zero;

    if (dx != 0 ) {
      inc = FixP{-dy} / FixP{dx};
    }
    
    incY = inc;

    if ( dx >= 0 ) {
      incX = -one;
    } else {
      incX = one;
    }
  } else {

    FixP inc = zero;

    if (dy != 0) {
      inc = FixP{-dx} / FixP{dy};
    }
    
    incX = inc;

    if ( dy >= 0 ) {
      incY = -one;
    } else {
      incY = one;
    }
  }

  auto iX = static_cast<int>(x + incX);
  auto iY = static_cast<int>(y + incY);
  int tile = enemyBaseMap[iY][iX];
  int id = enemiesInBase[iY][iX];

  if ( (tile == 0 || tile == 3 ) && id == 0 && (iX != playerCrawler.position.x && iY != playerCrawler.position.y) ) {
    spy->position = Vec2i{static_cast<int8_t>(iX), static_cast<int8_t>(iY)};
  }
}

bool canSeeSpy( const CrawlerAgent* seer, const CrawlerAgent *target ) {

  if (seer->seen) {
    return true;
  }

  int dx = (seer->position.x - target->position.x);
  int dy = (seer->position.y - target->position.y);
  FixP x{seer->position.x};
  FixP y{seer->position.y};    
  FixP one{1};
  FixP zero{0};
  FixP incX;
  FixP incY;

  if ( std::abs(dx) >= std::abs(dy) ) {
    FixP inc = zero;

    if (dx != 0 ) {
      inc = FixP{-dy} / FixP{dx};
    }
    
    incY = inc;

    if ( dx >= 0 ) {
      incX = -one;
    } else {
      incX = one;
    }
  } else {

    FixP inc = zero;

    if (dy != 0) {
      inc = FixP{-dx} / FixP{dy};
    }
    
    incX = inc;

    if ( dy >= 0 ) {
      incY = -one;
    } else {
      incY = one;
    }

  }

  int iX;
  int iY;

  iX = static_cast<int>(x);
  iY = static_cast<int>(y);

  int tile;

  tile = enemyBaseMap[iY][iX];
    
  do {
    x += incX;
    y += incY;

    iX = static_cast<int>(x);
    iY = static_cast<int>(y);
    tile = enemyBaseMap[iY][iX];
  } while (( tile == 0 || tile == 3 ) && ((std::abs(iX - target->position.x) > 1) || (std::abs(iY - target->position.y) > 1)));


  return ( ( tile == 0 || tile == 3 ) && (std::abs(iX - target->position.x) <= 1) && (std::abs(iY - target->position.y) <= 1 ) );
}

void shootSpy( CrawlerAgent* shooter, CrawlerAgent *target ) {

  if (shooter->ammo > 0 ) {
    target->life -= shooter->firePower;
    shooter->ammo--;    
  }
}

void Crawler_tickTurn() {
  
  ++crawlerTurn;
  cachedVisibility = false;
  
  if (timeToCloseDoor > 0 ) {
    --timeToCloseDoor;
    
    if (timeToCloseDoor == 0) {
      for ( int y = 0; y < kMapSize; ++y ) {
	for ( int x = 0; x < kMapSize; ++x ) {
	  
	  if ( enemyBaseMap[y][x] == 3 ) {
	    enemyBaseMap[y][x] = 4;      

	    if (playerCrawler.position.x == x && playerCrawler.position.y == y ) {
	      enemyBaseMap[y][x] = 3;
	      timeToCloseDoor = 1;
	    }	  
	  }	
	}
      }
    }    
  }
  
  auto spy = &enemies[0];

  memset( &enemiesInBase[0][0], 0, kMapSize * kMapSize );
  
  for ( int c = 0; c < kAgentsInBase && playerCrawler.life > 0; ++c ) {

      
    if (spy->life > 0 ) {
      enemiesInBase[spy->position.y][spy->position.x] = spy->id;
    
      if ((spy->id & 1) == (crawlerTurn & 1 ) ) {
	spy++;
	continue;
      }


      if (canSeeSpy(spy, &playerCrawler)) {
	shootSpy(spy, &playerCrawler);
	spy->showMuzzleflash = true;
	spy->target = playerCrawler.position;
      }

      pursueSpy( spy, spy->target);
    }

    spy++;
  }
}

void toggleTarget() {
  needRedrawView = true;

  for ( int c = 1; c <= kAgentsInBase; ++c ) {

    auto index = ((currentTarget - 1) + c) % kAgentsInBase;
    auto spy = &enemies[ index ];
    if ( spy->life > 0 && spy->seen ) {
      currentTarget = index + 1;
      gunPositionX = enemiesScreenOffsets[ index ];
      showGun();
      return;
    }
  }

  currentTarget = 0;
  hideGun();
}


void updatePlayerSector() {

  auto x = static_cast<int>(camera.x);
  auto y = static_cast<int>(camera.y);
  auto z = static_cast<int>(camera.z);

  if ( z < 0) {
    z = 0;
    camera.z = FixP{z};
  } else if ( z >= 32) {
    z = 31;
    camera.z = FixP{z};    
  }



  z = z + 8;
  
  for ( int c = 1; c < numRooms; ++c ) {
    Room *room = &rooms[c];

    if (room->p0.x <  x && x < room->p1.x &&
	room->p0.y <  z && z < room->p1.y &&
	room->height0 <  y && y < room->height1
	) {

      if ( playerRoom != c ) {
	printf("Player is now at %d\n", c);
	playerRoom = c;
      }
      return;
    }
  }
}

int32_t Crawler_tickCallback(int32_t tag, void* data) {

  if (playerCrawler.life == 0 ) {
    return Crawler_success();
  }
  
  uint8_t tile = 0;
  
  switch( tag ) {
  case kCommandLeft:
    playerCrawler.direction = leftOf( playerCrawler.direction );
    cachedVisibility = false;
    camera.x -= FixP{1};
    break;
  case kCommandRight:
    playerCrawler.direction = rightOf( playerCrawler.direction );    
    cachedVisibility = false;
    camera.x += FixP{1};    
    break;
  case kCommandUp:
    playerCrawler.position += mapOffsetForDirection(playerCrawler.direction);
    Crawler_tickTurn();
    tile = enemyBaseMap[playerCrawler.position.y][playerCrawler.position.x];

    if ( tile == 1 || tile == 4 || tile == 2 || (enemiesInBase[playerCrawler.position.y][playerCrawler.position.x] != 0) ) {
      playerCrawler.position -= mapOffsetForDirection(playerCrawler.direction);
    }

    if ( tile == 5 && playerHasClue ) {
      return Crawler_success();      
    }    

    camera.z += FixP{1};    
    break;
  case kCommandDown:
    playerCrawler.position -= mapOffsetForDirection(playerCrawler.direction);    
    Crawler_tickTurn();
    tile = enemyBaseMap[playerCrawler.position.y][playerCrawler.position.x];    

    if ( tile == 1 || tile == 4 || tile == 2 || (enemiesInBase[playerCrawler.position.y][playerCrawler.position.x] != 0) ) {      
      playerCrawler.position += mapOffsetForDirection(playerCrawler.direction);
    }


    if ( tile == 5 && playerHasClue ) {
      return Crawler_success();
    }    

    camera.z -= FixP{1};
    break;    
  case kCommandBack:
    return menuStateToReturn;
  case kCommandFire2:
    toggleTarget();
    camera.y -= FixP{1};    
    break;
  case kCommandFire1: {
    camera.y += FixP{1};
    
    Vec2i cursorPosition = playerCrawler.position + mapOffsetForDirection(playerCrawler.direction);    

    if ( currentTarget != 0 ) {
      playerCrawler.showMuzzleflash = true;
      playerCrawler.ammo--;
      enemies[currentTarget - 1 ].life = 0;
      currentTarget = 0;

      gunPositionY = 72;
      gunSpeedY = -1;

      cachedVisibility = false;      
      toggleTarget();
    }

    Crawler_tickTurn();
    
    if ( enemyBaseMap[cursorPosition.y][cursorPosition.x] == 4 ) {
      enemyBaseMap[cursorPosition.y][cursorPosition.x] = 3;
      timeToCloseDoor = 5;
    }

    if ( enemyBaseMap[cursorPosition.y][cursorPosition.x] == 2 ) {
      playerHasClue = true;
    }

  }
    break;
  default:
    break;
  }
  updatePlayerSector();
  
  return -1;
}

void Crawler_unloadStateCallback() {
  
  releaseBitmap(portableBitmap);
  releaseBitmap(pistolBitmap);
  releaseBitmap(pistolFiringBitmap);
  releaseBitmap(muzzleFlashBitmap);  
  releaseBitmap(enemySprite0);
  releaseBitmap(enemySprite1);  
  
  releaseTexture(wallTexture);
  releaseTexture(doorOpenTexture);
  releaseTexture(doorClosedTexture);
  releaseTexture(tableTexture);
  releaseTexture(clueTexture);
  releaseTexture(noClueTexture);
  releaseTexture(floorTexture);
  releaseTexture(ceilingTexture);
  releaseTexture(targetTexture);
}
