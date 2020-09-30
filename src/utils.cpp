/* 
 * BSP v30 viewer
 * 
 *
 * Autor: Enchanted Hunter
 */

#include "utils.h"

unsigned char* readFile(const char* file, size_t* size){
	unsigned char *source = NULL;
	FILE *fp = fopen(file, "r");
	if (fp != NULL) {

		if (fseek(fp, 0L, SEEK_END) == 0) {

			uint64_t bufsize = ftell(fp);
			if (bufsize == -1) { 
				fclose(fp);
				return NULL;
			}else{
				*size = bufsize;
			}

			source = (unsigned char *)malloc(sizeof(char) * (bufsize + 1));

			if (fseek(fp, 0L, SEEK_SET) != 0) { 
				fclose(fp);
				return NULL;
			}

			size_t newLen = fread(source, sizeof(char), bufsize, fp);
			if ( ferror( fp ) != 0 ) {
				fputs("Error reading file", stderr);
			} else {
				source[newLen++] = '\0';
			}
		}
		fclose(fp);
	}
	return source;
}

unsigned char * readBspFile(const char * file){
	unsigned char *source = NULL;
	FILE *fp = fopen(file, "r");
	if (fp != NULL) {

		if (fseek(fp, 0L, SEEK_END) == 0) {

			uint64_t bufsize = ftell(fp);
			if (bufsize == -1) { 
				// Error 
				fclose(fp);
				return NULL;
			}else{
#ifdef DEBUG_LEVEL_2
				printf("read : %u bytes\n", bufsize);
#endif
			}

			source = (unsigned char *)malloc(sizeof(char) * (bufsize + 1));

			if (fseek(fp, 0L, SEEK_SET) != 0) { 
				// Error 
				fclose(fp);
				return NULL;
			}

			size_t newLen = fread(source, sizeof(char), bufsize, fp);
			if ( ferror( fp ) != 0 ) {
				fputs("Error reading file", stderr);
			} else {
				source[newLen++] = '\0';
			}
		}
		fclose(fp);
	}
	return source;
}

void writeBytesFile(unsigned char* data, char* file, int counts){

    FILE *f = fopen(file, "wb");
    
    int results = fwrite (data , sizeof(char), counts, f);

    if (results == EOF) {
        fputs("Error write file", stderr);
    }
    fclose(f);

}