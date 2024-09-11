#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <math.h>

#include <fftw.h>

double t;
double PI;
fftw_plan myplan;

// Buffer Size is 2^16
// BUFFERSIZE: is number of samples per second
//             also known as Sampling Frequency
#define BUFFERSIZE 65536

// ADC 2 Float conversion
#define ADC2FLOAT(n) (5.0f*((float)n)/1023.0f)

// Exit Variable
int KeepRunning = 1;

// Synchronization 
sem_t synchrosem;

// Global Variables
float CaptureBuffer[4*BUFFERSIZE];
float *pinputdata = NULL;
float *pfillingdata = CaptureBuffer;

// Start ADC
int ADC_init()
{
	usleep( 100 );
}    

// Finish ADC
void ADC_end()
{
	usleep( 100 );
}

// Get ADC value from SPI
#define FREC 3000
int ADC_getchannel(int chan)
{
	float y;
	
	if ( chan == 0 ) {
		y = 5.0f*cos(2.0*PI*FREC*t);
	} else {
		y = 1.0f*cos(2.0*PI*FREC*t);
	}
	
	return ((2.5 + y/5.0)*1023.0/5.0);
}

// Capture time is 1.0 second
// Data is filling in pfillingdata 
static void *thread_capture(void *arg)
{
	int i;
	
	// Start ADC
	ADC_init();
		
	// Ethernal loop
	while (KeepRunning) {
		t = t + 1.0/BUFFERSIZE;
		printf("(A) Start ADC Sampling\n");
		// Capture data and fill in buffer
		for (i=0; i<2*BUFFERSIZE; i+=2) {
			// Get Channels
			// SPI velocity set to required sampling rate
			pfillingdata[i]   = ADC2FLOAT( ADC_getchannel(0) );
			pfillingdata[i+1] = ADC2FLOAT( ADC_getchannel(1) );
		}
		usleep( 1000000/BUFFERSIZE );
		printf("(A) End ADC Sampling\n");

		// END Capturing, set pointers for processing and next capture
		// next pointer for FFT data
		pinputdata = pfillingdata;
		// increase pointer
		if ( pfillingdata == CaptureBuffer ) 
			pfillingdata = &CaptureBuffer[2*BUFFERSIZE];
		else
			pfillingdata = &CaptureBuffer[0];
			
		// data available for processing
		printf("(A) Increasing Semaphore\n");
		sem_post( &synchrosem );
		// Checking valid value
		sem_getvalue( &synchrosem, &i );
		if ( i > 1 ) {
			printf("(A) ERROR: Overrrun detected \n");
			KeepRunning = 0;
		}
	}
	
	// END ADC
	ADC_end();
	
	return NULL;
}

// Approximate numerical integration
float ComputeVibrationEnergy(int nvalues, float *yvalues, float dx)
{
	float result = 0.0f;
	int i;
	
	for ( i=0; i<nvalues-1; i++) {
		result += 0.5f*dx*yvalues[i] + 0.5f*dx*yvalues[i+1];
	}
	
	return result;
}

// Local variables for processing thread
fftw_complex x1[BUFFERSIZE], yout1[BUFFERSIZE];
fftw_complex x2[BUFFERSIZE], yout2[BUFFERSIZE];
float FFT1[BUFFERSIZE/2], FFT2[BUFFERSIZE/2];

// Processing time must be lower than capture time, for avoiding collision
// Data is processed in pinputdata
static void *thread_processing(void *arg)
{
	int i, j, Ns;
	float dFreq, Energ1, Energ2, dissp;

	/* Main loop until user press Ctl-C */
	while (KeepRunning) {
		// Capture sem at start time
		printf("\t\t\t(B) Waiting for data ... \n");
		sem_wait( &synchrosem );
		
		// Copy data to local variable
		printf("\t\t\t(B) Copying Data \n");
		j = 0;
		for (i = 0; i < BUFFERSIZE; i++) {
			x1[i].re = pinputdata[j];
			x1[i].im = 0.0;
			x2[i].re = pinputdata[j+1];
			x2[i].im = 0.0;
			j += 2;
		}
			
		// Compute FFT
		printf("\t\t\t(B) Doing FFT \n");
		fftw_one(myplan, x1, yout1);
		fftw_one(myplan, x2, yout2);
		// Only half data is required
		Ns = BUFFERSIZE/2;
		for (i = 0; i < Ns; i++) {
			FFT1[i] = sqrt( yout1[i].re*yout1[i].re + yout1[i].im*yout1[i].im );
			FFT2[i] = sqrt( yout2[i].re*yout2[i].re + yout2[i].im*yout2[i].im );
		}
		// delta frequency
		dFreq = ((float)BUFFERSIZE/2.0) / ((float)Ns);
		
		// Calculate vibration energy
		printf("\t\t\t(B) Computing Energy \n");
		Energ1 = ComputeVibrationEnergy(Ns, FFT1, dFreq);
		Energ2 = ComputeVibrationEnergy(Ns, FFT2, dFreq);
		
		// Apply rule
		if ( Energ1 > Energ2 ) {
			dissp = (Energ1 - Energ2)/Energ1;
		} else {
			dissp = (Energ2 - Energ1)/Energ2;			
		}
		// Print result
		printf("\t\t\t(B) Dissipation is: %f \n", dissp);
	}	

	return NULL;
}

// Control-C function: exit approval
void  ExitRequest(int sig)
{
	// Aprove Exit
	KeepRunning = 0;
}


// Local variables for MAIN
pthread_t thprocessing, thcapturing;

int main(int argc, char *argv[])
{
	// PI constant
	PI = 2.0*asin(1.0);
	
	// FFT plan
	myplan = fftw_create_plan(BUFFERSIZE, FFTW_FORWARD, FFTW_ESTIMATE);
	
	// Catch control-C
	signal( SIGINT, ExitRequest );
		
	// Create a semaphore, no shares, zero initial value
	sem_init(&synchrosem, 0, 0);
	
	// Launch processing thread
	pthread_create(&thprocessing, NULL, thread_processing, NULL);
	// Wait for creation
	sleep( 1 );
	// Launch capturing thread
	pthread_create(&thcapturing, NULL, thread_capture, NULL);
	
	// Wait for finish
	pthread_join(thcapturing, NULL);
	pthread_join(thprocessing, NULL);
	
	// FFTW Plan
	fftw_destroy_plan( myplan );
	
	// Destroy semaphore
	sem_destroy( &synchrosem );
	
	return 0;
}
