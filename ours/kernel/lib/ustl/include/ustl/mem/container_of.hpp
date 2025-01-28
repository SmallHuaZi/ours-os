#ifndef USTL_MEM_CONTAINER_OF_HPP
#define USTL_MEM_CONTAINER_OF_HPP 1

#include <ustl/config.hpp>

namespace ustl::mem {
    template <typename Container, typename Member>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto offset_of(Member const Container::*ptr)
        USTL_NOEXCEPT -> isize
    {
        USTL_CONSTEXPR Container const *OWNER = 0;
        return reinterpret_cast<isize>
        (
            static_cast<void const *>(&(OWNER->*ptr))
        );
    }

    template <typename Container, typename Member>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto container_of(Member *member, Member Container::*ptr) 
        USTL_NOEXCEPT -> Container *
    {
        return static_cast<Container *>
        (
            static_cast<void *>
            (
                static_cast<char *>(static_cast<void *>(member)) - mem::offset_of(ptr)
            )
        );
    }

    template <typename Container, typename Member>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto container_of(Member const *member, Member const Container::*ptr) 
        USTL_NOEXCEPT -> Container const *
    {
        return static_cast<Container const *>
        (
            static_cast<void const *>
            (
                static_cast<char const *>(static_cast<void const *>(member)) - mem::offset_of(ptr)
            )
        );
    } 

} // namespace ustl::mem

#endif // #ifndef USTL_MEM_CONTAINER_OF_HPP