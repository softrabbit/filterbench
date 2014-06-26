#ifndef _LMMS_BASICS_H
#define _LMMS_BASICS_H

// Stuff needed for the filters
#include <stdint.h>
#include <cstddef>

typedef uint32_t sample_rate_t;
typedef uint8_t ch_cnt_t;
typedef float sample_t;

const float F_PI = 3.1415926535f;
const float F_2PI = 2*F_PI;
const float F_PI_2 = F_PI*0.5;

// From qglobal.h
template <typename T>
inline const T &qMin(const T &a, const T &b) { return (a < b) ? a : b; }
template <typename T>
inline const T &qMax(const T &a, const T &b) { return (a < b) ? b : a; }
template <typename T>
inline const T &qBound(const T &min, const T &val, const T &max)
{ return qMax(min, qMin(max, val)); }

template<class T>
inline T tLimit( const T x, const T x1, const T x2 )
{
        return qBound( x1, x, x2 );
}

#endif
