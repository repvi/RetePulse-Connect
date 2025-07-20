#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Configurable settings
#define HASHMAP_SIZE 8          // Must be power of 2
#define MAX_KEY_LENGTH 32
#define HASH_SEED 0x12345678    // Initial hash seed

typedef struct hashmap_t *HashMap;

HashMap hashmap_create(void);

bool hashmap_put(HashMap map, const char* key, void* value);

void* hashmap_get(HashMap map, const char* key);

bool hashmap_remove(HashMap map, const char* key);

#ifdef __cplusplus
}
#endif