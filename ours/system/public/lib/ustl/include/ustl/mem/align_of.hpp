#ifndef USTL_MEM_ALIGN_OF_H
#define USTL_MEM_ALIGN_OF_H 1

#include <ustl/config.hpp>

namespace ustl::mem {
namespace details {
    template <typename T>
    struct AlignOf
    {
        static auto const VALUE = USTL_ALIGNOF(T);
    };

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto align_of() USTL_NOEXCEPT -> usize
    {  return AlignOf<T>::VALUE;  }

} // namespace ustl::mem::details

    using  details::AlignOf;
    using  details::align_of;

} // namespace ustl::mem

#endif // #ifndef USTL_MEM_ALIGN_OF_H