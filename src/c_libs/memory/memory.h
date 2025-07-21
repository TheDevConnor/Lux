#pragma once

#include <stddef.h>
#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Buffer {
    size_t size;
    struct Buffer *next;
    char *ptr;
} Buffer;

typedef struct ArenaAllocator {
    size_t offset;
    Buffer *buffer;
    Buffer *head;
} ArenaAllocator;

/* Buffer management functions */
Buffer* buffer_create(size_t s, size_t alignment);

/* Arena allocator functions */
int arena_allocator_init(ArenaAllocator *arena, size_t size);
void* arena_alloc(ArenaAllocator *arena, size_t size, size_t alignment);
void arena_reset(ArenaAllocator *arena);
void arena_destroy(ArenaAllocator *arena);

#ifdef __cplusplus
}
#endif