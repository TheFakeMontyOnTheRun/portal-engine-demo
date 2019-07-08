#include <stdint.h>

#include "Vec2i.h"

Vec2i::Vec2i( int8_t aX, int8_t aY ): x( aX ), y( aY ) {
}

Vec2i::Vec2i() : x( 0 ), y( 0 ) {
}

bool operator==( const Vec2i &lh, const Vec2i &rh ) {
  return ( lh.x == rh.x ) && ( lh.y == rh.y );
}

bool operator!=( const Vec2i &lh, const Vec2i &rh ) {
  return ( lh.x != rh.x ) || ( lh.y != rh.y );
}

Vec2i& Vec2i::operator+=(const Vec2i &other) {
  x += other.x;
  y += other.y;
  
  return *this;
}

Vec2i& Vec2i::operator-=(const Vec2i &other) {
  x -= other.x;
  y -= other.y;
  
  return *this;
}

Vec2i mapOffsetForDirection( EDirection aDirection ) {
  switch (aDirection) {
  case EDirection::kEast:
    return { 1, 0 };
  case EDirection::kWest:
    return { -1, 0 };
  case EDirection::kSouth:
    return { 0, 1 };
  case EDirection::kNorth:
    return { 0 , -1 };
  case EDirection::kUp:
  case EDirection::kDown:    
  default:
    return {0,0};
  }
}

EDirection wrapDirection(EDirection direction, int offset) {
  int8_t index = static_cast<int8_t>(direction) + offset;
  
  switch (direction) {
  case EDirection::kUp:
    return EDirection::kDown;
  case EDirection::kDown:
    return EDirection::kUp;
  case EDirection::kSouth:
  case EDirection::kEast:
  case EDirection::kWest:
  case EDirection::kNorth:    
  default: 
    
    
    while (index < 0) {
      index += 4;
    }
    
    while (index >= 4) {
      index -= 4;
    }
    
    return static_cast<EDirection>( index );
    
  }
}

EDirection oppositeOf(EDirection d) {
  switch (d) {
  case EDirection::kSouth:
    return EDirection::kNorth;
  case EDirection::kEast:
    return EDirection::kWest;
  case EDirection::kWest:
    return EDirection::kEast;
  case EDirection::kUp:
    return EDirection::kDown;
  case EDirection::kDown:
    return EDirection::kUp;
  case EDirection::kNorth:
    return EDirection::kSouth;
  default:
    return d;
  }
}

EDirection leftOf(EDirection d) {
  switch (d) {
  case EDirection::kNorth:
    return EDirection::kWest;
  case EDirection::kSouth:
    return EDirection::kEast;
  case EDirection::kEast:
    return EDirection::kNorth;
  case EDirection::kWest:
    return EDirection::kSouth;
  case EDirection::kUp:
  case EDirection::kDown:
  default:
    return d;
  }
}

EDirection rightOf(EDirection d) {
  switch (d) {
  case EDirection::kNorth:
    return EDirection::kEast;
  case EDirection::kSouth:
    return EDirection::kWest;
  case EDirection::kEast:
    return EDirection::kSouth;
  case EDirection::kWest:
    return EDirection::kNorth;
  case EDirection::kUp:
  case EDirection::kDown:
  default:
    return d;
  }
}

Vec2i operator+(const Vec2i &lh, const Vec2i &rh) {
  return Vec2i{static_cast<int8_t >(lh.x + rh.x), static_cast<int8_t >(lh.y + rh.y) };
}

