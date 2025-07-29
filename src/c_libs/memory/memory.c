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

/* Create a new buffer with specified size and alignment */
Buffer *buffer_create(size_t s, size_t alignment) {
  if (alignment == 0)
    alignment = 1024;

  alignment = min_size(s, alignment);
  size_t buffer_align = max_size(alignment, alignof(Buffer));

  size_t total_size = sizeof(Buffer) + s + buffer_align;
  size_t aligned_total_size =
      (total_size + buffer_align - 1) & ~(buffer_align - 1);

  #if defined(_WIN32)
    void *raw = _aligned_malloc(aligned_total_size, buffer_align);
  #else
    void *raw = aligned_alloc(buffer_align, aligned_total_size);
  #endif

  TRACK_ALLOC(raw);
  if (!raw) {
    DEBUG_PRINT("buffer_create: FAILED to allocate %zu bytes (align %zu)\n",
                aligned_total_size, buffer_align);
    return NULL;
  }

  memset(raw, 0, aligned_total_size);

  Buffer *buf = (Buffer *)raw;
  buf->size = s;
  buf->next = NULL;

  char *raw_bytes = (char *)raw;
  size_t struct_end = (size_t)(raw_bytes + sizeof(Buffer));
  size_t aligned_data = (struct_end + alignment - 1) & ~(alignment - 1);
  buf->ptr = (char *)aligned_data;

  DEBUG_PRINT("buffer_create: allocated buffer %p (%zu bytes aligned to %zu), "
              "usable ptr %p\n",
              (void *)buf, aligned_total_size, buffer_align, (void *)buf->ptr);

  return buf;
}

/* Initialize arena allocator */
int arena_allocator_init(ArenaAllocator *arena, size_t size) {
  if (!arena)
    return -1;

  arena->buffer = buffer_create(size, 1024);
  if (!arena->buffer)
    return -1;

  arena->head = arena->buffer;
  arena->offset = 0;
  return 0;
}

/* Allocate memory from arena */
void *arena_alloc(ArenaAllocator *arena, size_t size, size_t alignment) {
  if (!arena || !arena->buffer)
    return NULL;

  if (alignment == 0)
    alignment = alignof(max_align_t);

  if ((size > arena->buffer->size / 4) ||
      (alignment > arena->buffer->size / 4)) {
    Buffer *new_buffer = buffer_create(size, alignment);
    if (!new_buffer)
      return NULL;

    new_buffer->next = arena->buffer->next;
    arena->buffer->next = new_buffer;
    DEBUG_PRINT("arena_alloc (large): %zu bytes (align %zu) at %p (dedicated "
                "buffer %p)\n",
                size, alignment, new_buffer->ptr, (void *)new_buffer);
    return new_buffer->ptr;
  }

  while (1) {
    size_t aligned_offset =
        (arena->offset + (alignment - 1)) & ~(alignment - 1);

    if (aligned_offset + size > arena->buffer->size) {
      if (arena->buffer->next) {
        arena->buffer = arena->buffer->next;
        arena->offset = 0;
        continue;
      }
      arena->buffer->next = buffer_create(arena->buffer->size, 1024);
      if (!arena->buffer->next)
        return NULL;

      arena->buffer = arena->buffer->next;
      arena->offset = 0;
      continue;
    }

    void *ptr = arena->buffer->ptr + aligned_offset;
    DEBUG_PRINT("arena_alloc: %zu bytes (align %zu) at %p in buffer %p\n", size,
                alignment, ptr, (void *)arena->buffer);
    arena->offset = aligned_offset + size;
    return ptr;
  }
}

/* Reset arena to initial state (reuse allocated buffers) */
void arena_reset(ArenaAllocator *arena) {
  if (!arena)
    return;

  arena->offset = 0;
  arena->buffer = arena->head;
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
}

char *arena_strdup(ArenaAllocator *arena, const char *src) {
    size_t len = strlen(src) + 1;
    char *dst = arena_alloc(arena, len, alignof(char));
    if (dst) memcpy(dst, src, len);
    return dst;
}


bool growable_array_init(GrowableArray *arr, ArenaAllocator *arena,
                         size_t initial_capacity, size_t item_size) {
  arr->data = arena_alloc(arena, initial_capacity * item_size, item_size);
  if (!arr->data) {
    DEBUG_PRINT("growable_array_init: FAILED to allocate initial array "
                "(cap=%zu, size=%zu)\n",
                initial_capacity, item_size);
    return false;
  }

  arr->arena = arena;
  arr->count = 0;
  arr->capacity = initial_capacity;
  arr->item_size = item_size;

  DEBUG_PRINT("growable_array_init: created array at %p (cap=%zu, "
              "item_size=%zu, total=%zu)\n",
              arr->data, arr->capacity, arr->item_size,
              arr->capacity * arr->item_size);

  return true;
}

void *growable_array_push(GrowableArray *arr) {
  if (arr->count >= arr->capacity) {
    size_t new_capacity = arr->capacity * 2;
    void *new_block =
        arena_alloc(arr->arena, new_capacity * arr->item_size, arr->item_size);
    if (!new_block) {
      DEBUG_PRINT("growable_array_push: FAILED to grow array (new cap=%zu)\n",
                  new_capacity);
      return NULL;
    }

    DEBUG_PRINT("growable_array_push: reallocating array from %p to %p (old "
                "cap=%zu → new cap=%zu)\n",
                arr->data, new_block, arr->capacity, new_capacity);

    memcpy(new_block, arr->data, arr->count * arr->item_size);
    arr->data = new_block;
    arr->capacity = new_capacity;
  }

  void *slot = (char *)arr->data + (arr->count * arr->item_size);
  arr->count += 1;
  DEBUG_PRINT("growable_array_push: pushed new item at index %zu (address %p)\n",
            arr->count - 1, (char*)arr->data + (arr->count - 1) * arr->item_size);
  return slot;
}