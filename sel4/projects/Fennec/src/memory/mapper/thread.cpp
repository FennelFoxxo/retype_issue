#include "thread.h"

extern "C" {
#include <stdio.h>
}

namespace Memory::Mapper {
	void thread() {
		printf("Memory mapper thread started\n");
		while (1) {}
	}
}