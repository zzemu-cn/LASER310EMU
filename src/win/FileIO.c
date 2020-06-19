#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#define BUF_LEN 0x10000

// Handle to track open file.
FILE *handle;
FILE *romHandle;

// Variable to hold number of bytes read from file.
unsigned long int bytesRead;

// External variable used to store the ROM binary.
uint8_t romBuffer[BUF_LEN];

unsigned int bootRomPresent;

//----------------------------------------//
// Function name: LoadRomFile             //
// Purpose: This function will load a ROM //
// binary into the ROM buffer.            //
// Variables: filename is a string holding//
// the ROM file to be opened.             //
//----------------------------------------//
uint8_t * LoadRomFile(const char filename[], unsigned long int *bytesRead)
{
	//----------------------------------------//
	// Try to open the file in Read-Only      //
	// binary mode, if it fails, return with  //
	// an error.                              //
	//----------------------------------------//
	if((romHandle = fopen(filename, "rb")) == NULL)
		return 0;

	//----------------------------------------//
	// Read up to the max size of a GB ROM.   //
	//----------------------------------------//
	*bytesRead = fread(romBuffer, 1, BUF_LEN, romHandle);

	//----------------------------------------//
	// Close the file.                        //
	//----------------------------------------//
	fclose(romHandle);

	//----------------------------------------//
	// See if there were any problems reading //
	// the file and make sure the file size   //
	// is not 0.                              //
	//----------------------------------------//
	if ((!bytesRead) || (bytesRead == 0))
		return 0;

	//----------------------------------------//
	// Return with no errors.                 //
	//----------------------------------------//
	return romBuffer;
}


