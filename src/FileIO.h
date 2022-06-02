#ifndef FILEIO_H_
#define FILEIO_H_

#ifdef __cplusplus
extern "C" {
#endif
uint8_t* LoadRomFile(const char *filename, unsigned long int *fileSize);

#ifdef __cplusplus
}
#endif
#endif  //FILEIO_H_
