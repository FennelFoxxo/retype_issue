
#include <stdio.h>
#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>
#include <utils/util.h>

int main(void)
{
	printf("\n\n--- ROOTSERVER START ---\n\n");

	seL4_TCB_Suspend(seL4_CapInitThreadTCB);
    return 0;
	
}