#ifndef USTL_MEM_COPY_H
#define USTL_MEM_COPY_H 1

#include <ustl/config.hpp>
#include <ustl/config/concept_require.hpp>
#include <ustl/mem/address_of.hpp>

namespace ustl::mem {
namespace details {

    // Copy on bytes.
    USTL_API 
    auto copy(void const *src, void *dst, usize n) USTL_NOEXCEPT -> void;

} // namespace ustl::mem::details

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto copy(T const &src, T &dst) USTL_NOEXCEPT -> void
    {
        // USTL_REQUIRES(traits::IsTriviallyDefaultConstructibleV<T>)
        return details::copy(address_of(src), address_of(dst), sizeof(T));
    }

    template <typename T, usize N>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto copy(T const (&src)[N], T *dst) USTL_NOEXCEPT -> void
    {
        // USTL_REQUIRES(traits::IsTriviallyDefaultConstructibleV<T>)
        return details::copy(address_of(src), address_of(dst), sizeof(T) * N);
    }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto copy(T const *src, T *dst, usize n) USTL_NOEXCEPT -> void
    {
        // USTL_REQUIRES(traits::IsTriviallyDefaultConstructibleV<T>)
        return details::copy(src, dst, n);
    }

} // namespace ustl::mem

#endif // #ifndef USTL_MEM_COPY_H