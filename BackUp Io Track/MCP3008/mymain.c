#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

#include <bcm2835.h>

// For FFT computation
extern int fft_pizero( int *, float * );


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
	if (!bcm2835_init()) {
		printf("bcm2835_init failed. Are you running as root??\n");
		return 1;
	}
	
	if (!bcm2835_spi_begin()) {
		printf("bcm2835_spi_begin failed. Are you running as root??\n");
		return 1;
	}
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
	// bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);   // The default
	bcm2835_spi_set_speed_hz(32*BUFFERSIZE);					// SPI velocity
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
}    

// Finish ADC
void ADC_end()
{
	bcm2835_spi_end();
	bcm2835_close();
}

// Get ADC value from SPI
int ADC_getchannel(int chan)
{
	uint8_t txbytes[2], rxbytes[2];

	txbytes[0] = 0xc0|(chan<<3);
	txbytes[1] = 0x00;

	bcm2835_spi_transfernb( txbytes, rxbytes, 2 );

    return ((((int)rxbytes[0] << 9) | ((int)rxbytes[1] << 1)) & 0x3ff);
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
		printf("(A) Start ADC Sampling\n");
		// Capture data and fill in buffer
		for (i=0; i<2*BUFFERSIZE; i+=2) {
			// Get Channels
			// SPI velocity set to required sampling rate
			pfillingdata[i]   = ADC2FLOAT( ADC_getchannel(0) );
			pfillingdata[i+1] = ADC2FLOAT( ADC_getchannel(1) );
		}
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
		if ( i >1 ) { 
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
float inputFFT1[BUFFERSIZE];
float inputFFT2[BUFFERSIZE];

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
			inputFFT1[i] = pinputdata[j];
			inputFFT2[i] = pinputdata[j+1];
			j += 2;
		}
			
		// Compute FFT
		Ns = BUFFERSIZE;
		printf("\t\t\t(B) Doing FFT \n");
		fft_pizero( &Ns, inputFFT1 );
		fft_pizero( &Ns, inputFFT2 );
		// Only half data is required
		Ns /= 2;
		// delta frequency
		dFreq = ((float)BUFFERSIZE/2.0) / ((float)Ns);
		
		// Calculate vibration energy
		printf("\t\t\t(B) Computing Energy \n");
		Energ1 = ComputeVibrationEnergy(Ns, inputFFT1, dFreq);
		Energ2 = ComputeVibrationEnergy(Ns, inputFFT2, dFreq);
		
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
	
	// Destroy semaphore
	sem_destroy( &synchrosem );
	
	return 0;
}
