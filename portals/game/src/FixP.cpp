#include <stdint.h>
#include <math.h>
#include "FixP.h"

bool operator<(const FixP v1, const FixP v2 ) {
  return v1.val < v2.val;
}

bool operator>(const FixP v1, const FixP v2 ) {
  return v1.val > v2.val;
}

bool operator==(const FixP v1, const FixP v2 ) {
  return v1.val == v2.val;
}

FixP operator+(const FixP v1, const FixP v2 ) {
  FixP toReturn;

  toReturn.val = v1.val + v2.val;

  return toReturn;
}

FixP operator-(const FixP v1, const FixP v2 ) {
  FixP toReturn;

  toReturn.val = v1.val - v2.val;

  return toReturn;
}

FixP operator*(const FixP v1, const FixP v2 ) {
  FixP toReturn;

  toReturn.val =  (((int64_t)v1.val) * ((int64_t)v2.val)) / ( 1 << kIntegerPart);
  
  return toReturn;
}

FixP operator/(const FixP v1, const FixP v2 ) {
  FixP toReturn;

  toReturn.val = (((int64_t)v1.val) * (1 << kIntegerPart) ) / v2.val;
  
  return toReturn;
}

FixP operator+=(FixP& v1, const FixP& v2 ) {

  v1.val = v1.val + v2.val;

  return v1;
}

FixP operator-=(FixP& v1, const FixP& v2 ) {

  v1.val = v1.val - v2.val;

  return v1;
}

FixP operator*=(FixP& v1, const FixP& v2 ) {

  
  v1.val =  (((int64_t)v1.val) * ((int64_t)v2.val)) / ( 1 << kIntegerPart);

  return v1;
}

FixP min(const FixP& v1, const FixP& v2) {
  return (v1.val <= v2.val)? v1 : v2;
}

FixP max(const FixP& v1, const FixP& v2) {
  return (v1.val <= v2.val)? v2 : v1;
}

FixP abs(const FixP v1 ) {
  FixP toReturn;
  toReturn.val = std::abs(v1.val);

  return toReturn;
}


FixP operator/=(FixP& v1, const FixP& v2 ) {

  v1.val = (((int64_t)v1.val) * (1 << kIntegerPart) ) / v2.val;  

  return v1;
}

float asFloat(FixP fp) {
  return (fp.val / ((float) pow( 2, (sizeof(FixP) * 8 ) - kIntegerPart)));
}
