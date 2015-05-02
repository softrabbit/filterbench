/* vocoder.c
   Version 0.3

   LADSPA Unique ID: 1337

   Version 0.31
   Added stereo output, renamed input/output ports, added,
   added a control for stereo balance

   Version 0.3
   Added support for changing bands in real time 2003-12-09

   Version 0.2
   Adapted to LADSPA by Josh Green <jgreen@users.sourceforge.net>
   15.6.2001 (for the LinuxTag 2001!)

   Original program can be found at:
   http://www.sirlab.de/linux/
   Author: Achim Settelmeier <settel-linux@sirlab.de>
   
   Adapted to LMMS by Hexasoft (hexasoft.corp@free.fr)
   
   
   Licence: GPL
   This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   
*/


/*****************************************************************************/
/* general includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif
/*****************************************************************************/
/* LADSPA headers */
#define LADSPA_Data float

/*****************************************************************************/


#define MAX_BANDS  16 /* max 16 bandsn should be increased */
#define AMPLIFIER 16.0

struct bandpass
{
  LADSPA_Data c, f, att;

  LADSPA_Data freq;
  LADSPA_Data low1, low2;
  LADSPA_Data mid1, mid2;
  LADSPA_Data high1, high2;
  LADSPA_Data y;
};

struct bands_out{
  LADSPA_Data decay;
  LADSPA_Data oldval;
  LADSPA_Data level;		/* 0.0 - 1.0 level of this output band */
};

const LADSPA_Data decay_table[] =
{
  1/100.0,
  1/100.0, 1/100.0, 1/100.0,
  1/125.0, 1/125.0, 1/125.0,
  1/166.0, 1/166.0, 1/166.0,
  1/200.0, 1/200.0, 1/200.0,
  1/250.0, 1/250.0, 1/250.0
};

/* The port numbers for the plugin: */

#define PORT_FORMANT   0  /* the track to "vocodify */
#define PORT_CARRIER   1  /* the track to control 1st track */
#define PORT_OUTPUT    2  /* left output */
#define PORT_OUTPUT2   3  /* right output */
#define CTRL_BANDCOUNT 4  /* selected # of bands to use */
#define CTRL_PAN       5  /* stereo balance for outputs */
#define CTRL_BAND1LVL  6  /* start of bands level */

#define PORT_COUNT     6 + MAX_BANDS  /* bands level */


/* useful macros */
#undef CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* Instance data for the vocoder plugin */
typedef struct {
  LADSPA_Data SampleRate;

  int num_bands;		/* current number of bands */
  float mainvol;		/* main volume */

  struct bandpass bands_formant[MAX_BANDS]; /* one structure per band */
  struct bandpass bands_carrier[MAX_BANDS]; /* one structure per band */
  struct bands_out bands_out[MAX_BANDS]; /* one structure per band */

  /* Ports */

  LADSPA_Data * portFormant;	/* Formant signal port data location */
  LADSPA_Data * portCarrier;	/* Carrier signal port data location */
  LADSPA_Data * portOutput;	/* Output audio port data location */
  LADSPA_Data * portOutput2;	/* Output audio port data location (copy of previous one) */
  LADSPA_Data * ctrlPan;	/* PAN for output */
  LADSPA_Data * ctrlBandCount;	/* Band count control */
  LADSPA_Data * ctrlBandLevels[MAX_BANDS]; /* level controls for each band */

} VocoderInstance;

/* Construct a new plugin instance. */
VocoderInstance * instantiateVocoder(
	unsigned long             SampleRate) {
  VocoderInstance * vocoder;

  vocoder = (VocoderInstance *)malloc(sizeof(VocoderInstance));

  if (vocoder == NULL)
    return NULL;

  vocoder->SampleRate = (LADSPA_Data)SampleRate;
  vocoder->num_bands = -1;

  return vocoder;
}

/*****************************************************************************/

/* Initialise and activate a plugin instance. */
void
activateVocoder(VocoderInstance *vocoder) {
  /*VocoderInstance *vocoder = (VocoderInstance *)Instance; */
  int i;

  vocoder->mainvol = 1.0 * AMPLIFIER;

  for (i = 0; i < MAX_BANDS; i++)
    vocoder->bands_out[i].oldval = 0.0;
}

/*****************************************************************************/

// vocoder_do_bandpasses /*fold00*/
void vocoder_do_bandpasses(struct bandpass *bands, LADSPA_Data sample,
			   VocoderInstance *vocoder)
{
  int i;
  for (i=0; i < vocoder->num_bands; i++)
    {
      bands[i].high1 = sample - bands[i].f * bands[i].mid1 - bands[i].low1;
      bands[i].mid1 += bands[i].high1 * bands[i].c;
      bands[i].low1 += bands[i].mid1;

      bands[i].high2 = bands[i].low1 - bands[i].f * bands[i].mid2
	- bands[i].low2;
      bands[i].mid2 += bands[i].high2 * bands[i].c;
      bands[i].low2 += bands[i].mid2;
      bands[i].y = bands[i].high2 * bands[i].att;
    }
}

/* Run a vocoder instance for a block of SampleCount samples. */
void 
runVocoder(VocoderInstance *vocoder,
	   unsigned long SampleCount)
{
  /* VocoderInstance *vocoder = (VocoderInstance *)Instance; */
  int i, j, numbands, pan;
  float a;
  LADSPA_Data x, c;
  float fl, fr;

  numbands = (int)(*vocoder->ctrlBandCount);
  if (numbands < 1 || numbands > MAX_BANDS) numbands = MAX_BANDS;

  /* initialize bandpass information if num_bands control has changed,
     or on first run */
  if (vocoder->num_bands != numbands)
    {
      vocoder->num_bands = numbands;

      for(i=0; i < numbands; i++)
	{
	  memset(&vocoder->bands_formant[i], 0, sizeof(struct bandpass));

	  a = 16.0 * i/(double)numbands;  // stretch existing bands

	  if (a < 4.0)
	    vocoder->bands_formant[i].freq = 150 + 420 * a / 4.0;
	  else
	    vocoder->bands_formant[i].freq = 600 * pow (1.23, a - 4.0);

	  c = vocoder->bands_formant[i].freq * 2 * M_PI / vocoder->SampleRate;
	  vocoder->bands_formant[i].c = c * c;

	  vocoder->bands_formant[i].f = 0.4/c;
	  vocoder->bands_formant[i].att =
	    1/(6.0 + ((exp (vocoder->bands_formant[i].freq
			    / vocoder->SampleRate) - 1) * 10));

	  memcpy(&vocoder->bands_carrier[i],
		 &vocoder->bands_formant[i], sizeof(struct bandpass));

	  vocoder->bands_out[i].decay = decay_table[(int)a];
	  vocoder->bands_out[i].level =
	    CLAMP (*vocoder->ctrlBandLevels[i], 0.0, 1.0);
	}
    }
  else		       /* get current values of band level controls */
    {
      for (i = 0; i < numbands; i++)
	vocoder->bands_out[i].level = CLAMP (*vocoder->ctrlBandLevels[i],
					     0.0, 1.0);
    }

  for (i=0; i < SampleCount; i++)
    {
      vocoder_do_bandpasses (vocoder->bands_carrier,
			     vocoder->portCarrier[i], vocoder);
      vocoder_do_bandpasses (vocoder->bands_formant,
			     vocoder->portFormant[i], vocoder);

      vocoder->portOutput[i] = 0.0;
      vocoder->portOutput2[i] = 0.0;
      for (j=0; j < numbands; j++)
	{
	  vocoder->bands_out[j].oldval = vocoder->bands_out[j].oldval
	    + (fabs (vocoder->bands_formant[j].y)
	       - vocoder->bands_out[j].oldval)
	    * vocoder->bands_out[j].decay;
	  x = vocoder->bands_carrier[j].y * vocoder->bands_out[j].oldval;
	  vocoder->portOutput[i] += x * vocoder->bands_out[j].level;
	  vocoder->portOutput2[i] += x * vocoder->bands_out[j].level;
	}
	    /* treat paning + main volume */
	    pan = (int)(*vocoder->ctrlPan);
      fl = fr = 1.;
	    if (pan != 0) { /* no paning, don't compute useless values */
        if (pan > 0) { /* reduce left */
          fl = (100.-pan)/100.;
        } else {
          fr = (100.+pan)/100.;
        }
      }
      /* apply volume and paning */
      vocoder->portOutput[i] *= vocoder->mainvol * fl;
      vocoder->portOutput2[i] *= vocoder->mainvol * fr;
    }
}


/*****************************************************************************/

/* Throw away a vocoder instance. */
void 
cleanupVocoder(  VocoderInstance * Vocoder)
{
  free(Vocoder);
}

/*****************************************************************************/

/* EOF */
