/*
 * ReadWrite.h
 *
 *  Created on: 04/12/2013
 *      Author: gabrielma
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
//#include <stddef.h>
#include <sys/time.h>
#include <ctype.h>

#ifndef READWRITE_H_
#define READWRITE_H_


void cleanString(char *cadena, char *out);
void readHeader(char* filename, int *cols, int *rows, int *numBands, int *dataType);
void Load_Image(char* filename, float *imageVector, int cols, int rows, int numBands, int dataType);
void Load_Image_imagenes(char* filename, float *imageVector, int cols, int rows, int numBands, int dataType);
void writeResult(double *imagen, const char* resultado_filename, int num_samples, int num_lines, int num_bands);
void writeHeader(const char* outHeader, int samples, int lines, int bands);

#endif /* READWRITE_H_ */

