// A little filter benchmarking...

#define STANDALONE
#ifdef BASELINE
#include "basic_filters.h"
#else
#include "basic_filters_modified.h"
#endif

#include <iostream>

using namespace std;

// Adjust so any one benchmark isn't too fast or too slow...
#define SAMPLERATE 96000
#define ITERATIONS SAMPLERATE*100

// Data to try, 600 frames is a realistic buffer size?
#include "akwf_raw_005.h"

__attribute__((optimize("-O0")))
int main( int argc, const char* argv[] ) {

     const string filternames[] = { "LowPass", "HiPass", "BandPass_CSG", "BandPass_CZPG", "Notch",
				    "AllPass", "Moog", "DoubleLowPass", "Lowpass_RC12", "Bandpass_RC12",
				    "Highpass_RC12", "Lowpass_RC24", "Bandpass_RC24", "Highpass_RC24",
				    "Formantfilter" };
     const int filters=15;

     basicFilters<2> * filter = new basicFilters<2>( SAMPLERATE );
     sample_t tmp1, tmp2;
     int i;

     if( argc < 2 ) {
	  cout << argv[0] << " filter [output|denormal]" << endl;
	  cout << "Valid filters:" <<endl;
	  for(i=0;i<filters; i++) {
	       cout << filternames[i] << " ";
	  }
	  cout << endl;
	  return 1;
     }

     int selection = -1;
     for(i=0;i<filters; i++) {
	  if(string(argv[1]) == filternames[i])
	       selection = i;
     }
     if(i==-1) {
	  cout << "Filter '" << argv[1] << "' invalid." << endl;
	  return 2;
     }

     filter->setFilterType(selection);


     if(argc>2 && string(argv[2]) == "output") {
	  filter->calcFilterCoeffs( 10000.0, 1.0 );
	  
	  // Assuming .25 seconds is enough to see any discrepancies
	  for(i=0; i<(SAMPLERATE>>2); i++) {
	       tmp1 = filter->update( buffer[i % BUFFERSIZE], 0 );
	       cout << tmp1 << endl;
	  }
	  return 0;
     } 


     // Test only setting frequency
     if(argc>2 && string(argv[2]) == "coeffs") {
	  for(i=0; i<BUFFERSIZE; i++) {
	       buffer[i] = 8000.0 + buffer[i]*7000.0;
	  }
	  for(i=0; i<ITERATIONS; i++) {
	       filter->calcFilterCoeffs( buffer[i % BUFFERSIZE], (i%100)*.05);
	  }
	  return 0;
     }

     // Tweak the buffer contents if needed
     if(argc>2 && string(argv[2]) == "denormal") {
	  for(i=0; i<BUFFERSIZE; i++) {
	       buffer[i] = 1e-39;
	  }
     }

     // Default benchmark: run through a lot of stereo samples
     filter->calcFilterCoeffs( 10000.0, 1.0 );
     
     for(i=0; i<ITERATIONS; i++) {
	  tmp1 = filter->update( buffer[i % BUFFERSIZE], 0 );
	  tmp2 = filter->update( buffer[(i+100) % BUFFERSIZE], 1 );
     }
     

     return 0;
}
