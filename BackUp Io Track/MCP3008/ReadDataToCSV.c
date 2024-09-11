#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include <bcm2835.h>


// Sampling Rate
// Number of samples per second
#define SAMPLINGFREQ 100000

// Measuring Time in ms
#define INPUTTIME 10

// Buffer size
#define BUFFERSIZE (INPUTTIME*(SAMPLINGFREQ/1000))

// ADC 2 Float conversion
#define ADC2FLOAT(n) (5.0f*((float)n)/1023.0f)

// Global Variables
// TRIGGER Point
int EndPoint;
// Buffer
float DataBuffer0[BUFFERSIZE];
float DataBuffer1[BUFFERSIZE];

// Start ADC
void ADC_init()
{
	if (!bcm2835_init()) {
		printf("bcm2835_init failed. Are you running as root??\n");
		exit(-1);
	}
	
	if (!bcm2835_spi_begin()) {
		printf("bcm2835_spi_begin failed. Are you running as root??\n");
		exit(-1);
	}
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
	// bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);   // The default
	bcm2835_spi_set_speed_hz(32*SAMPLINGFREQ);					  // SPI velocity
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
void CaptureData()
{
	float Data0, Data1;
	int i = 0, KeepRunning = 1, TriggerON = 0;
	
	// INIT ADC
	ADC_init();
	
	// Ethernal loop
	printf("** Start ADC Sampling\n");
	// Loop while capturing data
	EndPoint = BUFFERSIZE+1;
	while (KeepRunning) {
		// Capture data
		// SPI velocity set to required sampling rate
		DataBuffer0[i] = Data0 = 20.0f*(ADC2FLOAT( ADC_getchannel(0) ) - 2.5f);
		DataBuffer1[i] = Data1 = 20.0f*(ADC2FLOAT( ADC_getchannel(1) ) - 2.5f);
		
		// Check for Trigger 
		if ( (TriggerON == 0) && ((fabsf(Data0) >= 5.0f) || (fabsf(Data1) >= 5.0f)) ) {
			printf("** Trigger ON, sample %i\n", i);
			// Set Trigger ON
			TriggerON = 1;
			// Compute final point
			EndPoint = i - 10;
			if (EndPoint < 0) EndPoint += BUFFERSIZE;
		};

		// Increase pointer
		// Limit to BUFFERSIZE
		if (++i >= BUFFERSIZE) i = 0;
		
		// Check endpoint
		if ( i == EndPoint ) KeepRunning = 0;
	}
	
	// End ADC
	ADC_end();
}

// SAVE data to CSV file for further analysis
void SaveDataToCSVTrial()
{
	int i, j;
	
	FILE *pfile;	
	// SAVE CSV
	printf("Saving data to CSV \n");
	
	// Open file
	pfile = fopen("/home/pizero/bin/captured_data_trial.csv", "a+");
	// First channel
	j = EndPoint;
	for (i=0; i < BUFFERSIZE; i++) {
		fprintf( pfile, "%0.04f,", DataBuffer0[j] );
		if ( ++j>= BUFFERSIZE ) j = 0;
	};
	// Second channel
	j = EndPoint;
	for (i=0; i < BUFFERSIZE-1; i++) {
		fprintf( pfile, "%0.04f,", DataBuffer1[j] );
		if ( ++j>= BUFFERSIZE ) j = 0;
	};
	// Last value without comma
	fprintf( pfile, "%0.04f\n", DataBuffer1[j] );

	// Close file
	fclose( pfile );
}	

// SAVE data
void SaveDataToCSV()
{
	int i, j;
	
	FILE *pfile;	
	// SAVE CSV
	printf("Saving data to CSV \n");
	
	// Open file
	pfile = fopen("/home/pizero/bin/captured_data.csv", "a+");
	// First channel
	j = EndPoint;
	for (i=0; i < BUFFERSIZE; i++) {
		fprintf( pfile, "%0.04f,", DataBuffer0[j] );
		if ( ++j>= BUFFERSIZE ) j = 0;
	};
	// Second channel
	j = EndPoint;
	for (i=0; i < BUFFERSIZE-1; i++) {
		fprintf( pfile, "%0.04f,", DataBuffer1[j] );
		if ( ++j>= BUFFERSIZE ) j = 0;
	};
	// Last value without comma
	fprintf( pfile, "%0.04f\n", DataBuffer1[j] );

	// Close file
	fclose( pfile );
}	
	
// SAVE data
void SaveDataToTXT()
{
	int i, j;
	FILE *pfile;
	float dt;
	
	// Compute dt
	dt = 1.0f/((float)SAMPLINGFREQ);
	
	// Save to disk
	printf("Saving data to TXT \n");
	// Open file
	pfile = fopen("/home/pizero/Documents/MCP3008/captured_data.txt", "w");
	// Save previous data (1.0 ms)
	j = EndPoint;
	for (i=0; i < BUFFERSIZE; i++) {
		fprintf( pfile, "%0.06f, %0.04f, %0.04f \n", i*dt, DataBuffer0[j], DataBuffer1[j] );
		if (++j >= BUFFERSIZE ) j = 0;
	}
	// Close file
	printf("** Closing File\n");
	fclose( pfile );
}


// MAIN FILE
int main(int argc, char *argv[])
{
	// Start capturing data
	CaptureData();
	
	// Save data to file
	//SaveDataToCSV();
	SaveDataToCSV();
	//SaveDataToCSVTrial();
	//SaveDataToTXT();
	
	return 0;
}
