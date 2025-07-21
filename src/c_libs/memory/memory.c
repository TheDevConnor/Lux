#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    if (alignment == 0)
        alignment = 1024;

    alignment = min_size(s, alignment);
    size_t buffer_align = max_size(alignment, alignof(Buffer));

    // Round up total_size to be a multiple of buffer_align
    size_t total_size = sizeof(Buffer) + s + buffer_align;
    size_t aligned_total_size = (total_size + buffer_align - 1) & ~(buffer_align - 1);

    void *raw = aligned_alloc(buffer_align, aligned_total_size);
    if (!raw) {
        return NULL;
    }

    Buffer *buf = (Buffer*)raw;
    buf->size = s;
    buf->next = NULL;

    // Align the pointer to buffer data
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

bool growable_array_init(GrowableArray *arr, ArenaAllocator *arena, size_t initial_capacity, size_t item_size) {
    arr->data = arena_alloc(arena, initial_capacity * item_size, item_size);
    if (!arr->data) return false;

    arr->arena = arena;
    arr->count = 0;
    arr->capacity = initial_capacity;
    arr->item_size = item_size;
    return true;
}

void *growable_array_push(GrowableArray *arr) {
    if (arr->count >= arr->capacity) {
        size_t new_capacity = arr->capacity * 2;
        void *new_block = arena_alloc(arr->arena, new_capacity * arr->item_size, arr->item_size);
        if (!new_block) return NULL;

        memcpy(new_block, arr->data, arr->count * arr->item_size);
        arr->data = new_block;
        arr->capacity = new_capacity;
    }

    void *slot = (char*)arr->data + (arr->count * arr->item_size);
    arr->count += 1;
    return slot;
}