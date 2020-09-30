/* 
 * BSP v30 viewer
 * 
 *
 * Autor: Enchanted Hunter
 */
 
#ifndef UTILS_H
#define UTILS_H

#include <iostream>

#include "debug.h"

unsigned char* readBspFile(const char* file);
unsigned char* readFile(const char* file, size_t* size);
void writeBytesFile(unsigned char* data, char* file, int counts);

#endif