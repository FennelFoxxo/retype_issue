#include "globals.h"

extern "C" {
#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>
#include <utils/util.h>
}

namespace Globals {

bool is_initialized = false;

seL4_Word num_empty_slots;
seL4_CPtr bootstrap_memory;
seL4_CPtr memory_allocator_tcb;
seL4_CPtr memory_mapper_tcb;

UsableMemoryRegion usable_memory_regions[GLOBALS_MAX_USABLE_MEMORY_REGIONS];
seL4_Word usable_memory_regions_count = 0;
seL4_Word large_chunks_count = 0;

char memory_allocator_stack[1024] __attribute__((aligned(8)));
char memory_mapper_stack[1024] __attribute__((aligned(8)));


bool initialize() {
	seL4_Error error;
	
	if (is_initialized) return true;
	
	seL4_BootInfo *info = platsupport_get_bootinfo();
	
	// Setup CPtrs
	num_empty_slots = info->empty.end - info->empty.start;
	if (num_empty_slots < GLOBALS_MIN_EMPTY_SLOTS) return false; // Not enough empty cslots to satisfy requirements
	
	bootstrap_memory		= info->empty.start + 0;
	memory_allocator_tcb	= info->empty.start + 1;
	memory_mapper_tcb		= info->empty.start + 2;
	
	// Add memory regions to array
	for (seL4_CPtr slot = info->untyped.start; slot != info->untyped.end; slot++) {
        seL4_UntypedDesc *desc = &info->untypedList[slot - info->untyped.start];
		// Only add non-device memory that is at least large chunk in size
		if (!desc->isDevice && desc->sizeBits >= GLOBALS_LARGE_CHUNK_BITS) {
			if (usable_memory_regions_count >= GLOBALS_MAX_USABLE_MEMORY_REGIONS) break; // No more room in array
			usable_memory_regions[usable_memory_regions_count++] = UsableMemoryRegion{desc->paddr, BIT(desc->sizeBits), slot};
			large_chunks_count += BIT(desc->sizeBits - GLOBALS_LARGE_CHUNK_BITS);
		}
    }
	
	if (large_chunks_count < GLOBALS_MIN_LARGE_CHUNKS) { // Exit if not enough chunks
		return false;
	}
	
	// Now, we need to allocate a chunk for bootstrap memory. Since we already have a nice array of memory regions, just remove a chunk from there
	// It's fine if the region ends up with a size of 0 - it will just be ignored
	error = seL4_Untyped_Retype(usable_memory_regions[0].cptr, seL4_UntypedObject, GLOBALS_LARGE_CHUNK_BITS, seL4_CapInitThreadCNode, 0, 0, bootstrap_memory, 1);
	if (error != seL4_NoError) return false;
	
	usable_memory_regions[0].paddr += GLOBALS_LARGE_CHUNK_SIZE;
	usable_memory_regions[0].size -= GLOBALS_LARGE_CHUNK_SIZE;
	large_chunks_count--;
	
	error = seL4_Untyped_Retype(bootstrap_memory, seL4_TCBObject, seL4_TCBBits, seL4_CapInitThreadCNode, 0, 0, memory_allocator_tcb, 1);
    if (error != seL4_NoError) return false;
	
	error = seL4_Untyped_Retype(bootstrap_memory, seL4_TCBObject, seL4_TCBBits, seL4_CapInitThreadCNode, 0, 0, memory_mapper_tcb, 1);
    if (error != seL4_NoError) return false;
	
	is_initialized = true;
	return true;
}

}