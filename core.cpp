#include "core.h"

void* copy_val(const char* prefix, void* source, size_t cpy_size)
{
	if (prefix)
	{
		size_t prefix_size = strlen(prefix);
		void* cpy = malloc(cpy_size + prefix_size);
		memcpy(cpy, prefix, prefix_size);
		memcpy((void*)((uintptr_t)cpy+prefix_size), source, cpy_size);
		return cpy;
	}

	void* cpy = malloc(cpy_size);
	memcpy(cpy, source, cpy_size);
	return cpy;
}