#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ORIGINAL
#include "vocoder_original.c"
#else
#include "vocoder_optimized.c"
#endif

#define sample_t float
#include "../akwf_raw_005.h"

int main(int argc, char *argv[]) {
	LADSPA_Data pan = 0.0f;
	LADSPA_Data bandcount = 16.0f;
	LADSPA_Data levels[MAX_BANDS] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
					 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};

	LADSPA_Data carrier[BUFFERSIZE]; /* buffersize is in akwf */
	srand(1337);
	for(int i=0; i<BUFFERSIZE; ++i) {
		carrier[i] = ( rand()*2.0 ) / RAND_MAX - 1.0;
	}
	LADSPA_Data output[BUFFERSIZE], output2[BUFFERSIZE];

	VocoderInstance * v = instantiateVocoder(96000);
	v->ctrlPan = &pan;
	v->ctrlBandCount = &bandcount;
	for(int i=0; i<MAX_BANDS; ++i) {
		v->ctrlBandLevels[i] = &levels[i];	
	}
	v->portCarrier = carrier;
	v->portFormant = buffer; // akwf
	v->portOutput = output;
	v->portOutput2 = output2;

	activateVocoder(v);
	
	if(argc>1 && strcmp(argv[1],"--output")==0 ) {
		runVocoder(v, BUFFERSIZE);
		for(int i=0; i<BUFFERSIZE; ++i) {
			printf("%.5f\t%.5f\n",output[i],output2[i]);
		}
	} else {
		for(int i=0; i<30000; ++i) {
			runVocoder(v, BUFFERSIZE);
		}
	}
	
	cleanupVocoder(v);

}
