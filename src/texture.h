/* 
 * BSP v30 viewer
 * 
 *
 * Autor: Enchanted Hunter
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>

// =========== TEXTURE ============

typedef struct _TEXTURE
{
    uint32_t iWidth;
    uint32_t iHeight;
    unsigned char * data;
} TEXTURE;

// ============ CHUNK =============

typedef struct _CHUNK
{
    TEXTURE* tex;
    uint32_t uv_size;
    uint32_t* u;
    uint32_t* v;
} CHUNK;

#endif