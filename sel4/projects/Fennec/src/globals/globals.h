#pragma once

#include <sel4/simple_types.h>

#define GLOBALS_MAX_USABLE_MEMORY_REGIONS 100
#define GLOBALS_LARGE_CHUNK_BITS 20
#define GLOBALS_SMALL_CHUNK_BITS 12
#define GLOBALS_MIN_LARGE_CHUNKS 10

#define GLOBALS_MIN_EMPTY_SLOTS 10

#define GLOBALS_LARGE_CHUNK_SIZE BIT(GLOBALS_LARGE_CHUNK_BITS)
#define GLOBALS_SMALL_CHUNK_SIZE BIT(GLOBALS_SMALL_CHUNK_BITS)

#define GLOBALS_MEMORY_ALLOCATOR_PRIORITY 250
#define GLOBALS_MEMORY_MAPPER_PRIORITY 250

namespace Globals {

// Initializes global variables
bool initialize();
extern bool is_initialized;

// CSlots
extern seL4_Word num_empty_slots;
extern seL4_CPtr bootstrap_memory; // LARGE_CHUNK_SIZE-sized block of memory for initial bootstrapping tasks
extern seL4_CPtr memory_allocator_tcb;
extern seL4_CPtr memory_mapper_tcb;
	
struct UsableMemoryRegion {
	seL4_Word paddr;
	seL4_Word size;
	seL4_CPtr cptr;
};

// List of usable memory regions, excluding the chunk used for bootstrap memory
extern UsableMemoryRegion usable_memory_regions[GLOBALS_MAX_USABLE_MEMORY_REGIONS];
extern seL4_Word usable_memory_regions_count;
extern seL4_Word large_chunks_count;

extern char memory_allocator_stack[1024] __attribute__((aligned(8)));
extern char memory_mapper_stack[1024] __attribute__((aligned(8)));

}