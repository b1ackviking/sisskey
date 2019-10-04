#include <cassert>

#ifndef NDEBUG
#define VMA_ASSERT(expr) assert(expr)
#endif // !NDEBUG

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>