/**
 * @file memory.h
 * @brief Arena allocator and growable array utilities for fast memory management.
 *
 * This module implements a simple arena (region) allocator and a growable array 
 * type that uses the arena for storage. The arena allocator reduces allocation 
 * overhead by allocating large memory blocks ("buffers") at once and serving 
 * allocations sequentially from them.
 *
 * ## Overview
 * - **ArenaAllocator**: Allocates memory from contiguous blocks (buffers).
 * - **GrowableArray**: A dynamic array that stores items in arena-allocated memory.
 *
 * ## Features
 * - Minimal malloc/free calls.
 * - Custom growth strategy for buffers.
 * - Reset all allocations at once (fast memory recycling).
 * - Optional debug tracking of active buffers.
 *
 * ## Flow Diagram
 * @dot
 * digraph ArenaFlow {
 *   rankdir=LR;
 *   node [shape=box, style=filled, fillcolor=lightblue];
 *   
 *   ArenaAllocator -> Buffer [label="allocates"];
 *   Buffer -> GrowableArray [label="serves memory"];
 *   GrowableArray -> Items [label="stores items"];
 *   ArenaAllocator -> Buffer [label="reset", style=dashed, color=gray];
 *   Buffer -> Memory [label="free", style=dotted];
 *   Memory -> ArenaAllocator [label="recycled", style=dotted];
 *   
 *   Items [shape=ellipse, fillcolor=lightgreen];
 *   Memory [shape=oval, fillcolor=orange];
 * }
 * @enddot
 */

#pragma once

#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Minimum size of a newly allocated arena buffer in bytes. */
#define ARENA_MIN_BUFFER_SIZE (64 * 1024)    // 64KB minimum
/** @brief Factor by which buffer size grows when more space is needed. */
#define ARENA_GROWTH_FACTOR 2
/** @brief Maximum size of an arena buffer in bytes. */
#define ARENA_MAX_BUFFER_SIZE (16 * 1024 * 1024) // 16MB maximum per buffer

/** @brief Enable to print debug allocation logs. */
// #define DEBUG_ARENA_ALLOC 1

#ifdef DEBUG_ARENA_ALLOC
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

#ifdef DEBUG_ARENA_ALLOC
static int active_buffers = 0;
/** @brief Debug helper to track allocations. */
#define TRACK_ALLOC(ptr) do { if (ptr) active_buffers++; DEBUG_PRINT("ALLOC %p (count=%d)\n", ptr, active_buffers); } while(0)
/** @brief Debug helper to track frees. */
#define TRACK_FREE(ptr)  do { if (ptr) active_buffers--; DEBUG_PRINT("FREE  %p (count=%d)\n", ptr, active_buffers); } while(0)
#else
#define TRACK_ALLOC(ptr) ((void)0)
#define TRACK_FREE(ptr) ((void)0)
#endif

/**
 * @brief Represents a memory buffer in the arena.
 */
typedef struct Buffer {
    size_t size;           /**< Size of the buffer in bytes. */
    struct Buffer *next;   /**< Pointer to the next buffer in the chain. */
    char *ptr;             /**< Raw memory pointer. */
} Buffer;

/**
 * @brief Arena allocator structure.
 */
typedef struct ArenaAllocator {
    size_t offset;            /**< Current offset in the active buffer. */
    Buffer *buffer;           /**< Pointer to the current active buffer. */
    Buffer *head;             /**< Pointer to the first buffer in the chain. */
    size_t next_buffer_size;  /**< Size to use for the next buffer allocation. */
    size_t total_allocated;   /**< Total bytes allocated across all buffers. */
} ArenaAllocator;

/**
 * @brief Growable array backed by an arena allocator.
 */
typedef struct {
    void *data;               /**< Pointer to array data. */
    size_t count;             /**< Number of elements currently stored. */
    size_t capacity;          /**< Total allocated elements capacity. */
    size_t item_size;         /**< Size of each element in bytes. */
    ArenaAllocator *arena;    /**< Arena allocator used for storage. */
} GrowableArray;

/* =========================================================================
   Buffer Management
   ========================================================================= */

/**
 * @brief Creates a new buffer with given size and alignment.
 * @param s Buffer size in bytes.
 * @param alignment Memory alignment in bytes.
 * @return Pointer to the newly created buffer or NULL on failure.
 */
Buffer* buffer_create(size_t s, size_t alignment);

/* =========================================================================
   Arena Allocator Functions
   ========================================================================= */

/**
 * @brief Initializes an arena allocator.
 * @param arena Pointer to the arena to initialize.
 * @param initial_size Initial size of the first buffer.
 * @return 0 on success, non-zero on error.
 */
int arena_allocator_init(ArenaAllocator *arena, size_t initial_size);

/**
 * @brief Allocates memory from the arena.
 * @param arena Pointer to the arena.
 * @param size Number of bytes to allocate.
 * @param alignment Memory alignment in bytes.
 * @return Pointer to allocated memory, or NULL on failure.
 */
void* arena_alloc(ArenaAllocator *arena, size_t size, size_t alignment);

/**
 * @brief Resets the arena, keeping buffers but making them empty.
 * @param arena Pointer to the arena to reset.
 */
void arena_reset(ArenaAllocator *arena);

/**
 * @brief Frees all memory used by the arena.
 * @param arena Pointer to the arena to destroy.
 */
void arena_destroy(ArenaAllocator *arena);

/**
 * @brief Duplicates a string into arena-managed memory.
 * @param arena Pointer to the arena.
 * @param str String to duplicate.
 * @return Pointer to the duplicated string in arena memory.
 */
char *arena_strdup(ArenaAllocator *arena, const char *str);

/**
 * @brief Prints statistics about arena memory usage.
 * @param arena Pointer to the arena.
 */
void arena_print_stats(ArenaAllocator *arena);

/**
 * @brief Gets the total number of bytes allocated across all buffers.
 * @param arena Pointer to the arena.
 * @return Total allocated bytes.
 */
size_t arena_get_total_allocated(ArenaAllocator *arena);

/* =========================================================================
   Growable Array Functions
   ========================================================================= */

/**
 * @brief Initializes a growable array using an arena allocator.
 * @param arr Pointer to the GrowableArray to initialize.
 * @param arena Pointer to the arena allocator for storage.
 * @param initial_capacity Initial element capacity.
 * @param item_size Size of each array element in bytes.
 * @return true on success, false on failure.
 */
bool growable_array_init(GrowableArray *arr, ArenaAllocator *arena, size_t initial_capacity, size_t item_size);

/**
 * @brief Pushes a new element onto the growable array.
 * @param arr Pointer to the GrowableArray.
 * @return Pointer to the new element's memory slot.
 */
void* growable_array_push(GrowableArray *arr);

#ifdef __cplusplus
}
#endif
