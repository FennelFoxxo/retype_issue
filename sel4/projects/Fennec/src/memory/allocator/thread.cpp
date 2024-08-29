#include "thread.h"

extern "C" {
#include <stdio.h>
}

namespace Memory::Allocator {
	void thread() {
		printf("Memory allocator thread started\n");
		while (1) {}
	}
}