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
const double D_PI = (double) 3.14159265358979323846264338327950288419716939937510;
const double D_2PI = (double) D_PI * 2.0;
const float F_E = (float) 2.71828182845904523536028747135266249775724709369995;


// No need to involve the new memory manager in testing the algorithms
#define MM_OPERATORS /* */

// From qglobal.h
template <typename T>
inline const T &qMin(const T &a, const T &b) { return (a < b) ? a : b; }
template <typename T>
inline const T &qMax(const T &a, const T &b) { return (a < b) ? b : a; }
template <typename T>
inline const T &qBound(const T &min, const T &val, const T &max)
{ return qMax(min, qMin(max, val)); }

template <typename T>
inline T qAbs(const T &t) { return t >= 0 ? t : -t; }


template<class T>
inline T tLimit( const T x, const T x1, const T x2 )
{
        return qBound( x1, x, x2 );
}

// interpolation.h
inline float linearInterpolate( float v0, float v1, float x )
{
// take advantage of fma function if present in hardware

#ifdef FP_FAST_FMAF
        return fmaf( x, v1-v0, v0 );
#else
        return x * (v1-v0) + v0;
#endif  
}


#endif
