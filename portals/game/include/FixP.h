#ifndef FIXP_H
#define FIXP_H

#if UINT_MAX == UINT16_MAX
const uint8_t kIntegerPart = 6;
using FixPVal_t = int16_t;
#else
const uint8_t kIntegerPart = 16;
using FixPVal_t = int32_t;
#endif

struct FixP {
  FixPVal_t val = 0;
  
  operator int() const {
    return val >> kIntegerPart;
  }
  
  
FixP(FixPVal_t v) : val(v << kIntegerPart ){};
  FixP() = default;
};

FixP operator+=(FixP& v1, const FixP& v2 );
FixP operator-=(FixP& v1, const FixP& v2 );
FixP operator*=(FixP& v1, const FixP& v2 );
FixP operator/=(FixP& v1, const FixP& v2 );
bool operator<(const FixP v1, const FixP v2 );
bool operator>(const FixP v1, const FixP v2 );
bool operator==(const FixP v1, const FixP v2 );
FixP operator+(const FixP v1, const FixP v2 );
FixP operator-(const FixP v1, const FixP v2 );
FixP operator*(const FixP v1, const FixP v2 );
FixP operator/(const FixP v1, const FixP v2 );
FixP min(const FixP& v1, const FixP& v2);
FixP max(const FixP& v1, const FixP& v2);
FixP abs(const FixP v1 );
float asFloat(FixP fp);

#endif
