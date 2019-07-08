#ifndef VISIBILITYSTRATEGY_H
#define VISIBILITYSTRATEGY_H

const uint8_t kMapSize = 32;

using IntMap = uint8_t[ kMapSize ][ kMapSize ];
using VisMap = IntMap;
using DistanceDistribution = Vec2i[ kMapSize + kMapSize ][ kMapSize ];

void visibilityCastVisibility(VisMap &visMap, const IntMap &occluders, const Vec2i& pos, EDirection direction, bool cleanPrevious, DistanceDistribution& distances);

#endif

