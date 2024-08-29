#include "setup.h"
#include "thread.h"

extern "C" {
#include <sel4/sel4.h>
}

namespace Memory::Allocator {
	
	bool initialize() {
		seL4_Error error;
		
		if (!Globals::is_initialized) return false;
		
		error = seL4_TCB_Configure(Globals::memory_allocator_tcb, seL4_CapNull, seL4_CapInitThreadCNode, 0, seL4_CapInitThreadVSpace, 0, 0, seL4_CapNull);
		if (error != seL4_NoError) return false;
	
		error = seL4_TCB_SetPriority(Globals::memory_allocator_tcb, seL4_CapInitThreadTCB, GLOBALS_MEMORY_ALLOCATOR_PRIORITY);
		if (error != seL4_NoError) return false;


		seL4_UserContext regs;
		error = seL4_TCB_ReadRegisters(Globals::memory_allocator_tcb, 0, 0, sizeof(regs)/sizeof(seL4_Word), &regs);
		if (error != seL4_NoError) return false;

		regs.rip = (seL4_Word)&thread;
		regs.rsp = (seL4_Word)&Globals::memory_allocator_stack;

		error = seL4_TCB_WriteRegisters(Globals::memory_allocator_tcb, 0, 0, sizeof(regs)/sizeof(seL4_Word), &regs);
		if (error != seL4_NoError) return false;
	
		error = seL4_TCB_Resume(Globals::memory_allocator_tcb);
		if (error != seL4_NoError) return false;
		
		
		return true;
	}
	
}