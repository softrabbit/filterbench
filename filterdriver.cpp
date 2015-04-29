// A little filter benchmarking...

#define STANDALONE
#ifdef BASELINE
#include "BasicFilters.h"
#else
#include "BasicFilters_modified.h"
#endif

#ifndef CHANCOUNT
#define CHANCOUNT 2
#endif

#include <iostream>
#include <stdlib.h>

using namespace std;

// Adjust so any one benchmark isn't too fast or too slow...
#define SAMPLERATE 96000
#define ITERATIONS SAMPLERATE*100

// Data to try, 600 frames is a realistic buffer size?
#include "akwf_raw_005.h"


int main( int argc, const char* argv[] ) {

     const string filternames[] = { "LowPass", "HiPass", "BandPass_CSG", "BandPass_CZPG", "Notch",
				    "AllPass", "Moog", "DoubleLowPass", "Lowpass_RC12", "Bandpass_RC12",
				    "Highpass_RC12", "Lowpass_RC24", "Bandpass_RC24", "Highpass_RC24",
				    "Formantfilter", "DoubleMoog", "Lowpass_SV", "Bandpass_SV",	
				    "Highpass_SV", "Notch_SV", "FastFormant", "Tripole" };

     const int filters=22;

     // BasicFilters<CHANCOUNT> * filter = new BasicFilters<CHANCOUNT>( SAMPLERATE );
	 void *ptr;
	 int foo = posix_memalign(&ptr, 16, sizeof(BasicFilters<CHANCOUNT>));
	 BasicFilters<CHANCOUNT> * filter = new(ptr) BasicFilters<CHANCOUNT>( SAMPLERATE );

     sample_t tmp1, tmp2, tmp3, tmp4;
     sample_t tmp_array[CHANCOUNT];
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
     if(selection==-1) {
	  cout << "Filter '" << argv[1] << "' invalid." << endl;
	  return 2;
     }

     filter->setFilterType(selection);


     if(argc>2 && string(argv[2]) == "output") {
	  filter->calcFilterCoeffs( 10000.0, 1.0 );
	  
	  // Assuming .25 seconds is enough to see any discrepancies
	  for(i=0; i<(SAMPLERATE>>2); i++) {
#ifdef BASELINE
	       tmp1 = filter->update( buffer[i % BUFFERSIZE], 0 );
	       cout << tmp1 << endl;
#else
	       // 
	       tmp_array[0] =  buffer[i % BUFFERSIZE];
	       filter->update_n( tmp_array );
	       cout << tmp_array[0] << endl;	       
#endif
	      
	  }
	  cout.flush();
	  return 0;
     } 


     // Test only setting frequency, worst case scenario is once per frame
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
#ifdef BASELINE
	  tmp1 = filter->update( buffer[i % BUFFERSIZE], 0 );
	  tmp2 = filter->update( buffer[(i+100) % BUFFERSIZE], 1 );
	  if(CHANCOUNT == 4) {
		  tmp3 = filter->update( buffer[i % BUFFERSIZE], 2 );
		  tmp4 = filter->update( buffer[(i+100) % BUFFERSIZE], 3 );
	  }	  

#else
	  tmp_array[0] = buffer[i % BUFFERSIZE];
	  tmp_array[1] = buffer[(i+100) % BUFFERSIZE];
	  if(CHANCOUNT == 4) {
		  tmp_array[2] = buffer[i % BUFFERSIZE];
		  tmp_array[3] = buffer[(i+100) % BUFFERSIZE];
	  }
	  filter->update_n( tmp_array );
#endif
     }
     

     return 0;
}
