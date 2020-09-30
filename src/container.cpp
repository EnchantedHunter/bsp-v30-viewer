/* 
 * BSP v30 viewer
 * 
 *
 * Autor: Enchanted Hunter
 */
 
#include "container.h"

VECTOR* initVector(uint32_t capacity, uint32_t elementSize){
    VECTOR* vec = (VECTOR*)malloc(sizeof(VECTOR));
    vec->size = 0;
    vec->elementSize = elementSize;
    vec->capacity = capacity;
    vec->data = (unsigned char*)malloc(capacity*elementSize);
    return vec;
}

void initVector(VECTOR* vec, uint32_t capacity, uint32_t elementSize){
    vec->size = 0;
    vec->elementSize = elementSize;
    vec->capacity = capacity;
    vec->data = (unsigned char*)malloc(capacity*elementSize);
}

void addVector(VECTOR* vec, int val){

    if( vec->size + 1 >= vec->capacity ){
        vec->capacity += 3 * vec->capacity/ 2;
        vec->data = (unsigned char*)realloc(vec->data, vec->capacity * vec->elementSize);
    }
    *( ((int*)vec->data) + vec->size ) = val;
    vec->size++;
}

void addVector(VECTOR* vec, float val){

    if( vec->size + 1 >= vec->capacity ){
        vec->capacity += 3 * vec->capacity/ 2;
        vec->data = (unsigned char*)realloc(vec->data, vec->capacity * vec->elementSize);
    }
    *( ((float*)vec->data) + vec->size) = val;    
    vec->size++;

}

void addVector(VECTOR* vec, uint32_t val){

    if( vec->size + 1>= vec->capacity ){
        vec->capacity += 3 * vec->capacity/ 2;
        vec->data = (unsigned char*)realloc(vec->data, vec->capacity * vec->elementSize);
    }
    *( ((uint32_t*)vec->data) + vec->size) = val;
    vec->size++;
}

void addVector(VECTOR* vec, size_t val){

    if( vec->size + 1>= vec->capacity ){
        vec->capacity += 3 * vec->capacity/ 2;
        vec->data = (unsigned char*)realloc(vec->data, vec->capacity * vec->elementSize);
    }
    *( ((size_t*)vec->data) + vec->size) = val;
    vec->size++;
}

void destroyVector(VECTOR* vec){
    free(vec->data);
    free(vec);
}