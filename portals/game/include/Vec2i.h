//
// Created by monty on 05/10/16.
//

#ifndef VEC2I_H
#define VEC2I_H


struct Vec2i {
  int8_t x;
  int8_t y;
  Vec2i( int8_t aX, int8_t aY ); 
  Vec2i();  
  Vec2i& operator+=( const Vec2i &other );  
  Vec2i& operator-=( const Vec2i &other );
};

enum EDirection {
  kNorth,
  kEast,
  kSouth,
  kWest,
  kDown,
  kUp
};

Vec2i operator+( const Vec2i &lh, const Vec2i &rh );

bool operator==( const Vec2i &lh, const Vec2i &rh );

bool operator!=( const Vec2i &lh, const Vec2i &rh );

EDirection wrapDirection( EDirection direction, int offset );
Vec2i mapOffsetForDirection( EDirection direction );
EDirection leftOf(EDirection d);
EDirection rightOf(EDirection d);
EDirection oppositeOf(EDirection d);

#endif //VEC2I_H
