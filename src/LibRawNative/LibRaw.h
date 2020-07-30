#pragma once

#include "libraw/libraw.h"

#ifdef __APPLE__
#define DllExport(t) extern "C" t
#elif __GNUC__
#define DllExport(t) extern "C" t
#else
#define DllExport(t) extern "C"  __declspec( dllexport ) t __cdecl
#endif


typedef struct {
	char* Make;
	char* Model;
	char* LensMake;
	char* LensModel;
	float FocalLength;
	float Aperture;
	int Width;
	int Height;
	char* Data;

} RawImage;

DllExport(void) lrFree(RawImage image);
DllExport(RawImage) lrLoad(const char* filename);