#include <stdlib.h>

void* aligned_alloc(size_t size, size_t align) {
	size_t addr;
	void* p = malloc(size + align + sizeof(void*));
	if (!p) return p;
	addr = (size_t) p;
  addr = (addr + sizeof(void*) + align-1) & -align;
	((void**) addr)[-1] = p;
	return (void*) addr;
}

void aligned_free(void* p) {
  free(((void**) p)[-1]);
}
