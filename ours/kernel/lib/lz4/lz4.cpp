#include <lz4/lz4.hpp>
#include <ustl/algorithms/copy.hpp>
#include <ustl/algorithms/generation.hpp>

#define LZ4_memcpy(dest, src, size)     \
    ustl::algorithms::copy_n((char const *)src, size, (char *)dest) 
#define LZ4_memmove(dest, src, size)    \
    ustl::algorithms::move((char const *)src, ((char const *)src) + size, (char *)dest) 
#define LZ4_memset(dest, value, size)    \
    ustl::algorithms::fill_n((char *)dest, size, value) 

// At third_party/lz4-1.10.0
#include <lib/lz4.c>

namespace lz4 {
    auto compress(void const *src, void *dest, usize src_size, usize dest_size) -> isize
    { return LZ4_compress_default(static_cast<char const *>(src), static_cast<char *>(dest), src_size, dest_size); }

    auto decompress(void const *src, void *dest, usize src_size, usize dest_size) -> isize
    { return LZ4_decompress_safe(static_cast<char const *>(src), static_cast<char *>(dest), src_size, dest_size); }

} // namespace lz4