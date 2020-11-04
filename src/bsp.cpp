/* 
 * BSP v30 viewer
 * 
 *
 * Autor: Enchanted Hunter
 */
 
#include "bsp.h"
#include "container.h"
#include "utils.h"
#include "wad.h"

#include <math.h> 
#include <string.h>

void splitModelByMaterial(VECTOR* chunks, uint32_t texId, uint32_t size, uint32_t offset, VECTOR* light_maps, CHUNK* curr_chunk){
    
    addVector(((VECTOR*)light_maps->data) + texId, *curr_chunk);

    if( (chunks + texId)->size == 0){

        addVector((chunks + texId), size);
        addVector((chunks + texId), offset);
        return;
    }

    uint32_t lastOffs = *(((uint32_t*)(chunks + texId)->data) + (chunks + texId)->size - 1);
    uint32_t lastSize = *(((uint32_t*)(chunks + texId)->data) + (chunks + texId)->size - 2);
    size_t ptr = lastOffs + lastSize;

    if(offset == ptr){

        *(((uint32_t*)(chunks + texId)->data) + (chunks + texId)->size - 2) += size;
    }else{

        addVector((chunks + texId), size);
        addVector((chunks + texId), offset);
    }
}

TEXTURE* makeLightMapsAtlas(VECTOR* lms, VECTOR* vertexes){

    uint32_t m_width = 0;
    uint32_t m_height = 0;

    for(uint32_t i = 0; i < lms->size ; i++){

        CHUNK* clm = ((CHUNK*)lms->data) + i;
        TEXTURE* lm = clm->tex;

        if( lm->iWidth > m_width )
            m_width = lm->iWidth;

        if( lm->iHeight > m_height )
            m_height = lm->iHeight;
    }

    m_width += 2;
    m_height += 2;

    uint32_t lx = 0;
    uint32_t ly = 0;

    for(uint32_t i = 0 ; i < lms->size ; i ++){
        
        lx++;
        if(lx * ly >= lms->size)
            break;
        
        ly++;
        if(lx * ly >= lms->size)
            break;
    }

    TEXTURE* atl = (TEXTURE*)malloc( sizeof(TEXTURE));
    
    unsigned char* atlas = (unsigned char*)malloc( lx * m_width * ly * m_height * 3 * sizeof(unsigned char));
    memset(atlas, 0x00, lx * m_width * ly * m_height * 3 * sizeof(unsigned char));

    atl->data = atlas;
    atl->iWidth = lx * m_width;
    atl->iHeight = ly * m_height;
    
    if(lms->size == 0){
        atl->data = (unsigned char*)malloc(3);
        atl->iWidth = 1;
        atl->iHeight = 1;
        memset(atl->data, 0x00, 3 * sizeof(unsigned char));
        return atl;
    }

    for(uint32_t i = 0; i < lms->size ; i++){

        CHUNK* clm = ((CHUNK*)lms->data) + i;
        TEXTURE* lm = clm->tex;

        uint32_t px = ( i + lx ) % lx ;
        uint32_t py = i / lx + 1;
        
        for(uint32_t ii = 0 ; ii < clm->uv_size ; ii++){
            
            float u = *( ((float*)vertexes->data) + *(((uint32_t*)clm->u) + ii) );
            float v = *( ((float*)vertexes->data) + *(((uint32_t*)clm->v) + ii) );

            float xu = ((int)(u * ((float)lm->iWidth) + ((float)(px)) * m_width + 1 )) / ((float)atl->iWidth);
            float xv = ((int)(v * ((float)lm->iHeight) + ((float)(py - 1)) * m_height + 1 )) / ((float)atl->iHeight) ;

            *( ((float*)vertexes->data) + *(((uint32_t*)clm->u) + ii) ) = xu;
            *( ((float*)vertexes->data) + *(((uint32_t*)clm->v) + ii) ) = xv;
        }

        for(uint32_t x = 0; x < lm->iHeight ; x++){

            uint32_t sft = ( py - 1 )* lx * m_width * 3 * m_height + (px) * m_width * 3 + lx * m_width * 3 * x + lx * m_width * 3 * 1 + 3;
            uint32_t lm_sft = (x) * lm->iWidth * 3;

            memcpy(atlas + sft, lm->data + lm_sft, lm->iWidth * 3);

            //fill nearest pixels in atlas for gl_linear
            memcpy(atlas + sft + lm->iWidth * 3 , lm->data + lm_sft + lm->iWidth * 3, 3);
            memcpy(atlas + sft - 3, lm->data + lm_sft , 3);
        }
        
        //fill nearest pixels in atlas for gl_linear
        uint32_t sft = ( py - 1 )* lx * m_width * 3 * m_height + (px) * m_width * 3 + 3 ;
        memcpy(atlas + sft, lm->data , lm->iWidth * 3);
        sft = ( py - 1 )* lx * m_width * 3 * m_height + (px) * m_width * 3 + lx * m_width * 3 * (lm->iHeight - 1) + lx * m_width * 3 * 1 + 3;
        memcpy(atlas + sft, lm->data + (lm->iHeight - 1) * lm->iWidth * 3, lm->iWidth * 3);
    }

    return atl;
}

void loadVertexesIndexes(unsigned char* data, unsigned char** vertexes, uint32_t* verts_count, VECTOR** modelChunks, uint32_t* textures_count, unsigned char** indexes, uint32_t* indexes_count, TEXTURE* texturesRaw, uint32_t texturesCount, TEXTURE** lightMapAtl, uint32_t* lightMapAtlCount){

    uint32_t verticesCount;
    BSPVERTEX* vertices = getVertices(data, &verticesCount);

    uint32_t textureInfoCount;
    BSPTEXTUREINFO* texturesInfo = getTextureInfo(data, &textureInfoCount);

    uint32_t modelsCount;
    BSPMODEL* models = getModels(data, &modelsCount);

    uint32_t surfaceEdgesCount;
    BSPSURFEDGE* surfaceEdges = getSurfaceEdges(data, &surfaceEdgesCount);

    uint32_t facesCount = 0;
    BSPFACE* faces = getFaces(data, &facesCount);

    uint32_t edgesCount = 0;
    BSPEDGE* edges = getEdges(data, &edgesCount);

    uint32_t idx = 0;

    VECTOR* light_maps = initVector(texturesCount, sizeof(VECTOR));       //need array
    for(uint32_t i = 0; i < texturesCount; i++)                           //==========
        initVector( ((VECTOR*)light_maps->data) + i, 16, sizeof(CHUNK));  //==========

    VECTOR* light_atlases = initVector(16, sizeof(TEXTURE));

    VECTOR* indexesVec = initVector(16, sizeof(int));
    VECTOR* newIndicesTri = initVector(16, sizeof(int));
    VECTOR* newVertexesTri = initVector(16, sizeof(float));

    VECTOR* chunks = (VECTOR*)malloc(texturesCount*sizeof(VECTOR)); 
    for(uint32_t i = 0; i < texturesCount; i++)
        initVector(chunks+i, 16, sizeof(uint32_t));

    BSPHEADER* header = (BSPHEADER*)data;
    unsigned char* light  = (unsigned char*)(data + (header->lump + LUMP_LIGHTING)->nOffset);

    for (uint32_t m = 0; m < modelsCount; m++)
    for (int32_t f = 0; f < (models + m)->nFaces; f++){
    
        int32_t firstFace = (models + m)->iFirstFace;
        BSPFACE* face = (faces + f + firstFace);
        BSPTEXTUREINFO textureInfo = *(texturesInfo + face->iTextureInfo);
        
        uint32_t fptr = indexesVec->size;

        float minU = 999999.0f;
        float minV = 999999.0f;
        float maxU = -999999.0f;
        float maxV = -999999.0f;

        for (uint32_t ei = 0; ei < face->nEdges; ei++)
        {
            int se = *(surfaceEdges + face->iFirstEdge + ei);
            BSPEDGE edge = *(edges + abs(se));
            int index = se >= 0 ? *(edge.iVertex + 0) : *(edge.iVertex + 1);
            addVector(indexesVec, index);

            float x = (vertices + index)->x;
            float y = (vertices + index)->y;
            float z = (vertices + index)->z;

            float u = (textureInfo.vS.x * x + textureInfo.vS.y * y + textureInfo.vS.z * z + textureInfo.fSShift);
            float v = (textureInfo.vT.x * x + textureInfo.vT.y * y + textureInfo.vT.z * z + textureInfo.fTShift);

    		if (u < minU)
				minU = u;
			if (u > maxU)
				maxU = u;

            if (v < minV)
				minV = v;
			if (v > maxV)
				maxV = v;
        }

        uint32_t p0 = *((int*)indexesVec->data + fptr);
        uint32_t pp00 = newIndicesTri->size;

        float texMinU = (float)floor(minU / 16.0);
        float texMinV = (float)floor(minV / 16.0);
        float texMaxU = (float)ceil(maxU / 16.0);
        float texMaxV = (float)ceil(maxV / 16.0);

        uint32_t width = floor((texMaxU - texMinU) + 1);
        uint32_t height = floor((texMaxV - texMinV) + 1);

        float midPolyU = (minU + maxU) / 2.0;
        float midPolyV = (minV + maxV) / 2.0;
        float midTexU = width / 2.0;
        float midTexV = height / 2.0;

        CHUNK* chunk = (CHUNK*)malloc(sizeof(CHUNK));
        chunk->u = (uint32_t*)malloc(350*sizeof(uint32_t)); // need vector 
        chunk->v = (uint32_t*)malloc(350*sizeof(uint32_t)); // ===========
        chunk->uv_size = 0;

        if(face->nLightmapOffset <= (header->lump + LUMP_LIGHTING)->nLength){
            
            unsigned char* currLight = light + face->nLightmapOffset;
            unsigned char* lightMap = (unsigned char*) malloc(width*height*3*sizeof(unsigned char));
            memcpy(lightMap, currLight, width*height*3*sizeof(unsigned char));

            TEXTURE* tex = (TEXTURE*)malloc(sizeof(TEXTURE));

            tex->data = lightMap;
            tex->iWidth = width;
            tex->iHeight = height;
            chunk->tex = tex;

        }
        else{
            unsigned char* dummy = (unsigned char*) malloc(width*height*3*sizeof(unsigned char));
            memset(dummy, 0x0f, width*height*3*sizeof(unsigned char));

            TEXTURE* tex = (TEXTURE*)malloc(sizeof(TEXTURE));

            tex->data = dummy;
            tex->iWidth = width;
            tex->iHeight = height;
            chunk->tex = tex;
        }

        for (int i = 2; i < face->nEdges; i++)
        {
            uint32_t w = (texturesRaw + textureInfo.iMiptex)->iWidth;
            uint32_t h = (texturesRaw + textureInfo.iMiptex)->iHeight;

            addVector(newIndicesTri, idx++);
            addVector(newIndicesTri, idx++);
            addVector(newIndicesTri, idx++);

            float x = (vertices + p0)->x;
            float y = (vertices + p0)->y;
            float z = (vertices + p0)->z;

            float u = (textureInfo.vS.x * x + textureInfo.vS.y * y + textureInfo.vS.z * z + textureInfo.fSShift);
            float v = (textureInfo.vT.x * x + textureInfo.vT.y * y + textureInfo.vT.z * z + textureInfo.fTShift);

            float lightMapU = midTexU + (u - midPolyU) / 16.0;
			float lightMapV = midTexV + (v - midPolyV) / 16.0;

            addVector(newVertexesTri, x * 0.01f );
            addVector(newVertexesTri, y * 0.01f );
            addVector(newVertexesTri, z * 0.01f );
            addVector(newVertexesTri, u/w);
            addVector(newVertexesTri, v/h);
            addVector(newVertexesTri, lightMapU/width);
            addVector(newVertexesTri, lightMapV/height);

            *( ((uint32_t*)chunk->u) + chunk->uv_size  ) = newVertexesTri->size - 2;
            *( ((uint32_t*)chunk->v) + chunk->uv_size  ) = newVertexesTri->size - 1;

            chunk->uv_size++;

            x = (vertices + *(((int*)indexesVec->data) + fptr + i - 1))->x;
            y = (vertices + *(((int*)indexesVec->data) + fptr + i - 1))->y;
            z = (vertices + *(((int*)indexesVec->data) + fptr + i - 1))->z;

            u = (textureInfo.vS.x * x + textureInfo.vS.y * y + textureInfo.vS.z * z + textureInfo.fSShift);
            v = (textureInfo.vT.x * x + textureInfo.vT.y * y + textureInfo.vT.z * z + textureInfo.fTShift);

            lightMapU = midTexU + (u - midPolyU) / 16.0;
			lightMapV = midTexV + (v - midPolyV) / 16.0;

            addVector(newVertexesTri, x * 0.01f );
            addVector(newVertexesTri, y * 0.01f );
            addVector(newVertexesTri, z * 0.01f );
            addVector(newVertexesTri, u/w);
            addVector(newVertexesTri, v/h);
            addVector(newVertexesTri, lightMapU/width);
            addVector(newVertexesTri, lightMapV/height);

            *( ((uint32_t*)chunk->u) + chunk->uv_size ) = newVertexesTri->size - 2;
            *( ((uint32_t*)chunk->v) + chunk->uv_size ) = newVertexesTri->size - 1;

            chunk->uv_size++;

            x = (vertices + *((int*)indexesVec->data + fptr + i))->x;
            y = (vertices + *((int*)indexesVec->data + fptr + i))->y;
            z = (vertices + *((int*)indexesVec->data + fptr + i))->z;

            u = (textureInfo.vS.x * x + textureInfo.vS.y * y + textureInfo.vS.z * z + textureInfo.fSShift);
            v = (textureInfo.vT.x * x + textureInfo.vT.y * y + textureInfo.vT.z * z + textureInfo.fTShift);

            lightMapU = midTexU + (u - midPolyU) / 16.0;
			lightMapV = midTexV + (v - midPolyV) / 16.0;

            addVector(newVertexesTri, x * 0.01f );
            addVector(newVertexesTri, y * 0.01f );
            addVector(newVertexesTri, z * 0.01f );
            addVector(newVertexesTri, u/w);
            addVector(newVertexesTri, v/h);
            addVector(newVertexesTri, lightMapU/width);
            addVector(newVertexesTri, lightMapV/height);

            *( ((uint32_t*)chunk->u) + chunk->uv_size ) = newVertexesTri->size - 2;
            *( ((uint32_t*)chunk->v) + chunk->uv_size ) = newVertexesTri->size - 1;
            
            chunk->uv_size++;
        }

        splitModelByMaterial(chunks, textureInfo.iMiptex, newIndicesTri->size - pp00, pp00, light_maps, chunk);
    }

    light_maps->size = texturesCount;
    
    for(uint32_t idx = 0 ; idx < light_maps->size ; idx++){
        
        VECTOR* lm_cont = ((VECTOR*)light_maps->data) + idx;
        TEXTURE* atl = makeLightMapsAtlas(lm_cont, newVertexesTri);

        addVector(light_atlases, *atl);
    }

    *lightMapAtl = (TEXTURE*)light_atlases->data;
    *lightMapAtlCount = light_maps->size;

    *vertexes = newVertexesTri->data;
    *verts_count = newVertexesTri->size;

    *modelChunks = chunks;
    *textures_count = texturesCount;

    *indexes_count = newIndicesTri->size;
    *indexes = newIndicesTri->data;
}

void makeTexture(TEXTURE* texture, void * dataPtr, void * palettePtr)
{
    unsigned char indices[texture->iWidth * texture->iHeight];
    memcpy(indices, dataPtr, texture->iWidth * texture->iHeight);

    unsigned char palette[256*3];
    memcpy(palette, palettePtr, 256*3);

    for(uint32_t i = 0 ; i < texture->iWidth * texture->iHeight ; i++){
        unsigned char* paletteIdx = palette + ((unsigned char)(indices[i])*3);
        memcpy(texture->data + i*4, paletteIdx, 3);
                    
        *(texture->data + i*4+3) = 0xff;

        if ( texture->name[0] == '!' || strncmp (texture->name, "glass", 5) == 0){
            *(texture->data + i*4+3) = 0x80;
            texture->transparent_type = 0x01;
        }

        if ( strcmp (texture->name, "aaatrigger") == 0){
            *(texture->data + i*4+3) = 0x00;
        }

        if(indices[i] == 255)
        if( *((unsigned char*)(texture->data + i*4 + 0)) == 0x0 &&
            *((unsigned char*)(texture->data + i*4 + 1)) == 0x0  &&
            *((unsigned char*)(texture->data + i*4 + 2)) == 0xff)
        {
            *(texture->data + i*4+2) = 0x00;
            *(texture->data + i*4+3) = 0x00;
        }
    }
}

void textureInWadFind(LINKEDLIST* wadsDataFirst, TEXTURE* texture){

    LINKEDLIST* wadsData = wadsDataFirst;
    
    while(wadsData->size != 0){

        unsigned char* wadFile = wadsData->data;
        wadsData = wadsData->next;

        WADHEADER* header = (WADHEADER*)wadFile;
        WADDIRENTRY* direntries = (WADDIRENTRY*)(wadFile + header->nDirOffset);

        for(int32_t i = 0 ; i < header->nDir ; i++){

            WADDIRENTRY * dir = direntries + i;
            BSPMIPTEXWAD* tex = (BSPMIPTEXWAD*)(wadFile + dir->nFilePos);

            if (strcmp (texture->name, tex->szName) == 0 && tex->nWidth == texture->iWidth && tex->nHeight == texture->iHeight){
#ifdef DEBUG_LEVEL_2
                printf("found texture in wad file : %s\n", tex->szName);
#endif
                makeTexture(texture, 
                    (unsigned char*)wadFile + dir->nFilePos + *(tex->nOffsets + 0), 
                    (unsigned char*)wadFile + dir->nFilePos + *(tex->nOffsets + 3) + ((tex->nWidth/8) * (tex->nHeight/8)) + 2);
                return;
            }
        }
    }
    
    return;
}

TEXTURE* loadTextures(unsigned char* data, uint32_t* count){
    
    BSPHEADER* header = (BSPHEADER*)data;

    char wadWord[] = ".wad";
    char * entities = (char*)data + (header->lump + LUMP_ENTITIES)->nOffset;
    char* end = NULL;

    LINKEDLIST* wadsData = (LINKEDLIST*)malloc(sizeof(LINKEDLIST));
    LINKEDLIST* wadFirst = wadsData;

    while((end = strstr(entities, wadWord)) != NULL){
        for(size_t ii = 0 ; ii < 100; ii++){
            if(*(end - ii) == '\\' || *(end - ii) == ' ' || *(end - ii) == ';' || *(end - ii) == '"'){
                
                size_t offset = (size_t)( end - entities - ii + 1);
                size_t size = (size_t)( ii +  sizeof(wadWord) - 2);
                
                char wadName[size + 1];
                memcpy(wadName, entities + offset, size);
                wadName[size] = '\0';

                size_t wadSize;
                unsigned char* wad = readFile(wadName, &wadSize);
                if(wad != NULL){
                    wadsData->data = wad;
                    wadsData->size = wadSize;
                    wadsData->next = (LINKEDLIST*)malloc(sizeof(LINKEDLIST));
                    wadsData->next->size = 0;
                    wadsData = wadsData->next;
#ifdef DEBUG_LEVEL_2
                    printf("file %s exists\n", wadName);
#endif
                }else{
#ifdef DEBUG_LEVEL_2
                    printf("error: file %s not found\n", wadName);
#endif
                }
                
                break;
            }
        }

        entities = end + sizeof(wadWord);
    }

    entities = (char*)data + (header->lump + LUMP_ENTITIES)->nOffset;

    size_t lumpTextureOffset = (header->lump + LUMP_TEXTURES)->nOffset;
    
    BSPTEXTUREHEADER* textureHeader = (BSPTEXTUREHEADER*)(data + lumpTextureOffset);
    BSPMIPTEXOFFSET* texturesOffsets = (BSPMIPTEXOFFSET*)(data + lumpTextureOffset + sizeof(BSPTEXTUREHEADER));

    TEXTURE* textureArray = (TEXTURE*)malloc(textureHeader->nMipTextures*sizeof(TEXTURE));
    *count = textureHeader->nMipTextures;
    
    for(uint32_t i = 0; i < textureHeader->nMipTextures ; i++ ){

        BSPMIPTEX* bspTex = (BSPMIPTEX*)(data + lumpTextureOffset + *(texturesOffsets + i));

        if(bspTex->nWidth > 1024 || bspTex->nHeight > 1024){

            //if bad texture size 
            TEXTURE* texture = (TEXTURE*)malloc(sizeof(TEXTURE));
            texture->iWidth = 1;
            texture->iHeight = 1;
            texture->data = (unsigned char*)malloc(1 * 1 * 4);
            *(textureArray + i) = *texture;
#ifdef DEBUG_LEVEL_2
            printf("Error: texture [%s] wrong size %d %d\n", bspTex->szName , bspTex->nWidth, bspTex->nHeight);
#endif
            continue;
        }
        
        unsigned char* image = (unsigned char *)malloc(bspTex->nWidth * bspTex->nHeight * 4);

        TEXTURE* texture = (TEXTURE*)malloc(sizeof(TEXTURE));
        texture->transparent_type = 0x0;
        
        if(bspTex->nOffsets[0] == 0 && 
            bspTex->nOffsets[1] == 0 && 
            bspTex->nOffsets[2] == 0 && 
            bspTex->nOffsets[3] == 0){

            texture->name = bspTex->szName;
            texture->data = image;
            texture->iWidth = bspTex->nWidth;
            texture->iHeight = bspTex->nHeight;

            textureInWadFind(wadFirst, texture);

        }else{
            
            texture->name = bspTex->szName;
            texture->data = image;
            texture->iWidth = bspTex->nWidth;
            texture->iHeight = bspTex->nHeight;

            makeTexture(texture,
                data + lumpTextureOffset + *(texturesOffsets + i) + *(bspTex->nOffsets + 0),
                data + lumpTextureOffset + *(texturesOffsets + i) + *(bspTex->nOffsets + 3) + ((bspTex->nWidth/8) * (bspTex->nHeight/8)) + 2);

        }
  
        *(textureArray + i) = *texture;
    }

    return textureArray;
}

BSPTEXTUREINFO* getTextureInfo(unsigned char* data, uint32_t* count){
    BSPHEADER* header = (BSPHEADER*)data;
    *count = (header->lump + LUMP_TEXINFO)->nLength / sizeof(BSPVERTEX);
    return (BSPTEXTUREINFO*)(data + (header->lump + LUMP_TEXINFO)->nOffset);
}

BSPVERTEX* getVertices(unsigned char* data, uint32_t* count){
    BSPHEADER* header = (BSPHEADER*)data;
    *count = (header->lump + LUMP_VERTICES)->nLength / sizeof(BSPVERTEX);
    return (BSPVERTEX*)(data + (header->lump + LUMP_VERTICES)->nOffset);
}

BSPMARKSURFACE* getMarksurfaces(unsigned char* data, uint32_t* count){
    BSPHEADER* header = (BSPHEADER*)data;
    *count = (header->lump + LUMP_MARKSURFACES)->nLength / sizeof(BSPMARKSURFACE);
    return (BSPMARKSURFACE*)(data + (header->lump + LUMP_MARKSURFACES)->nOffset);
}

BSPFACE* getFaces(unsigned char* data, uint32_t* count){
    BSPHEADER* header = (BSPHEADER*)data;
    *count = (header->lump + LUMP_FACES)->nLength / sizeof(BSPFACE);
    return (BSPFACE*)(data + (header->lump + LUMP_FACES)->nOffset);
}

BSPSURFEDGE* getSurfaceEdges(unsigned char* data, uint32_t* count){
    BSPHEADER* header = (BSPHEADER*)data;
    *count = (header->lump + LUMP_SURFEDGES)->nLength / sizeof(BSPSURFEDGE);
    return (BSPSURFEDGE*)(data + (header->lump + LUMP_SURFEDGES)->nOffset);
}

BSPEDGE* getEdges(unsigned char* data, uint32_t* count){
    BSPHEADER* header = (BSPHEADER*)data;
    *count = (header->lump + LUMP_EDGES)->nLength / sizeof(BSPEDGE);
    return (BSPEDGE*)(data + (header->lump + LUMP_EDGES)->nOffset);
}

BSPMODEL* getModels(unsigned char* data, uint32_t* count){
    BSPHEADER* header = (BSPHEADER*)data;
    *count = (header->lump + LUMP_MODELS)->nLength / sizeof(BSPMODEL);
    return (BSPMODEL*)(data + (header->lump + LUMP_MODELS)->nOffset);
}