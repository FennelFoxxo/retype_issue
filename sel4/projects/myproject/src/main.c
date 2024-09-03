
#include <stdio.h>
#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>
#include <utils/util.h>


int main(void)
{
	printf("\n\n--- ROOTSERVER START ---\n\n");
	
	seL4_BootInfo* info = platsupport_get_bootinfo();
	
	seL4_CPtr empty_ptr = info->empty.start;
	seL4_CPtr empty_end = info->empty.end;
	
	seL4_Error error;
	
	for (seL4_CPtr slot = info->untyped.start; slot != info->untyped.end; slot++) { // Iterate over list of untypeds provided by bootinfo
		seL4_UntypedDesc *desc = &info->untypedList[slot - info->untyped.start];

		if (!desc->isDevice && desc->sizeBits >= 20) { // Non-device memory at least 1MB in size
			printf("Attempting to retype slot %lu - sizeBits = %d - paddr = 0x%lx\n", slot, desc->sizeBits, desc->paddr);
			
			error = seL4_Untyped_Retype(slot, seL4_UntypedObject, 20, seL4_CapInitThreadCNode, 0, 0, empty_ptr++, 1); // Retype and place in next empty slot
			if (error != seL4_NoError) {
				printf("Error while retyping: %d\n", error);
				while (1);
			}
			
			printf("  Retyped untyped in slot %lu ok\n", slot);
			
			if (empty_ptr == empty_end) {
				printf("Out of empty slots\n");
				while (1);
			}
		}
	}
	
	printf("All done, reached end of program\n");

    while (1);
	
	return 0;
	
}