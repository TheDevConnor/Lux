#include <stdlib.h>
#include <stdio.h>

#include "memory.h"

/* Helper function to get maximum of two values */
static inline size_t max_size(size_t a, size_t b) {
    return (a > b) ? a : b;
}

/* Helper function to get minimum of two values */
static inline size_t min_size(size_t a, size_t b) {
    return (a < b) ? a : b;
}

/* Create a new buffer with specified size and alignment */
Buffer* buffer_create(size_t s, size_t alignment) {
    if (alignment == 0) alignment = 1024;
    
    alignment = min_size(s, alignment);
    /* Align the whole block to max(alignment, alignof(Buffer)) */
    size_t buffer_align = max_size(alignment, alignof(Buffer));
    size_t total_size = sizeof(Buffer) + s + buffer_align;

    void *raw = aligned_alloc(buffer_align, total_size);
    if (!raw) {
        return NULL;
    }

    /* Initialize the Buffer object */
    Buffer *buf = (Buffer*)raw;
    buf->size = s;
    buf->next = NULL;

    /* Compute aligned pointer for buffer data after Buffer struct */
    char *raw_bytes = (char*)raw;
    size_t struct_end = (size_t)(raw_bytes + sizeof(Buffer));
    size_t aligned_data = (struct_end + alignment - 1) & ~(alignment - 1);
    buf->ptr = (char*)aligned_data;

    return buf;
}

/* Initialize arena allocator */
int arena_allocator_init(ArenaAllocator *arena, size_t size) {
    if (!arena) return -1;
    
    arena->buffer = buffer_create(size, 1024);
    if (!arena->buffer) return -1;
    
    arena->head = arena->buffer;
    arena->offset = 0;
    return 0;
}

/* Allocate memory from arena */
void* arena_alloc(ArenaAllocator *arena, size_t size, size_t alignment) {
    if (!arena || !arena->buffer) return NULL;
    
    if (alignment == 0) alignment = alignof(max_align_t);

    if ((size > arena->buffer->size / 4) || (alignment > arena->buffer->size / 4)) {
        Buffer *new_buffer = buffer_create(size, alignment);
        if (!new_buffer) return NULL;
        
        new_buffer->next = arena->buffer->next;
        arena->buffer->next = new_buffer;
        return new_buffer->ptr;
    }

    while (1) {
        size_t aligned_offset = (arena->offset + (alignment - 1)) & ~(alignment - 1);

        if (aligned_offset + size > arena->buffer->size) {
            if (arena->buffer->next) {
                arena->buffer = arena->buffer->next;
                arena->offset = 0;
                continue;
            }
            arena->buffer->next = buffer_create(arena->buffer->size, 1024);
            if (!arena->buffer->next) return NULL;
            
            arena->buffer = arena->buffer->next;
            arena->offset = 0;
            continue;
        }

        void *ptr = arena->buffer->ptr + aligned_offset;
        arena->offset = aligned_offset + size;
        return ptr;
    }
}

/* Reset arena to initial state (reuse allocated buffers) */
void arena_reset(ArenaAllocator *arena) {
    if (!arena) return;
    
    arena->offset = 0;
    arena->buffer = arena->head;
}

/* Free all memory and cleanup arena */
void arena_destroy(ArenaAllocator *arena) {
    if (!arena) return;
    
    arena_reset(arena);
    Buffer *current = arena->buffer;
    
    while (current != NULL) {
        Buffer *next = current->next;
        free(current);
        current = next;
    }
    
    arena->buffer = NULL;
    arena->head = NULL;
    arena->offset = 0;
}

#ifdef __cplusplus
#endif