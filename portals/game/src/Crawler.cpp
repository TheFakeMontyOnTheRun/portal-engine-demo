
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

struct Room {
	Vec2i p0;
	Vec2i p1;
	Texture *texture;
	int link[6];
	int height0;
	int height1;
	uint8_t lastRenderedFrame = 255;
};

Texture *wallTexture = NULL;
Texture *doorOpenTexture = NULL;
Texture *doorClosedTexture = NULL;
Texture *tableTexture = NULL;
Texture *clueTexture = NULL;
Texture *noClueTexture = NULL;
Texture *floorTexture = NULL;
Texture *ceilingTexture = NULL;
Texture *targetTexture = NULL;

P3D camera{FixP{-1}, FixP{0}, FixP{-15}};
Room *rooms;
int numRooms = 7;
int playerRoom = 1;

const uint8_t kDrawFaceFront = 0b00000001;
const uint8_t kDrawFaceRight = 0b00000010;
const uint8_t kDrawFaceBack = 0b00000100;
const uint8_t kDrawFaceLeft = 0b00001000;
const uint8_t kDrawFaceDown = 0b00010000;
const uint8_t kDrawFaceUp = 0b00100000;

uint8_t crawlerFrame = 0;


void addRoom(int8_t x0, int8_t y0, int8_t x1, int8_t y1) {
	Room room{Vec2i{x0, static_cast<int8_t>(y0)},
			  Vec2i{x1, static_cast<int8_t>(y1)},
			  ((numRooms % 2) == 0) ? noClueTexture : tableTexture,
			  {0, 0, 0, 0, 0, 0}, 1, 3};

	numRooms++;
	rooms = (Room *) realloc(rooms, sizeof(Room) * numRooms);
	memcpy(&rooms[numRooms - 1], &room, sizeof(Room));
}

bool compilerPass() {
	int xSlab;
	int ySlab;
	//  int zSlab;

	for (int c = 1; c < numRooms; ++c) {
		Room *roomC = &rooms[c];

		ySlab = roomC->p0.y;

		for (int d = 1; d < numRooms; ++d) {

			if (c == d) {
				continue;
			}

			Room *roomD = &rooms[d];

			if (roomD->p0.y < ySlab && ySlab < roomD->p1.y) {
				addRoom(roomD->p0.x, ySlab, roomD->p1.x, roomD->p1.y);
				roomD->p1.y = ySlab;
				return false;
			}
		}


		ySlab = roomC->p1.y;

		for (int d = 1; d < numRooms; ++d) {
			Room *roomD = &rooms[d];

			if (roomD->p0.y < ySlab && ySlab < roomD->p1.y) {
				addRoom(roomD->p0.x, ySlab, roomD->p1.x, roomD->p1.y);
				roomD->p1.y = ySlab;
				return false;
			}
		}

		xSlab = roomC->p0.x;

		for (int d = 1; d < numRooms; ++d) {
			Room *roomD = &rooms[d];

			if (roomD->p0.x < xSlab && xSlab < roomD->p1.x) {
				addRoom(xSlab, roomD->p0.y, roomD->p1.x, roomD->p1.y);
				roomD->p1.x = xSlab;
				return false;
			}
		}

		xSlab = roomC->p1.x;

		for (int d = 1; d < numRooms; ++d) {
			Room *roomD = &rooms[d];

			if (roomD->p0.x < xSlab && xSlab < roomD->p1.x) {
				addRoom(xSlab, roomD->p0.y, roomD->p1.x, roomD->p1.y);
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

	for (int c = 1; c < numRooms; ++c) {
		Room *roomC = &rooms[c];

		ySlab = roomC->p0.y;

		for (int d = 1; d < numRooms; ++d) {

			if (c == d) {
				continue;
			}

			Room *roomD = &rooms[d];

			if (roomD->p0.x == roomC->p0.x && roomD->p1.x == roomC->p1.x &&
				ySlab == roomD->p1.y) {
				roomC->link[0] = d;
				roomD->link[2] = c;
			}
		}

		ySlab = roomC->p1.y;

		for (int d = 1; d < numRooms; ++d) {

			if (c == d) {
				continue;
			}

			Room *roomD = &rooms[d];

			if (roomD->p0.x == roomC->p0.x && roomD->p1.x == roomC->p1.x &&
				ySlab == roomD->p0.y) {
				roomC->link[2] = d;
				roomD->link[0] = c;
			}
		}


		xSlab = roomC->p0.x;

		for (int d = 1; d < numRooms; ++d) {

			if (c == d) {
				continue;
			}

			Room *roomD = &rooms[d];

			if (roomD->p0.y == roomC->p0.y && roomD->p1.y == roomC->p1.y &&
				xSlab == roomD->p1.x) {
				roomC->link[3] = d;
				roomD->link[1] = c;
			}
		}


		xSlab = roomC->p1.x;

		for (int d = 1; d < numRooms; ++d) {

			if (c == d) {
				continue;
			}

			Room *roomD = &rooms[d];

			if (roomD->p0.y == roomC->p0.y && roomD->p1.y == roomC->p1.y &&
				ySlab == roomD->p0.x) {
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

int32_t Crawler_initStateCallback(int32_t tag, void *data) {
	wallTexture = makeTextureFrom(loadBitmap("res/wall.img"));
	floorTexture = makeTextureFrom(loadBitmap("res/ceiling.img"));
	ceilingTexture = makeTextureFrom(loadBitmap("res/floor.img"));
	targetTexture = makeTextureFrom(loadBitmap("res/target.img"));
	tableTexture = makeTextureFrom(loadBitmap("res/table.img"));
	doorOpenTexture = makeTextureFrom(loadBitmap("res/dooro.img"));
	doorClosedTexture = makeTextureFrom(loadBitmap("res/doorc.img"));
	clueTexture = makeTextureFrom(loadBitmap("res/clue.img"));
	noClueTexture = makeTextureFrom(loadBitmap("res/noclue.img"));

	numRooms = 7;
	rooms = (Room *) malloc(sizeof(Room) * numRooms);
	Room room0{Vec2i{-128, -128}, Vec2i{127, 127}, tableTexture,
			   {0, 0, 0, 0, 0, 0}, -128, 128};
	memcpy(&rooms[0], &room0, sizeof(Room));


	Room room1{Vec2i{-2, 14}, Vec2i{3, 9}, tableTexture, {2, 0, 0, 0, 0, 0}, -1,
			   1};
	Room room2{Vec2i{-2, 15}, Vec2i{3, 14}, noClueTexture, {3, 4, 1, 0, 0, 0},
			   -1, 1};
	Room room3{Vec2i{-2, 16}, Vec2i{3, 15}, tableTexture, {0, 0, 2, 5, 0, 0},
			   -1, 1};
	Room room4{Vec2i{3, 15}, Vec2i{4, 14}, clueTexture, {0, 0, 0, 2, 0, 0}, -1,
			   1};
	Room room5{Vec2i{-4, 16}, Vec2i{-2, 15}, clueTexture, {0, 3, 0, 0, 0, 0},
			   -1, 1};
	Room room6{Vec2i{-2, 14}, Vec2i{3, 9}, tableTexture, {0, 0, 6, 0, 1, 0}, 1,
			   3};

	memcpy(&rooms[1], &room1, sizeof(Room));
	memcpy(&rooms[2], &room2, sizeof(Room));
	memcpy(&rooms[3], &room3, sizeof(Room));
	memcpy(&rooms[4], &room4, sizeof(Room));
	memcpy(&rooms[5], &room5, sizeof(Room));
	memcpy(&rooms[6], &room6, sizeof(Room));

	compileMap();

	return 0;
}

void Crawler_initialPaintCallback(void) {
	graphicsFill(256, 0, 64, 128, 0);
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
	FixP cameraToFarEnd = (abs((FixP{v0.y} - (camera.z))));
	FixP sectorLength = FixP{abs(v1.y - v0.y)};
	const FixP _dz = min(sectorLength, cameraToFarEnd);

	if (_dz == zero) {
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
		/*
		graphicsDrawSprite( p2->x, p2->y,
				p3->x, p3->y,
				texture->regular,
				false, _dx, _dy);
		*/
	}

}

void setClippingRectForLink(int roomNumber, int link, const P3D &camera) {

	if (roomNumber == playerRoom) {
		graphicsSetClipRect(0, 0, 256, 128);
		return;
	}

	Room *room = &rooms[roomNumber];
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

	switch (link) {
		case 0: {

			cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p1->x),
											static_cast<int16_t>(p4->x)));
			cy0 = std::max<int>(0, std::min(static_cast<int16_t>(p1->y),
											static_cast<int16_t>(p4->y)));
			cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p1->x),
											  static_cast<int16_t>(p4->x)));
			cy1 = std::min<int>(128, std::max(static_cast<int16_t>(p1->y),
											  static_cast<int16_t>(p4->y)));
		}
			break;

		case 5: {

			cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p2->x),
											static_cast<int16_t>(p3->x))); //
			cy0 = std::min<int>(0, std::min(static_cast<int16_t>(p1->y),
											static_cast<int16_t>(p2->y))); //
			cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p2->x),
											  static_cast<int16_t>(p3->x))); //
			cy1 = std::max<int>(128, std::max(static_cast<int16_t>(p1->y),
											  static_cast<int16_t>(p2->y))); //

		}
			break;

		case 4: {

			cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p2->x),
											static_cast<int16_t>(p3->x))); //
			cy0 = std::min<int>(0, std::min(static_cast<int16_t>(p4->y),
											static_cast<int16_t>(p3->y))); //
			cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p2->x),
											  static_cast<int16_t>(p3->x))); //
			cy1 = std::max<int>(128, std::max(static_cast<int16_t>(p4->y),
											  static_cast<int16_t>(p3->y))); //

		}
			break;

		case 1: {

			cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p3->x),
											static_cast<int16_t>(p4->x)));
			cy0 = std::max<int>(0, std::min(static_cast<int16_t>(p3->y),
											static_cast<int16_t>(p2->y)));
			cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p3->x),
											  static_cast<int16_t>(p4->x)));
			cy1 = std::min<int>(128, std::max(static_cast<int16_t>(p3->y),
											  static_cast<int16_t>(p2->y)));

		}
			break;

		case 3: {
			cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p2->x),
											static_cast<int16_t>(p1->x)));
			cy0 = std::max<int>(0, std::min(static_cast<int16_t>(p2->y),
											static_cast<int16_t>(p3->y)));
			cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p2->x),
											  static_cast<int16_t>(p1->x)));
			cy1 = std::min<int>(128, std::max(static_cast<int16_t>(p2->y),
											  static_cast<int16_t>(p3->y)));
		}
			break;


		case 2: {
			cx0 = std::max<int>(0, std::min(static_cast<int16_t>(p2->x),
											static_cast<int16_t>(p3->x)));
			cy0 = std::max<int>(0, std::min(static_cast<int16_t>(p2->y),
											static_cast<int16_t>(p3->y)));
			cx1 = std::min<int>(256, std::max(static_cast<int16_t>(p2->x),
											  static_cast<int16_t>(p3->x)));
			cy1 = std::min<int>(128, std::max(static_cast<int16_t>(p2->y),
											  static_cast<int16_t>(p3->y)));
		}
			break;
	}

	graphicsEncloseClipRect(cx0, cy0, cx1, cy1);
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
						   oppositeOf(static_cast<EDirection>(fromLink)),
						   camera
	);

	if (clippingRect.x0 == clippingRect.x1 ||
		clippingRect.y0 == clippingRect.y1) {
		puts("null portal");
		return;
	}

	auto clipRect = graphicsGetCurrentClipRect();
	if (roomNumber == playerRoom) {
		flags = flags & ~kDrawFaceFront;
	}

	drawRoomAt(camera,
			   room->p0,
			   room->p1,
			   room->height0,
			   room->height1,
			   flags, room->texture);


//  graphicsFill( 256 + (  2* room->p0.x ),
//		2* ( room->p0.y ),
//		2* ( room->p1.x - room->p0.x ),
//		2* ( room->p1.y - room->p0.y ),
//		(roomNumber == playerRoom ) ? 255 : 128
//		);

	for (int link = 0; link < 6; ++link) {

		if (link == 0) {
			continue;
		}

		auto roomLinked = room->link[link];

		if (roomLinked) {

			if (rooms[roomLinked].lastRenderedFrame == crawlerFrame) {
				continue;
			}

			renderRooms(roomLinked, link, camera);
			graphicsSetClipRect(clipRect);

		}
	}
}


void Crawler_repaintCallback(void) {
	++crawlerFrame;
	graphicsFill(0, 0, 320, 200, 255);
	graphicsSetClipRect(0, 0, 256, 128);
	renderRooms(playerRoom, 2, camera);
	graphicsSetClipRect(0, 0, 320, 200);
}


void updatePlayerSector() {

	auto x = static_cast<int>(camera.x);
	auto y = static_cast<int>(camera.y);
	auto z = static_cast<int>(camera.z);

	if (z < 0) {
		z = 0;
		camera.z = FixP{z};
	} else if (z >= 32) {
		z = 31;
		camera.z = FixP{z};
	}

	z = z + 8;

	for (int c = 1; c < numRooms; ++c) {
		Room *room = &rooms[c];

		if (room->p0.x < x && x < room->p1.x &&
			room->p0.y < z && z < room->p1.y &&
			room->height0 < y && y < room->height1
				) {

			if (playerRoom != c) {
				printf("Player is now at %d\n", c);
				playerRoom = c;
			}
			return;
		}
	}
}

int32_t Crawler_tickCallback(int32_t tag, void *data) {

	switch (tag) {
		case kCommandLeft:
			camera.x -= FixP{1};
			break;
		case kCommandRight:
			camera.x += FixP{1};
			break;
		case kCommandUp:
			camera.z += FixP{1};
			break;
		case kCommandDown:
			camera.z -= FixP{1};
			break;
		case kCommandBack:
			return menuStateToReturn;
		case kCommandFire2:
			camera.y -= FixP{1};
			break;
		case kCommandFire1:
			camera.y += FixP{1};
			break;
		default:
			break;
	}
	updatePlayerSector();

	return -1;
}

void Crawler_unloadStateCallback() {
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
