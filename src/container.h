/* 
 * BSP v30 viewer
 * 
 *
 * Autor: Enchanted Hunter
 */
 
#ifndef CONTAINER_H
#define CONTAINER_H

#include <iostream>

typedef struct _VECTOR{
    uint32_t size;
    uint32_t capacity;
    uint32_t elementSize;
    unsigned char* data;
} VECTOR;

typedef struct _LINKEDLIST{
    uint32_t size;
    unsigned char* data;
    struct _LINKEDLIST* next;
} LINKEDLIST;

VECTOR* initVector(uint32_t capacity, uint32_t elementSize);
void initVector(VECTOR* vec, uint32_t capacity, uint32_t elementSize);
void addVector(VECTOR* vec, int val);
void addVector(VECTOR* vec, float val);
void addVector(VECTOR* vec, uint32_t val);
void addVector(VECTOR* vec, size_t val);
void destroyVector(VECTOR* vec);

#endif