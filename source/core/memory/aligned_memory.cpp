#include "aligned_memory.h"
#include "cpu_memory.h"

#ifdef __linux__
#include <cstdlib>
#elif _WIN32
#include <malloc.h>
#endif

namespace core
{

void* alignedMalloc(std::size_t size, std::size_t alignment) throw(std::bad_alloc)
{
    void* ptr{nullptr};
#ifdef __linux__
    posix_memalign(&ptr, alignment, size);
#elif _WIN32
    ptr = _aligned_malloc(size, alignment);
#endif
    //For just C++11 implementation with std::align
    //See http://en.cppreference.com/w/cpp/core/memory/align

    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }

    return ptr;
}

void alignedFree(void* ptr) noexcept
{
#ifdef __linux__
    free(ptr);
#elif _WIN32
    _aligned_free(ptr);
#endif
}

} // namespace