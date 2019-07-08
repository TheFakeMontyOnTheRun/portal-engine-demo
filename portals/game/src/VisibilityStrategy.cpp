//
// Created by monty on 30/07/16.
//
#include <cmath>
#include <cstdlib>
#include <stdint.h>
#include <string.h>
#include "Vec2i.h"
#include "VisibilityStrategy.h"

const bool kNarrowByDistance = true;
//const bool kConservativeOccluders = true;
const int kUseLimitedDrawingDistance = false;
const int kDrawingDistance = 20;
//const char* occluderString = "1IYXPR";
//const uint8_t kBucketSize = 128;

enum EVisibility{ kInvisible, kVisible };

Vec2i visibilityTransform( EDirection from, const Vec2i& currentPos ) {
  
  switch( from ) {
  case EDirection::kNorth:
  case EDirection::kUp:
  case EDirection::kDown:
    return currentPos;
  case EDirection::kSouth:
    return { static_cast<int8_t>(kMapSize - currentPos.x - 1), static_cast<int8_t>(kMapSize - currentPos.y - 1)};
  case EDirection::kEast:
    return { static_cast<int8_t>(kMapSize - currentPos.y - 1), static_cast<int8_t>(kMapSize - currentPos.x - 1)};
  case EDirection::kWest:
    return { static_cast<int8_t>(currentPos.y), static_cast<int8_t>(currentPos.x)};
  }
}

bool visibilityIsValid(const Vec2i& pos) {
  return 0 <= pos.x && pos.x < kMapSize && 0 <= pos.y && pos.y < kMapSize;
}

bool visibilityIsBlock(const IntMap& occluders, const Vec2i& transformed) {
  
  auto tile = occluders[ transformed.y ][ transformed.x];
  
  return tile == 1 || tile == 4;
}


bool visibilityIsVisibleAt(const VisMap& visMap, const Vec2i& transformed ) {
  return visMap[ transformed.y ][ transformed.x ] == EVisibility::kVisible;
}

void visibilitySetIsVisible(VisMap& visMap, const Vec2i& transformed ) {
  visMap[ transformed.y ][ transformed.x ] = EVisibility::kVisible;
}

void visibilityCastVisibility(EDirection from, VisMap &visMap, const IntMap &occluders,
			      const Vec2i& originalPos, DistanceDistribution& distances) {
  
  Vec2i positions[kMapSize + kMapSize];
  Vec2i currentPos;
  
  //The -1 is due to the fact I will add a new element.
  auto stackHead = &positions[0];
  auto stackEnd = stackHead + (kMapSize + kMapSize);
  auto stackRoot = stackHead;
  
  auto rightOffset = mapOffsetForDirection(EDirection::kEast);
  auto leftOffset = mapOffsetForDirection(EDirection::kWest);
  auto northOffset = mapOffsetForDirection(EDirection::kNorth);
  
  *stackHead = originalPos;
  ++stackHead;
  
  uint8_t bucketPositions[ kMapSize + kMapSize ];
  
  memset( &distances, INT8_MIN, ((kMapSize + kMapSize) * kMapSize) * sizeof(Vec2i) );
  memset( &bucketPositions, 0, (kMapSize + kMapSize) );
  
  while (stackHead != stackRoot ) {
    --stackHead;
    
    currentPos = *stackHead;
    
    auto transformed = visibilityTransform( from, currentPos);
    
    if (!visibilityIsValid(transformed)) {
      continue;
    }
    
    if ( visibilityIsVisibleAt( visMap, transformed ) ) {
      continue;
    }
    
    visibilitySetIsVisible( visMap, transformed );
    
    int verticalDistance = ( currentPos.y - originalPos.y );
    
    if (kUseLimitedDrawingDistance ) {      
      if ( std::abs(verticalDistance) > kDrawingDistance) {
	continue;
      }
    }
    
    int manhattanDistance = std::abs( verticalDistance ) + std::abs( currentPos.x - originalPos.x );

    if ( manhattanDistance < (kMapSize + kMapSize) ) {   
      distances[ manhattanDistance ][ (bucketPositions[ manhattanDistance ]++) ] = transformed;
    }
    
    if (visibilityIsBlock(occluders, transformed )) {
      continue;
    }
    
    int narrowing = kNarrowByDistance ? std::abs(verticalDistance) : 1;
    
    if ( ( !kNarrowByDistance || ( currentPos.x - originalPos.x ) >= -narrowing )&& ( currentPos.x - originalPos.x ) <= 0 && (stackHead != stackEnd ) ) {
      *stackHead++ =  Vec2i{static_cast<int8_t>(currentPos.x + leftOffset.x), static_cast<int8_t>(currentPos.y + leftOffset.y)};
    }
    
    if ( ( !kNarrowByDistance || ( currentPos.x - originalPos.x ) <= narrowing ) && ( currentPos.x - originalPos.x ) >= 0 && (stackHead != stackEnd ) ) {
      *stackHead++ =  Vec2i{static_cast<int8_t>(currentPos.x + rightOffset.x), static_cast<int8_t>(currentPos.y + rightOffset.y)};
    }
    
    if (  verticalDistance <= 0 && (stackHead != stackEnd) ) {
      *stackHead++ =  Vec2i{static_cast<int8_t>(currentPos.x + northOffset.x), static_cast<int8_t>(currentPos.y + northOffset.y)};
    }    
  }
}

void visibilityCastVisibility(VisMap &visMap, const IntMap &occluders, const Vec2i& pos, EDirection direction, bool cleanPrevious, DistanceDistribution& distances) {
  
  if ( cleanPrevious ) {
    memset( visMap, EVisibility::kInvisible, kMapSize * kMapSize );       
  }
  
  visibilityCastVisibility(direction, visMap, occluders, visibilityTransform( direction, pos ), distances);
}
