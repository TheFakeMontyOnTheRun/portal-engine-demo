
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
#include <Renderer.h>
#include <vector>

#include "FixP.h"
#include "3D.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "Graphics.h"
#include "Vec2i.h"

enum Team{Player, Aliens};
enum GameState { Arriving, Battling, GameOver, Proceeding};

GameState currentGameState = Arriving;

struct Projectile {
    P3D mPosition{ FixP{0}, FixP{0}, FixP{100}};
    P3D mSpeed;
    bool mActive = true;
    Team mTeam;
    int mSector = 0;

    Projectile( const P3D& initialPos, const P3D& speed, Team team ) : mPosition{initialPos}, mSpeed{speed}, mTeam{team} {
    }
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

struct Enemy {
    P3D mPosition{ FixP{0}, FixP{0}, FixP{100}};
    P3D mSpeed;
    int mLife = 50;
    int mSector = 0;

    Enemy( const P3D& pos, const P3D& speed ) : mPosition(pos), mSpeed(speed) {

    }
};

int32_t onArriving();

int32_t onProceeding();

int32_t onGameOver();

void onInitRoom(int i);

Texture *wallTexture = NULL;
Texture *tableTexture = NULL;
Texture *clueTexture = NULL;
Texture *noClueTexture = NULL;
Texture *floorTexture = NULL;
Texture *ceilingTexture = NULL;
Texture *targetTexture = NULL;
Texture *playerSprite = NULL;
Texture *projectileSprite = NULL;
Texture *alienShotSprite = NULL;
Texture *enemySprite = NULL;

P3D camera{FixP{-1}, FixP{0}, FixP{0}};

P3D playerPosition{FixP{0}, FixP{0}, FixP{0}};
P3D playerAccel{FixP{0}, FixP{0}, FixP{0}};

std::vector<Projectile> projectiles;

std::vector<Enemy> enemies;

int fireCooldown = 0;
Room *rooms;
int numRooms = 7;
int playerRoom = 1;
int room = 1;

const uint8_t kDrawFaceFront = 0b00000001;
const uint8_t kDrawFaceRight = 0b00000010;
const uint8_t kDrawFaceBack  = 0b00000100;
const uint8_t kDrawFaceLeft  = 0b00001000;
const uint8_t kDrawFaceDown  = 0b00010000;
const uint8_t kDrawFaceUp    = 0b00100000;

uint8_t crawlerFrame = 0;



int32_t Crawler_initStateCallback(int32_t tag, void *data) {
    currentGameState = Arriving;
    playerPosition.z = 0;
    room = 1;
    wallTexture = makeTextureFrom(loadBitmap("res/wall.img"));
    floorTexture = makeTextureFrom(loadBitmap("res/ceiling.img"));
    ceilingTexture = makeTextureFrom(loadBitmap("res/floor.img"));
    targetTexture = makeTextureFrom(loadBitmap("res/target.img"));
    tableTexture = makeTextureFrom(loadBitmap("res/table.img"));
    clueTexture = makeTextureFrom(loadBitmap("res/clue.img"));
    noClueTexture = makeTextureFrom(loadBitmap("res/noclue.img"));
    playerSprite = makeTextureFrom(loadBitmap("res/player.img"));
    projectileSprite = makeTextureFrom(loadBitmap("res/shot.img"));
    alienShotSprite = makeTextureFrom(loadBitmap("res/shot2.img"));
    enemySprite = makeTextureFrom(loadBitmap("res/enemy.img"));
    onInitRoom(room++);

    return 0;
}

void Crawler_initialPaintCallback(void) {
}

void
drawRoomAt(const P3D &camera, const Vec2i &v0, const Vec2i &v1, int h0, int h1,
           uint8_t drawFacesFlags, Texture *texture) {

    FixP zero{0};
    auto kMinZCull{0};

    if (v1.y <= kMinZCull) {
        puts("behind frustum");
        return;
    }

    const FixP _dx = FixP{v1.x - v0.x};
    const FixP _dy = FixP{h1 - h0};

    FixP cameraToFarEnd = std::abs(FixP{std::min(v1.y, v0.y)} - (camera.z + FixP{4}));
    FixP sectorLength = FixP{abs(v1.y - v0.y)};
    const FixP _dz = min(sectorLength, cameraToFarEnd);

    if (_dz == zero) {
        puts("slim room");
        return;
    }

    /*        P1_____________
     *         /|          /|
     *     P2 /_|_________/ |
     *       |  |        |  |
     *       |  |        |  |
     *       |  |        |  |
     *       |__|________P3 |
     *       \  |         \ |
     *        \ |          \|
     *         \|___________\
     *                      P4
     */

    {
        const FixP x0 = FixP{v0.x} - camera.x;
        const FixP y0 = FixP{h0} - camera.y;
        const FixP z0 = FixP{v0.y} - camera.z - FixP{8};

        const FixP x1 = FixP{v1.x} - camera.x;
        const FixP y1 = FixP{h1} - camera.y;
        FixP z1 = FixP{v1.y} - camera.z - FixP{8};

        if (z1 < zero) {
            //      z1 = zero;
        }

        toProject[0] = P3D{x0, y1, z1};
        toProject[1] = P3D{x0, y1, z0};
        toProject[2] = P3D{x1, y0, z0};
        toProject[3] = P3D{x1, y0, z1};
    }

    projectPoints(4);

    const auto p1 = &projected[0];
    const auto p2 = &projected[1];
    const auto p3 = &projected[2];
    const auto p4 = &projected[3];


    if (drawFacesFlags & kDrawFaceDown) {

        graphicsDrawFloor(p4->y, p3->y,
                          p1->x, p4->x,
                          p2->x, p3->x,
                          texture->regular, _dx, _dz);

    }

    if (drawFacesFlags & kDrawFaceUp) {
        graphicsDrawFloor(p2->y, p1->y,
                          p2->x, p3->x,
                          p1->x, p4->x,
                          texture->regular, _dx, _dz);

    }


    if (drawFacesFlags & kDrawFaceRight) {

        graphicsDrawWall(p3->x, p4->x,
                         p2->y, p3->y,
                         p1->y, p4->y,
                         texture->rotated, _dz, _dy);

    }

    if (drawFacesFlags & kDrawFaceLeft) {

        graphicsDrawWall(p1->x, p2->x,
                         p1->y, p4->y,
                         p2->y, p3->y,
                         texture->rotated, _dz, _dy);

    }

    if (drawFacesFlags & kDrawFaceBack) {

        graphicsDrawSprite(p1->x, p1->y,
                           p4->x, p4->y,
                           texture->regular,
                           false, _dx, _dy);

    }


    if (drawFacesFlags & kDrawFaceFront) {

        graphicsDrawSprite(p2->x, p2->y,
                           p3->x, p3->y,
                           texture->regular,
                           false, _dx, _dy);

    }
}

void setClippingRectForLink(int roomNumber, int fromDirection, const P3D &camera) {

    if (roomNumber == playerRoom) {
        graphicsSetClipRect(0, 0, 256, 200);
        return;
    }

    Room *room = &rooms[roomNumber];
    auto v0 = room->p0;
    auto v1 = room->p1;
    auto h0 = room->height0;
    auto h1 = room->height1;

    int cx0 = 0;
    int cy0 = 0;
    int cx1 = 0;
    int cy1 = 0;

    FixP zero{0};
    auto kMinZCull{0};

    if (v1.y <= kMinZCull) {
        return;
    }

    /*        P1_____________
     *         /|          /|
     *     P2 /_|_________/ |
     *       |  |        |  |
     *       |  |        |  |
     *       |  |        |  |
     *       |__|________P3 |
     *       \  |         \ |
     *        \ |          \|
     *         \|___________\
     *                      P4
     */

    {

        const FixP x0 = FixP{v0.x} - camera.x;
        const FixP y0 = FixP{h0} - camera.y;
        const FixP z0 = FixP{v0.y} - camera.z - FixP{8};
        const FixP x1 = FixP{v1.x} - camera.x;
        const FixP y1 = FixP{h1} - camera.y;
        const FixP z1 = FixP{v1.y} - camera.z - FixP{8};
        toProject[0] = P3D{x0, y1, z1};
        toProject[1] = P3D{x0, y1, z0};
        toProject[2] = P3D{x1, y0, z0};
        toProject[3] = P3D{x1, y0, z1};
    }

    projectPoints(4);

    const auto p1 = &projected[0];
    const auto p2 = &projected[1];
    const auto p3 = &projected[2];
    const auto p4 = &projected[3];

    switch (fromDirection) {
        case 0: {

            cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p1->x),
                                            static_cast<int16_t>(p2->x)));
            cy0 = std::max<int>(0, std::min(static_cast<int16_t>(p1->y),
                                            static_cast<int16_t>(p2->y)));
            cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p3->x),
                                              static_cast<int16_t>(p4->x)));
            cy1 = std::min<int>(200, std::max(static_cast<int16_t>(p3->y),
                                              static_cast<int16_t>(p4->y)));
        }
            break;

        case 1: {

            cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p1->x),
                                            static_cast<int16_t>(p2->x)));
            cy0 = std::max<int>(0, std::min(static_cast<int16_t>(p1->y),
                                            static_cast<int16_t>(p2->y)));
            cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p1->x),
                                              static_cast<int16_t>(p2->x)));
            cy1 = std::min<int>(200, std::max(static_cast<int16_t>(p3->y),
                                              static_cast<int16_t>(p4->y)));

        }
            break;

        case 2: {
            cx0 = std::max<int>(0, std::max(static_cast<int16_t>(p1->x),
                                            static_cast<int16_t>(p2->x)));
            cy0 = std::max<int>(0, std::max(static_cast<int16_t>(p1->y),
                                            static_cast<int16_t>(p2->y)));
            cx1 = std::min<int>(256, std::min(static_cast<int16_t>(p3->x),
                                              static_cast<int16_t>(p4->x)));
            cy1 = std::min<int>(200, std::min(static_cast<int16_t>(p3->y),
                                              static_cast<int16_t>(p4->y)));
        }
            break;


        case 3: {
            cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p3->x),
                                            static_cast<int16_t>(p4->x)));
            cy0 = std::max<int>(0, std::min(static_cast<int16_t>(p1->y),
                                            static_cast<int16_t>(p2->y)));

            cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p3->x),
                                              static_cast<int16_t>(p4->x)));
            cy1 = std::min<int>(200, std::max(static_cast<int16_t>(p3->y),
                                              static_cast<int16_t>(p4->y)));
        }
            break;

        case 4: {

            cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p1->x),
                                            static_cast<int16_t>(p2->x))); //
            cy0 = std::min<int>(0, std::min(static_cast<int16_t>(p1->y),
                                            static_cast<int16_t>(p2->y))); //
            cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p3->x),
                                              static_cast<int16_t>(p4->x))); //
            cy1 = std::max<int>(200, std::max(static_cast<int16_t>(p1->y),
                                              static_cast<int16_t>(p2->y))); //

        }
            break;

        case 5: {

            cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p1->x),
                                            static_cast<int16_t>(p2->x))); //
            cy0 = std::min<int>(0, std::min(static_cast<int16_t>(p3->y),
                                            static_cast<int16_t>(p4->y))); //
            cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p3->x),
                                              static_cast<int16_t>(p4->x))); //
            cy1 = std::max<int>(200, std::max(static_cast<int16_t>(p3->y),
                                              static_cast<int16_t>(p4->y))); //

        }
            break;
    }

    graphicsEncloseClipRect(cx0, cy0, cx1, cy1);
}

void drawSpriteAt( const P3D& position, NativeBitmap* sprite) {
    FixP one{1};
    FixP half = one / FixP{1};


    {
        const FixP x0 = FixP{position.x} - half - camera.x;
        const FixP y0 = FixP{position.y} - half - camera.y;
        const FixP z0 = FixP{position.z} - half - camera.z;

        const FixP x1 = FixP{position.x} + half - camera.x;
        const FixP y1 = FixP{position.y} + half - camera.y;

        toProject[0] = P3D{x0, y0, z0};
        toProject[1] = P3D{x1, y1, z0};
    }

    projectPoints(2);

    const auto p1 = &projected[0];
    const auto p2 = &projected[1];


    graphicsDrawSprite(p1->x, p1->y,
                       p2->x, p2->y,
                       sprite,
                       true, one, one);

}

void renderRooms(int roomNumber, int fromLink, const P3D &camera) {

    Room *room = &rooms[roomNumber];

    if (room->lastRenderedFrame == crawlerFrame) {
        return;
    }

    auto flags = 0xFF;

    room->lastRenderedFrame = crawlerFrame;

    for (int link = 0; link < 6; ++link) {
        if (room->link[link]) {
            flags = flags & ~(1 << link);
        }
    }

    setClippingRectForLink(roomNumber,
                           fromLink,
                           camera
    );

    if (clippingRect.x0 == clippingRect.x1 ||
        clippingRect.y0 == clippingRect.y1) {
        return;
    }

    auto clipRect = graphicsGetCurrentClipRect();


    for (int link = 0; link < 6; ++link) {

        auto roomLinked = room->link[link];

        if (roomLinked) {

            if (rooms[roomLinked].lastRenderedFrame == crawlerFrame) {
                continue;
            }

            renderRooms(roomLinked, link, camera);
            graphicsSetClipRect(clipRect);

        }
    }

    flags = flags & ~kDrawFaceBack;

    drawRoomAt(camera,
               room->p0,
               room->p1,
               room->height0,
               room->height1,
               flags,
               room->texture);

    for (auto& enemy : enemies ) {
        if ( enemy.mSector == roomNumber ) {
            drawSpriteAt(enemy.mPosition, enemySprite->regular);
        }
    }

    for ( const auto& projectile : projectiles ) {
        if (projectile.mSector == roomNumber ) {
            drawSpriteAt(projectile.mPosition, projectile.mTeam == Player ? projectileSprite->regular : alienShotSprite->regular );
        }
    }
}

void Crawler_repaintCallback(void) {
    ++crawlerFrame;
    graphicsFill(0, 0, 320, 200, 255);
    graphicsSetClipRect(0, 0, 256, 200);
    renderRooms(playerRoom, 2, camera);

    drawSpriteAt(playerPosition, playerSprite->regular);
    
    graphicsSetClipRect(0, 0, 320, 200);

    graphicsFill(0, 192, fireCooldown >= 0 ? fireCooldown : 0, 8, 128 );
}


int updatePlayerSector( const P3D& pos ) {

    auto x = static_cast<int>(pos.x);
    auto y = static_cast<int>(pos.y);
    auto z = static_cast<int>(pos.z);

    if (z < 0) {
        z = 0;
        camera.z = FixP{z};
    } else if (z >= 32) {
        z = 31;
        camera.z = FixP{z};
    }

    for (int c = 1; c < numRooms; ++c) {
        Room *room = &rooms[c];

        if (room->p0.x <= x && x <= room->p1.x &&
            room->p1.y <= z && z <= room->p0.y && //yes, it's reversed. I'm weird.
            room->height0 <= y && y <= room->height1
                ) {
            return c;
        }
    }

    return 0;
}


int32_t onGamePlay(int32_t tag ) {

    for (auto& enemy : enemies ) {
        enemy.mPosition.x += enemy.mSpeed.x;
        enemy.mPosition.y += enemy.mSpeed.y;
        enemy.mSector = updatePlayerSector(enemy.mPosition);
        
        if ( enemy.mSector == 0 ) {
            enemy.mPosition.x -= enemy.mSpeed.x;
            enemy.mPosition.y -= enemy.mSpeed.y;

            enemy.mSpeed.x = FixP{(rand() % 32)} / FixP{1024} * ( (rand() % 2 ) ? FixP{-1} : FixP{1});
            enemy.mSpeed.y = FixP{(rand() % 32)} / FixP{1024} * ( (rand() % 2 ) ? FixP{-1} : FixP{1});
        }



        if (rand() % 128 == 0 && projectiles.size() < 32 ) {
            projectiles.emplace_back( enemy.mPosition, P3D{0, 0, FixP{-1} / FixP{10}}, Aliens );
        }
    }



    for ( auto& projectile : projectiles ) {
        projectile.mPosition.x += projectile.mSpeed.x;
        projectile.mPosition.y += projectile.mSpeed.y;
        projectile.mPosition.z += projectile.mSpeed.z;
        projectile.mSector = updatePlayerSector(projectile.mPosition);
        FixP half = FixP{1} / FixP{2};

        if (projectile.mTeam == Player ) {
            for ( auto& enemy : enemies ) {
                if (
                        ((projectile.mPosition.x - half) <= enemy.mPosition.x && enemy.mPosition.x <= (projectile.mPosition.x + half) ) &&
                        ((projectile.mPosition.y - half) <= enemy.mPosition.y && enemy.mPosition.y <= (projectile.mPosition.y + half) ) &&
                        static_cast<int>(projectile.mPosition.z) == static_cast<int>(enemy.mPosition.z) &&
                        projectile.mActive) {

                    enemy.mLife -= 20;
                    projectile.mActive = false;
                }
            }
        } else {
            if (
                    ((projectile.mPosition.x - half) <= playerPosition.x && playerPosition.x <= (projectile.mPosition.x + half) ) &&
                    ((projectile.mPosition.y - half) <= playerPosition.y && playerPosition.y <= (projectile.mPosition.y + half) ) &&
                    static_cast<int>(projectile.mPosition.z) == static_cast<int>(playerPosition.z) &&
                    projectile.mActive) {

                projectile.mActive = false;

                return kMainMenu;
            }

        }

    }


        enemies.erase(std::remove_if(std::begin(enemies),
                                     std::end(enemies),
                                     [](auto &enemy) {
                                         return enemy.mLife <= 0;
                                     }),
                      std::end(enemies)
        );


    projectiles.erase(std::remove_if(std::begin(projectiles),
                                     std::end(projectiles),
                                     [](auto& projectile){
                                         return !projectile.mActive || ( projectile.mSector == 0);
                                     }),
                      std::end(projectiles)
    );


    if (enemies.size() == 0) {
        if (projectiles.size() == 0) {
            currentGameState = Proceeding;
        }
        return -1;
    }

    fireCooldown--;

    P3D positionBackup{playerPosition};

    switch (tag) {
        case kCommandLeft:
            playerAccel.x -= FixP{1};
            break;
        case kCommandRight:
            playerAccel.x += FixP{1};
            break;
        case kCommandBack:
            return menuStateToReturn;
        case kCommandUp:
            playerAccel.y += FixP{1};
            break;




        case kCommandDown:
            playerAccel.y -= FixP{1};
            break;

        case kCommandFire2:
            currentGameState = Proceeding;
            break;

        case kCommandFire1:

            if (fireCooldown <= 0 ) {
                projectiles.emplace_back( playerPosition, P3D{0, 0, FixP{1} / FixP{10}}, Player );
                fireCooldown = 16;
            }
            break;


        default:
            break;
    }

    playerPosition.x += playerAccel.x;
    playerPosition.y += playerAccel.y;
    playerPosition.z += playerAccel.z;

    playerAccel.x /= FixP{2};
    playerAccel.y /= FixP{2};
    playerAccel.z /= FixP{2};

    camera.z = playerPosition.z - FixP{10};
    camera.y =( (FixP{4} * playerPosition.y) / FixP{10});
    camera.x = (playerPosition.x  * FixP{1}) / FixP{4};

    if ( updatePlayerSector(playerPosition) == 0 ) {
        playerPosition = positionBackup;
    }

    return -1;
}



int32_t Crawler_tickCallback(int32_t tag, void *data) {

    switch( currentGameState) {
        case Arriving:
            return onArriving();
        case Battling:
            return onGamePlay(tag);
        case GameOver:
            return onGameOver();
        case Proceeding:
            return onProceeding();
    }
}

int32_t onGameOver() {
    return kMainMenu;
}

int32_t onProceeding() {

    if (playerPosition.z < FixP{100}) {
        playerPosition.z += FixP{1};
    } else {
        currentGameState = Arriving;
        onInitRoom(room++);
        playerPosition.z = FixP{0};
    }

    return -1;
}

void onInitRoom(int room) {
    enemies.clear();
    projectiles.clear();

    numRooms = 7;
    rooms = (Room *) malloc(sizeof(Room) * numRooms);
    Room room0{Vec2i{-128, -128}, Vec2i{127, 127}, tableTexture, {0, 0, 0, 0, 0, 0}, -128, 128};

    //1
    Room room1{Vec2i{-2, 14}, Vec2i{3, 9}, noClueTexture, {2, 0, 0, 0, 0, 3}, -1, 1};

    //2
    Room room2{Vec2i{-2, 20}, Vec2i{3, 14}, clueTexture, {0, 4, 1, 6, 5, 0}, -1, 1};

    //3
    Room room3{Vec2i{-2, 14}, Vec2i{3, 9}, tableTexture, {0, 0, 0, 0, 1, 0}, 1, 5};

    //4
    Room room4{Vec2i{3, 20}, Vec2i{6, 14}, tableTexture, {0, 0, 0, 2, 0, 0}, -1, 1};

    //5
    Room room5{Vec2i{-2, 20}, Vec2i{3, 14}, tableTexture, {0, 0, 0, 0, 0, 2}, -2, -1};

    //6
    Room room6{Vec2i{-5, 20}, Vec2i{-2, 14}, tableTexture, {0, 2, 0, 0, 0, 0}, -1, 1};

    memcpy(&rooms[0], &room0, sizeof(Room));
    memcpy(&rooms[1], &room1, sizeof(Room));
    memcpy(&rooms[2], &room2, sizeof(Room));
    memcpy(&rooms[3], &room3, sizeof(Room));
    memcpy(&rooms[4], &room4, sizeof(Room));
    memcpy(&rooms[5], &room5, sizeof(Room));
    memcpy(&rooms[6], &room6, sizeof(Room));


    enemies.reserve(room);

    for ( int c = 0; c < room; ++c ) {
        FixP x;
        FixP y;
        FixP sx;
        FixP sy;

        sx =   FixP{(rand() % 32)} / FixP{1024} * ( (rand() % 2 ) ? FixP{-1} : FixP{1});
        sy =   FixP{(rand() % 32)} / FixP{1024} * ( (rand() % 2 ) ? FixP{-1} : FixP{1});

        do {
            x = FixP{(rand() % 128)} / FixP{128};
            y = FixP{(rand() % 128)} / FixP{128};
        } while(updatePlayerSector(P3D{x, y, FixP{15}}) == 0);

        enemies.emplace_back(P3D{x, y, FixP{15}}, P3D{ sx, sy, FixP{0}} );
    }
}

int32_t onArriving() {
    if (playerPosition.z < FixP{10}) {
        playerPosition.z += FixP{1};
    } else {
        currentGameState = Battling;
    }

    return -1;
}

void Crawler_unloadStateCallback() {
    releaseTexture(wallTexture);
    releaseTexture(tableTexture);
    releaseTexture(clueTexture);
    releaseTexture(noClueTexture);
    releaseTexture(floorTexture);
    releaseTexture(ceilingTexture);
    releaseTexture(targetTexture);
}
