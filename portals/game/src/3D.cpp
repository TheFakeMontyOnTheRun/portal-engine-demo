#include <stdint.h>

#include "FixP.h"
#include "3D.h"

P3D toProject[8];
P2D projected[8];

void projectPoints( uint8_t count ) {

  P3D* vertex = &toProject[0];
  P2D* dst = &projected[0];
  const FixP halfWidth{128};
  const FixP halfHeight{64};
  const FixP zero{0};
  const FixP one{1};
  const FixP two{2};
  const FixP bias = one / FixP{128};

  for ( uint8_t c = 0; c < count; ++c ) {
    
    auto z = (vertex->z);
    
    if ( z < one ) {
      z = one;
    }
    
    FixP projected = z / two;
    
    if ( projected == zero ) {
      projected += bias;
    }
    
    const FixP oneOver = (halfHeight) / projected;
    
    dst->x = halfWidth + (vertex->x * oneOver);
    dst->y = halfHeight - (vertex->y * oneOver);    
    
    ++vertex;
    ++dst;
  }  
}
