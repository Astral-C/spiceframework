#include <spice_util.h>

uint64_t spHash(char* data, size_t size){
    uint64_t hash = FNV_OFFSET_BASIS;
    for (char* byte = data; byte < data + size; byte++){
        hash = hash ^ (*byte);
        hash = hash * FNV_PRIME;
    }
    return hash;
}

sp_hashmap* spHashmapNew(){
    sp_hashmap* hashmap = (sp_hashmap*)malloc(sizeof(sp_hashmap));
    hashmap->_bucket_count = SPICE_HASHMAP_INITIAL_CAPACITY;
    hashmap->buckets = (sp_hashmap_bucket*)malloc(sizeof(sp_hashmap_bucket) * SPICE_HASHMAP_INITIAL_CAPACITY);
    memset(hashmap->buckets, 0, sizeof(sp_hashmap_bucket) * SPICE_HASHMAP_INITIAL_CAPACITY);

    for (size_t i = 0; i < SPICE_HASHMAP_INITIAL_CAPACITY; i++){
        hashmap->buckets[i].size = SPICE_HASHMAP_INITIAL_BUCKET_SIZE;
        hashmap->buckets[i].items = (sp_hashmap_item*)malloc(sizeof(sp_hashmap_item) * SPICE_HASHMAP_INITIAL_BUCKET_SIZE);
        memset(hashmap->buckets[i].items, 0, sizeof(sp_hashmap_item) * SPICE_HASHMAP_INITIAL_BUCKET_SIZE);
    }
    return hashmap;
}

void spHashmapFree(sp_hashmap* map){
    for (size_t i = 0; i < map->_bucket_count; i++){
        free(map->buckets[i].items);
    }
    free(map->buckets);
    free(map);
}

sp_status spHashmapAdd(sp_hashmap* map, char* key, void* data){
    uint64_t bucket_idx = spHash(key, strlen(key)) & (map->_bucket_count - 1);
    
    printf("Bucket index is %d/%d\n", bucket_idx, map->_bucket_count);
    sp_hashmap_bucket* bucket = &map->buckets[bucket_idx];

    for (size_t item = 0; item < bucket->size; item++){
        printf("Checking for empty on key \"%s\"\n", bucket->items[item].key);
        if(sp_strcmp(bucket->items[item].key, "")){
            sp_strcpy(bucket->items[item].key, key);
            bucket->items[item].data = data;
            break;
        }
    }
    
    return SUCCESS;
}

sp_status spHashmapRemove(sp_hashmap* map, sp_str key){
    uint64_t bucket_idx = spHash(key, strlen(key)) & (map->_bucket_count - 1);
    
    sp_hashmap_bucket* bucket = &map->buckets[bucket_idx];

    for (size_t item = 0; item < bucket->size; item++){
        if(sp_strcmp( bucket->items[item].key, key)){
            memset(bucket->items[item].key, 0, sizeof(sp_str));
            // Data belongs to user, we shouldn't be freeing it, but I really want to.
            bucket->items[item].data = NULL;
            break;
        }
    }

    return SUCCESS;
}

void* spHashmapGet(sp_hashmap* map, sp_str key){
    uint64_t bucket_idx = spHash(key, strlen(key)) & (map->_bucket_count - 1);
    
    sp_hashmap_bucket* bucket = &map->buckets[bucket_idx];

    for (size_t item = 0; item < bucket->size; item++){
        if(sp_strcmp(bucket->items[item].key, key)){
            return bucket->items[item].data;
        }
    }

    return NULL;
}