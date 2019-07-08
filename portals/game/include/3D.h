#ifndef P3D_H
#define P3D_H

struct P3D {
  FixP x{0};
  FixP y{0};
  FixP z{0};
};

struct P2D {
  FixP x{0};
  FixP y{0};
};

extern P3D toProject[8];
extern P2D projected[8];

void projectPoints( uint8_t count );

#endif
