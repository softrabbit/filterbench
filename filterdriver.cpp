// A little filter benchmarking...

#define STANDALONE
#ifdef BASELINE
#include "basic_filters.h"
#else
#include "basic_filters_modified.h"
#endif

#ifndef FILTER
#error "No filter!"
#endif

#include <iostream>
#include <string.h>

// Adjust so one benchmark isn't too fast or too slow...
#define SAMPLERATE 96000
#define ITERATIONS SAMPLERATE*100

int main( int argc, const char* argv[] ) {

  basicFilters<2> * filter = new basicFilters<2>( SAMPLERATE );
  sample_t tmp1, tmp2;
  int i;

#include "akwf_raw_005.h"

  if( argc == 2 && strcmp(argv[1], "output")==0 ) {
    filter->setFilterType(filter->FILTER);
    filter->calcFilterCoeffs( 10000.0, 1.0 );

    // Assuming .25 seconds is enough to see any discrepancies
    for(i=0; i<(SAMPLERATE>>2); i++) {
      tmp1 = filter->update( buffer[i % BUFFERSIZE], 0 );
      std::cout << tmp1 << std::endl;
    }
    return 0;
  } else {
    filter->setFilterType(filter->FILTER);
    filter->calcFilterCoeffs( 10000.0, 1.0 );

    for(i=0; i<ITERATIONS; i++) {
      tmp1 = filter->update( buffer[i % BUFFERSIZE], 0 );
      tmp2 = filter->update( buffer[(i+100) % BUFFERSIZE], 1 );
    }

  }
  return 0;
}
