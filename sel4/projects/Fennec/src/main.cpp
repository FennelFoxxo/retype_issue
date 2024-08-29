#include "globals/globals.h"
#include "memory/allocator/setup.h"
#include "memory/mapper/setup.h"

extern "C" {
#include <stdio.h>
#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>
#include <utils/util.h>
}


void halt(const char* halt_message = nullptr) {
	if (halt_message != nullptr) printf("%s\n", halt_message);
	seL4_TCB_Suspend(seL4_CapInitThreadTCB);
}


int main(void)
{
	printf("\n\n--- ROOTSERVER START ---\n\n");
	
	if (Globals::initialize()) printf("Globals initialized successfully\n");
	else halt("Failed to initialize globals");
	
	if (Memory::Allocator::initialize()) printf("Memory allocator initialized successfully\n");
	else halt("Failed to initialize memory allocator");
	
	if (Memory::Mapper::initialize()) printf("Memory mapper initialized successfully\n");
	else halt("Failed to initialize memory mapper");
	
	printf("Total large memory regions: %zu\n", Globals::large_chunks_count);
	printf("Total empty slots: %zu\n", Globals::num_empty_slots);

	
	seL4_DebugDumpScheduler();

	halt();
    return 0;
	
}