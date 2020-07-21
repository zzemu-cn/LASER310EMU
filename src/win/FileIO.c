#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#include "prgdef.h"

// Handle to track open file.
FILE *handle;
//FILE *fp;

// Variable to hold number of bytes read from file.
unsigned long int bytesRead;

unsigned int bootRomPresent;


size_t SaveFile(const char fn[], char* buf, const size_t sz)
{
	FILE* fp;
	size_t cnt;

	if((fp = fopen(fn, "wb")) == NULL)
		return 0;

	cnt = fwrite(buf, sz, 1, fp);

	fclose(fp);

	return cnt;
}

//----------------------------------------//
// Function name: LoadRomFile             //
// Purpose: This function will load a ROM //
// binary into the ROM buffer.            //
// Variables: filename is a string holding//
// the ROM file to be opened.             //
//----------------------------------------//
uint8_t * LoadRomFile(const char filename[], unsigned long int *bytesRead)
{
	FILE *fp;
	//----------------------------------------//
	// Try to open the file in Read-Only      //
	// binary mode, if it fails, return with  //
	// an error.                              //
	//----------------------------------------//
	if((fp = fopen(filename, "rb")) == NULL)
		return 0;

	//----------------------------------------//
	// Read up to the max size of a GB ROM.   //
	//----------------------------------------//
	*bytesRead = fread(tmp_buf, 1, TMP_BUF_LEN, fp);

	//----------------------------------------//
	// Close the file.                        //
	//----------------------------------------//
	fclose(fp);

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
	return tmp_buf;
}

size_t filelen(FILE *fp)
{
	size_t	curpos, pos;

	if( (curpos = ftello(fp))==-1LL )
		return -1LL;

	if( fseeko( fp, 0LL, SEEK_END ) )
		return -1LL;

	if( (pos = ftello(fp))==-1LL )
		return -1LL;

	if( fseeko( fp, curpos, SEEK_SET ) )
		return -1LL;

	return pos;
}


/* 每次读取 1G */
#define FREAD_LEN 0x40000000LL

size_t readfile(const char* fn, uint8_t* buf)
{
	FILE	*fp;
	size_t	sz;

	//printf("fopen\n");
	if( ( fp = fopen(fn, "rb") ) == NULL ) return 0;

	//printf("filelen\n");
	sz = filelen(fp);
	if( sz<=0 ) { fclose(fp); return 0; }

	/* 实测，win64 mingw环境下，fread 一次读取不能超过 2G (包括2G) */

	/* 每次读取 1G */
	size_t	read_c=0LL;
	size_t	read_sz;

	while(read_c<sz) {
		read_sz = (read_c+FREAD_LEN<=sz) ? FREAD_LEN : sz-read_c;
		//printf("fread %d %d %d\n", sz, read_c, read_sz);
		if( fread( (char*)buf+read_c, read_sz, 1, fp ) != 1LL ) { fclose(fp); return 0; }
		read_c+=read_sz;
	}

	fclose(fp);

	return read_c;
}


