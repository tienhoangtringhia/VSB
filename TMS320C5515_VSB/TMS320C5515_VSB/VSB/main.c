/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/* FILENAME                                                                  */
/*  main.c                                                                   */
/*                                                                           */
/* DESCRIPTION                                                               */
/*   TMS320C5515 USB Stick. Code containing main() function.                 */
/*****************************************************************************/

#include "stdio.h"
#include "usbstk5515.h"
#include "aic3204.h"
#include "sinewaves.h"
#include "Transfer.h"
#include "PLL.h"

#define SAMPLES_PER_SECOND 48000
#define GAIN_IN_dB  5

Int16 left_input;
Int16 right_input;
Int16 left_output;
Int16 right_output;

unsigned long int i = 0;

#define Amp 2				// The amplitude of carrier wave
#define N 1000				// The number of samples


Int16 BP[49] = { 109,    419,   -226,   -340,    210,    207,    -42,    129,   -347,
     -507,    813,    700,  -1033,   -541,    623,     49,    648,    548,
    -2667,   -927,   4934,    856,  -6726,   -346,   7407,   -346,  -6726,
      856,   4934,   -927,  -2667,    548,    648,     49,    623,   -541,
    -1033,    700,    813,   -507,   -347,    129,    -42,    207,    210,
     -340,   -226,    419,    109};
     
 Int16 Temp[49] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*****************************************************************************/
/* DSB()                                                                     */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Perform DSB modulation by multiplying input by sinewave.                  */
/*                                                                           */
/*                                                                           */
/* PARAMETER 1: Audio input.                                                 */ 
/* PARAMETER 2: Carrier signal. Frequency fc                                 */
/*                                                                           */
/*****************************************************************************/

static int DSB( int input, int fc)
{ 
  signed long result;
  Int16 carrierwave;
  /* For TMS320C5505 it is necessary to first cast one int to long before */
  /* carrying out the multiplication                                      */
  carrierwave = generate_sinewave_1 (fc, 32767); 			// The amplitude of fc sine wave 1.000 = 32767
  result = Amp * ( ( (long) input * carrierwave) >> 15);    // 
  return ( (int) result);
}

/*****************************************************************************/
/* Filter()                                                                  */
/*---------------------------------------------------------------------------*/
/* Apply VSB Filter on DSB signal to obtain VSB modulation.                  */
/* PARAMETER 1: DSB Signal.                                                  */ 
/*****************************************************************************/

static int Filter(int input)
{	signed long result = 0;
	int i;
	//Dich cac gia tri cu di 1 don vi
	for (i = 47  ; i >= 0 ; i--)
	{
		result = result + (long)BP[i+1] * (long)Temp[i];
		Temp[i+1] = Temp[i];
	}
	result = result + (long)input * (long)BP[0];
	result = result >> 15;
	Temp[0] = input;
    return ( (int) result);
}


/* ------------------------------------------------------------------------ *
 *                                                                          *
 *  main( )                                                                 *
 *                                                                          *
 * ------------------------------------------------------------------------ */

void main( void ) 
{	
	Uint16 *px;
	Int16 input[N],output[N];
	int i = 0,j = 0;
	
    /* Initialize BSL */
    USBSTK5515_init( );

	/* Initialize Phase Locked Loop (PLL) */
    pll_frequency_setup(100);

    /* Initialise hardware interface and I2C for code */
    aic3204_hardware_init();
    
    /* Initialise the AIC3204 codec */
	aic3204_init(); 
	
	/* Setup sampling frequency and 30dB gain for microphone */
    set_sampling_frequency_and_gain(SAMPLES_PER_SECOND, GAIN_IN_dB);

    puts("\nDieu che VSB Song mang 5 kHz");


	while(1) {
		aic3204_codec_read(&left_input, &right_input); // Configured for one interrupt per two channels.
		
		left_output = DSB(right_input, 5000);  // DSB modulating with carrier frequency 5 kHz
		left_output = Filter(left_output);     // Bandpass Filter -> VSB modulating 
		right_output = right_input;	           // Directly connect input to output
		
		if(i < N ) {
			input[i] = right_output;
			output[i] = left_output;
			i++;
		} else {
			px = &input[0];
			ExportFile( N, (Uint16*) px, 0) ;  // Export input signal to binary file
			px = &output[0];
			ExportFile( N, (Uint16*) px, 1) ;  // Export VSB output signal to binary file
			}
		}

		aic3204_codec_write(left_output, right_output);
	}
}

/*****************************************************************************/
/* End of main.c                                                             */
/*****************************************************************************/

