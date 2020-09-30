/* 
 * BSP v30 viewer
 * 
 *
 * Autor: Enchanted Hunter
 */
 
#ifndef BSP_H
#define BSP_H

#include "debug.h"
#include "container.h"

// ========== VECTOR3D ===========

typedef struct _VECTOR3D
{
    float x, y, z;
} VECTOR3D;

// ============ LUMPs =============

#define LUMP_ENTITIES      0
#define LUMP_PLANES        1
#define LUMP_TEXTURES      2
#define LUMP_VERTICES      3
#define LUMP_VISIBILITY    4
#define LUMP_NODES         5
#define LUMP_TEXINFO       6
#define LUMP_FACES         7
#define LUMP_LIGHTING      8
#define LUMP_CLIPNODES     9
#define LUMP_LEAVES       10
#define LUMP_MARKSURFACES 11
#define LUMP_EDGES        12
#define LUMP_SURFEDGES    13
#define LUMP_MODELS       14
#define HEADER_LUMPS      15

typedef struct _BSPLUMP
{
    int32_t nOffset; // File offset to data
    int32_t nLength; // Length of data
} BSPLUMP;

// ============ BSPHEADER =============

typedef struct _BSPHEADER
{
    int32_t nVersion;           // Must be 30 for a valid HL BSP file
    BSPLUMP lump[HEADER_LUMPS]; // Stores the directory of lumps
} BSPHEADER;

#define MAX_MAP_HULLS        4

#define MAX_MAP_MODELS       400
#define MAX_MAP_BRUSHES      4096
#define MAX_MAP_ENTITIES     1024
#define MAX_MAP_ENTSTRING    (128*1024)

#define MAX_MAP_PLANES       32767
#define MAX_MAP_NODES        32767
#define MAX_MAP_CLIPNODES    32767
#define MAX_MAP_LEAFS        8192
#define MAX_MAP_VERTS        65535
#define MAX_MAP_FACES        65535
#define MAX_MAP_MARKSURFACES 65535
#define MAX_MAP_TEXINFO      8192
#define MAX_MAP_EDGES        256000
#define MAX_MAP_SURFEDGES    512000
#define MAX_MAP_TEXTURES     512
#define MAX_MAP_MIPTEX       0x200000
#define MAX_MAP_LIGHTING     0x200000
#define MAX_MAP_VISIBILITY   0x200000

#define MAX_MAP_PORTALS     65536

#define MAX_KEY     32
#define MAX_VALUE   1024

// ============ TEXTURES =============

#define MAXTEXTURENAME 16
#define MIPLEVELS 4

typedef struct _BSPMIPTEX
{
    char szName[MAXTEXTURENAME];  // Name of texture
    uint32_t nWidth, nHeight;     // Extends of the texture
    uint32_t nOffsets[MIPLEVELS]; // Offsets to texture mipmaps BSPMIPTEX;
} BSPMIPTEX;

typedef int32_t BSPMIPTEXOFFSET;

typedef struct _BSPTEXTUREHEADER
{
    uint32_t nMipTextures; // Number of BSPMIPTEX structures
} BSPTEXTUREHEADER;
 
// ========= VERTICES ===========

typedef VECTOR3D BSPVERTEX;

// =========== FACES ============

typedef struct _BSPFACE
{
    uint16_t iPlane;          // Plane the face is parallel to
    uint16_t nPlaneSide;      // Set if different normals orientation
    uint32_t iFirstEdge;      // Index of the first surfedge
    uint16_t nEdges;          // Number of consecutive surfedges
    uint16_t iTextureInfo;    // Index of the texture info structure
    uint8_t nStyles[4];       // Specify lighting styles
    uint32_t nLightmapOffset; // Offsets into the raw lightmap data
} BSPFACE;

// ======== MARKSURFACE =========

typedef uint16_t BSPMARKSURFACE;

// ======== SURFEDGES =========

typedef int32_t BSPSURFEDGE;

// ======== LUMP_EDGES =========

typedef struct _BSPEDGE
{
    uint16_t iVertex[2];
} BSPEDGE;

// ======== LUMP_MODELS ======== 

#define MAX_MAP_HULLS 4

typedef struct _BSPMODEL
{
    float nMins[3], nMaxs[3];          // Defines bounding box
    VECTOR3D vOrigin;                  // Coordinates to move the // coordinate system
    int32_t iHeadnodes[MAX_MAP_HULLS]; // Index into nodes array
    int32_t nVisLeafs;                 // ???
    int32_t iFirstFace, nFaces;        // Index and count into faces
} BSPMODEL;

// =========== LUMP_TEXINFO ============

typedef struct _BSPTEXTUREINFO
{
    VECTOR3D vS; 
    float fSShift;    // Texture shift in s direction
    VECTOR3D vT; 
    float fTShift;    // Texture shift in t direction
    uint32_t iMiptex; // Index into textures array
    uint32_t nFlags;  // Texture flags, seem to always be 0
} BSPTEXTUREINFO;

// =========== TEXTURE ============

typedef struct _TEXTURE
{
    uint32_t iWidth;
    uint32_t iHeight;
    unsigned char * data;
} TEXTURE;

// =========== VBO Chunk ============

typedef struct _VBOCHUNK
{
    uint32_t size;
    uint32_t offset;
} VBOCHUNK;

// =========== FUNCS ============

BSPVERTEX* getVertices(unsigned char* data, uint32_t* count);
BSPMARKSURFACE* getMarksurfaces(unsigned char* data, uint32_t* count);
BSPFACE* getFaces(unsigned char* data, uint32_t* count);
BSPSURFEDGE* getSurfaceEdges(unsigned char* data, uint32_t* count);
BSPEDGE* getEdges(unsigned char* data, uint32_t* count);
BSPMODEL* getModels(unsigned char* data, uint32_t* count);
BSPTEXTUREINFO* getTextureInfo(unsigned char* data, uint32_t* count);

TEXTURE* loadLightMap(unsigned char* data, uint32_t* count);
TEXTURE* loadTextures(unsigned char* data, uint32_t* count);
void loadVertexesIndexes(unsigned char* data, unsigned char** vertexes, uint32_t* verts_count, VECTOR** models, uint32_t* textures_count, unsigned char** indexes, uint32_t* indexes_count, TEXTURE* texturesRaw, uint32_t texturesCount);

#endif