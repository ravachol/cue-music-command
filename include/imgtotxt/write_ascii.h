#ifndef C_H
#define C_H
#include <stdbool.h>
#include "options.h"

/* This ifdef allows the header to be used from both C and C++. */
#ifdef __cplusplus
extern "C"
{
#endif
    int output_ascii(char *pathToImgFile, int height, int width, bool coverBlocks);
#ifdef __cplusplus
}
#endif

#endif
