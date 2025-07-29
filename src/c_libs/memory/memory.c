#include <stdio.h>
#include <string.h>
#include "memory.h"

#if defined(_WIN32)
#include <malloc.h>
#define ALIGNED_ALLOC(sz, align) _aligned_malloc((sz), (align))
#define ALIGNED_FREE(ptr) _aligned_free(ptr)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#include <stdlib.h>
#define ALIGNED_ALLOC(sz, align) aligned_alloc((align), (sz))
#define ALIGNED_FREE(ptr) free(ptr)
#else
#error "aligned_alloc or platform equivalent not available"
#endif

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#endif

/* Helper function to get maximum of two values */
static inline size_t max_size(size_t a, size_t b) { return (a > b) ? a : b; }

/* Helper function to get minimum of two values */
static inline size_t min_size(size_t a, size_t b) { return (a < b) ? a : b; }

/* Calculate next buffer size with growth factor */
static size_t calculate_next_buffer_size(size_t current_size, size_t requested_size) {
    size_t next_size = max_size(current_size * ARENA_GROWTH_FACTOR, requested_size);
    next_size = max_size(next_size, ARENA_MIN_BUFFER_SIZE);
    next_size = min_size(next_size, ARENA_MAX_BUFFER_SIZE);
    return next_size;
}

/* Create a new buffer with specified size and alignment */
Buffer *buffer_create(size_t s, size_t alignment) {
    if (alignment == 0)
        alignment = 1024;

    alignment = max_size(alignment, alignof(Buffer));
    
    size_t total_size = sizeof(Buffer) + s + alignment;
    size_t aligned_total_size = (total_size + alignment - 1) & ~(alignment - 1);

    #if defined(_WIN32)
        void *raw = _aligned_malloc(aligned_total_size, alignment);
    #else
        void *raw = aligned_alloc(alignment, aligned_total_size);
    #endif

    TRACK_ALLOC(raw);
    if (!raw) {
        DEBUG_PRINT("buffer_create: FAILED to allocate %zu bytes (align %zu)\n",
                    aligned_total_size, alignment);
        return NULL;
    }

    Buffer *buf = (Buffer *)raw;
    buf->size = s;
    buf->next = NULL;

    char *raw_bytes = (char *)raw;
    size_t struct_end = (size_t)(raw_bytes + sizeof(Buffer));
    size_t aligned_data = (struct_end + alignment - 1) & ~(alignment - 1);
    buf->ptr = (char *)aligned_data;

    DEBUG_PRINT("buffer_create: allocated buffer %p (%zu bytes aligned to %zu), "
                "usable ptr %p\n",
                (void *)buf, aligned_total_size, alignment, (void *)buf->ptr);

    return buf;
}

/* Initialize arena allocator with dynamic growth */
int arena_allocator_init(ArenaAllocator *arena, size_t initial_size) {
    if (!arena)
        return -1;

    // Ensure minimum size
    initial_size = max_size(initial_size, ARENA_MIN_BUFFER_SIZE);

    arena->buffer = buffer_create(initial_size, 1024);
    if (!arena->buffer)
        return -1;

    arena->head = arena->buffer;
    arena->offset = 0;
    arena->next_buffer_size = calculate_next_buffer_size(initial_size, 0);
    arena->total_allocated = initial_size;

    DEBUG_PRINT("arena_allocator_init: initialized with %zu bytes, next buffer will be %zu bytes\n",
                initial_size, arena->next_buffer_size);

    return 0;
}

/* Allocate a new buffer and add it to the chain */
static Buffer* arena_add_buffer(ArenaAllocator *arena, size_t min_size) {
    size_t buffer_size = calculate_next_buffer_size(arena->next_buffer_size, min_size);
    
    Buffer *new_buffer = buffer_create(buffer_size, 1024);
    if (!new_buffer) {
        DEBUG_PRINT("arena_add_buffer: FAILED to create new buffer of size %zu\n", buffer_size);
        return NULL;
    }

    // Add to the end of the chain
    Buffer *current = arena->head;
    while (current->next) {
        current = current->next;
    }
    current->next = new_buffer;

    arena->total_allocated += buffer_size;
    arena->next_buffer_size = calculate_next_buffer_size(buffer_size, 0);

    DEBUG_PRINT("arena_add_buffer: added new buffer %p (size %zu), total allocated: %zu bytes\n",
                (void *)new_buffer, buffer_size, arena->total_allocated);

    return new_buffer;
}

/* Allocate memory from arena with automatic growth */
void *arena_alloc(ArenaAllocator *arena, size_t size, size_t alignment) {
    if (!arena || !arena->buffer)
        return NULL;

    if (alignment == 0)
        alignment = alignof(max_align_t);

    // For very large allocations, create a dedicated buffer
    if (size > ARENA_MAX_BUFFER_SIZE / 4) {
        Buffer *dedicated_buffer = arena_add_buffer(arena, size);
        if (!dedicated_buffer)
            return NULL;

        DEBUG_PRINT("arena_alloc (dedicated): %zu bytes (align %zu) at %p in buffer %p\n",
                    size, alignment, dedicated_buffer->ptr, (void *)dedicated_buffer);
        return dedicated_buffer->ptr;
    }

    // Try to allocate from current buffer
    while (1) {
        size_t aligned_offset = (arena->offset + (alignment - 1)) & ~(alignment - 1);

        if (aligned_offset + size <= arena->buffer->size) {
            // Allocation fits in current buffer
            void *ptr = arena->buffer->ptr + aligned_offset;
            arena->offset = aligned_offset + size;
            
            DEBUG_PRINT("arena_alloc: %zu bytes (align %zu) at %p in buffer %p (offset now %zu/%zu)\n",
                        size, alignment, ptr, (void *)arena->buffer, arena->offset, arena->buffer->size);
            return ptr;
        }

        // Current buffer is full, try next buffer
        if (arena->buffer->next) {
            arena->buffer = arena->buffer->next;
            arena->offset = 0;
            DEBUG_PRINT("arena_alloc: switched to existing buffer %p (size %zu)\n",
                        (void *)arena->buffer, arena->buffer->size);
            continue;
        }

        // No next buffer exists, create a new one
        Buffer *new_buffer = arena_add_buffer(arena, size);
        if (!new_buffer) {
            DEBUG_PRINT("arena_alloc: FAILED to add new buffer for %zu bytes\n", size);
            return NULL;
        }

        arena->buffer = new_buffer;
        arena->offset = 0;
        DEBUG_PRINT("arena_alloc: switched to new buffer %p (size %zu)\n",
                    (void *)arena->buffer, arena->buffer->size);
        // Continue the loop to allocate from the new buffer
    }
}

/* Reset arena to initial state (reuse allocated buffers) */
void arena_reset(ArenaAllocator *arena) {
    if (!arena)
        return;

    arena->offset = 0;
    arena->buffer = arena->head;
    DEBUG_PRINT("arena_reset: reset to first buffer %p\n", (void *)arena->head);
}

/* Free all memory and cleanup arena */
void arena_destroy(ArenaAllocator *arena) {
    if (!arena)
        return;

    Buffer *current = arena->head;
    while (current) {
        DEBUG_PRINT("arena_destroy: freeing buffer %p (size %zu, ptr %p)\n",
                    (void *)current, current->size, (void *)current->ptr);
        Buffer *next = current->next;
        #if defined(_WIN32)
            _aligned_free(current);
        #else
            free(current);
        #endif
        TRACK_FREE(current);
        current = next;
    }

    arena->head = NULL;
    arena->buffer = NULL;
    arena->offset = 0;
    arena->next_buffer_size = 0;
    arena->total_allocated = 0;
}

/* Get total allocated memory across all buffers */
size_t arena_get_total_allocated(ArenaAllocator *arena) {
    if (!arena)
        return 0;
    return arena->total_allocated;
}

/* Print arena statistics */
void arena_print_stats(ArenaAllocator *arena) {
    if (!arena) {
        fprintf(stderr, "Arena is NULL\n");
        return;
    }

    size_t buffer_count = 0;
    size_t total_size = 0;
    Buffer *current = arena->head;
    
    while (current) {
        buffer_count++;
        total_size += current->size;
        current = current->next;
    }

    fprintf(stderr, "Arena Statistics:\n");
    fprintf(stderr, "  Total buffers: %zu\n", buffer_count);
    fprintf(stderr, "  Total allocated: %zu bytes (%.2f MB)\n", 
            total_size, (double)total_size / (1024 * 1024));
    fprintf(stderr, "  Current buffer: %p (offset: %zu/%zu)\n",
            (void *)arena->buffer, arena->offset, 
            arena->buffer ? arena->buffer->size : 0);
    fprintf(stderr, "  Next buffer size: %zu bytes\n", arena->next_buffer_size);
}

/* Duplicate string in arena */
char *arena_strdup(ArenaAllocator *arena, const char *src) {
    size_t len = strlen(src) + 1;
    char *dst = arena_alloc(arena, len, alignof(char));
    if (dst) memcpy(dst, src, len);
    return dst;
}

/* Initialize growable array with proper alignment */
bool growable_array_init(GrowableArray *arr, ArenaAllocator *arena,
                         size_t initial_capacity, size_t item_size) {
    if (initial_capacity == 0)
        initial_capacity = 4;  // Minimum capacity

    size_t alignment = (item_size == sizeof(void*)) ? alignof(void*) : 
                      (item_size >= alignof(max_align_t)) ? alignof(max_align_t) : item_size;

    arr->data = arena_alloc(arena, initial_capacity * item_size, alignment);
    if (!arr->data) {
        DEBUG_PRINT("growable_array_init: FAILED to allocate initial array "
                    "(cap=%zu, size=%zu, align=%zu)\n",
                    initial_capacity, item_size, alignment);
        return false;
    }

    arr->arena = arena;
    arr->count = 0;
    arr->capacity = initial_capacity;
    arr->item_size = item_size;

    DEBUG_PRINT("growable_array_init: created array at %p (cap=%zu, "
                "item_size=%zu, alignment=%zu, total=%zu bytes)\n",
                arr->data, arr->capacity, arr->item_size, alignment,
                arr->capacity * arr->item_size);

    return true;
}

/* Push new item to growable array with automatic growth */
void *growable_array_push(GrowableArray *arr) {
    if (arr->count >= arr->capacity) {
        size_t new_capacity = arr->capacity * 2;
        size_t alignment = (arr->item_size == sizeof(void*)) ? alignof(void*) : 
                          (arr->item_size >= alignof(max_align_t)) ? alignof(max_align_t) : arr->item_size;
        
        void *new_block = arena_alloc(arr->arena, new_capacity * arr->item_size, alignment);
        if (!new_block) {
            DEBUG_PRINT("growable_array_push: FAILED to grow array (new cap=%zu, total size=%zu)\n",
                        new_capacity, new_capacity * arr->item_size);
            return NULL;
        }

        DEBUG_PRINT("growable_array_push: growing array from %p to %p "
                    "(old cap=%zu → new cap=%zu, item_size=%zu)\n",
                    arr->data, new_block, arr->capacity, new_capacity, arr->item_size);

        // Copy existing data
        memcpy(new_block, arr->data, arr->count * arr->item_size);
        
        // Update array metadata
        arr->data = new_block;
        arr->capacity = new_capacity;
    }

    void *slot = (char *)arr->data + (arr->count * arr->item_size);
    arr->count += 1;
    
    DEBUG_PRINT("growable_array_push: pushed item at index %zu (address %p)\n",
                arr->count - 1, slot);
    
    return slot;
}