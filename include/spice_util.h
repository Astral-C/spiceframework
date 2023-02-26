#ifndef __SPICE_UTIL_H__
#define __SPICE_UTIL_H__
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum {
    SP_ERROR = -1,
    SP_SUCCESS = 1,
    SP_FAIL = 0
} sp_status;

#ifdef __WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\')+1)
#endif

#ifdef __linux__
#define __FILENAME__ (strrchr(__FILE__, '/')+1)
#endif

#define spice_error(msg, ...) fprintf(stderr, "[ERROR %s:L%d]: ", __FILENAME__, __LINE__); fprintf(stderr, msg __VA_OPT__(,) __VA_ARGS__);
#define spice_info(msg, ...) fprintf(stdout, "[INFO %s:L%d]: ", __FILENAME__, __LINE__); fprintf(stdout, msg __VA_OPT__(,) __VA_ARGS__);

#ifdef SPICE_DEBUG_PRINT
#define spice_debug(msg, ...) fprintf(stdout, "[DEBUG %s:L%d]: ", __FILENAME__, __LINE__); fprintf(stdout, msg __VA_OPT__(,) __VA_ARGS__);
#else
#define spice_debug(msg, ...) do {} while(0);
#endif

#define SPICE_KEYMAX 32
#define SPICE_STRMAX 512
#define sp_strcmp(a, b) (strncmp(a, b, SPICE_STRMAX) == 0)
#define sp_keycmp(a, b) (strncmp(a, b, SPICE_KEYMAX) == 0)
#define sp_strcpy(dest, src) strncpy(dest, src, SPICE_STRMAX)
#define sp_keycpy(dest, src) strncpy(dest, src, SPICE_KEYMAX)

#define SPICE_HASHMAP_INITIAL_CAPACITY 32
#define SPICE_HASHMAP_INITIAL_BUCKET_SIZE 32

typedef char sp_str[SPICE_STRMAX];
typedef char sp_key[SPICE_KEYMAX];

#define FNV_PRIME 0x100000001b3
#define FNV_OFFSET_BASIS 0xcbf29ce484222325

/////////////////////////////////////////
/// Allocation Functions 
/////////////////////////////////////////

//void* (*spiceAllocMem)(size_t sz);
//void* (*spiceFreeMem)(size_t sz);

uint64_t spHash(char* data, size_t size);

/////////////////////////////////////////
/// Hashmap Code
/////////////////////////////////////////

typedef struct {
    sp_str key;
    void* data;
} sp_hashmap_item;

typedef struct {
    uint32_t size;
    sp_hashmap_item* items;
} sp_hashmap_bucket;

typedef struct {
    uint32_t _bucket_count;
    sp_hashmap_bucket* buckets;
} sp_hashmap;

sp_hashmap* spHashmapNew();
void spHashmapFree(sp_hashmap* map);
void spHashmapFreeItems(sp_hashmap* map);

sp_status spHashmapAdd(sp_hashmap* map, char* key, void* data);
sp_status spHashmapRemove(sp_hashmap* map, char* key);
void* spHashmapGet(sp_hashmap* map, char* key);

#endif