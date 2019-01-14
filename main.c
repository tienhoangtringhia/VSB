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
#include "Bandpass_1024.h"

#define SAMPLES_PER_SECOND 48000
#define GAIN_IN_dB  5
#define PI 3.14159265359

Int16 left_input;
Int16 right_input;
Int16 left_output;
Int16 right_output;

unsigned long int i = 0;

#define Amp 1				// The amplitude of carrier wave
#define N 1024				// The number of samples

/*****************************************************************************/
/* DSB()                                                                     */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Perform DSB modulation by multiplying input by sinewave.                  */
/*                                                                           */
/*                                                                           */
/* PARAMETER 1: Modulating signal.                                                 */ 
/* PARAMETER 2: Carrier input signal                               */
/*                                                                           */
/*****************************************************************************/

static int DSB( int modulating_signal, int carrier_signal)
{ 
  signed long result;
  /* For TMS320C5505 it is necessary to first cast one int to long before */
  /* carrying out the multiplication                                      */
  result = Amp * ( ( (long) modulating_signal * carrier_signal) >> 15);    // 
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
	for (i = 1023  ; i >= 0 ; i--)
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
	int modulating_signal = 0;
	int dsb_modulated_wave = 0;
	int i = 0;
	
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
		if ( i < 1024)
		{
			aic3204_codec_read(&left_input, &right_input); // Configured for one interrupt per two channels.
			modulating_signal = 32768*(0.25*sin(2*PI*50*i/48000) + 0.25*sin(2*PI*80*i/48000) + 0.25*sin(2*PI*150*i/48000) + 0.25*sin(2*PI*300*i/48000));
			dsb_modulated_wave = DSB(modulating_signal, right_input); 
			left_output = Filter(dsb_modulated_wave);     // Bandpass Filter -> VSB modulating 
			right_output = right_input;	                  // Directly connect input to output
			input[i] = right_output;
			output[i] = left_output;
			i++;
			
			if (j = 0){
				j = 1;
				px = &input[0];
				ExportFile( N, (Uint16*) px, 0) ;  // Export input signal to binary file
				px = &output[0];
				ExportFile( N, (Uint16*) px, 1) ;  // Export VSB output signal to binary file
			}
		} else {
			i = 0;
		}

		aic3204_codec_write(left_output, right_output);
	}
}

/*****************************************************************************/
/* End of main.c                                                             */
/*****************************************************************************/

