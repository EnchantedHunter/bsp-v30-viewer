/* 
 * BSP v30 viewer
 * 
 *
 * Autor: Enchanted Hunter
 */

#ifndef WAD_H
#define WAD_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct _WADHEADER
{
    char szMagic[4];    // should be WAD2/WAD3
    int32_t nDir;       // number of directory entries
    int32_t nDirOffset; // offset into directory
} WADHEADER;

#define MAXTEXTURENAME 16

typedef struct _WADDIRENTRY
{
    int32_t nFilePos;            // offset in WAD
    int32_t nDiskSize;           // size in file
    int32_t nSize;               // uncompressed size
    int8_t nType;                // type of entry
    unsigned char bCompression;  // 0 if none
    int16_t nDummy;              // not used
    char szName[MAXTEXTURENAME]; // must be null terminated
} WADDIRENTRY;

#define MAXTEXTURENAME 16
#define MIPLEVELS 4

typedef struct _BSPMIPTEXWAD
{
    char szName[MAXTEXTURENAME];  // Name of texture
    uint32_t nWidth, nHeight;     // Extends of the texture
    uint32_t nOffsets[MIPLEVELS]; // Offsets to texture mipmaps BSPMIPTEX;
} BSPMIPTEXWAD;

#endif