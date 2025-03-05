#ifndef USTL_MEM_ALIGNED_H
#define USTL_MEM_ALIGNED_H 1

#include <ustl/mem/align_of.hpp>

namespace ustl::mem::details {
    template <usize Alignment>
    struct alignas(Alignment) AlignedTo {};

    template <usize Size, usize Alignment>
    union AlignedImpl
    {
        auto address() -> void *
        {  return data; }
    
    private:
        u8 data[Size];
        AlignedTo<Alignment> aligned;
    };

    template <typename T, usize Alignment = alignof(T)>
    struct Aligned
    {
        auto as_ptr_mut() USTL_NOEXCEPT -> T *
        {  return static_cast<T *>(inner_.address());  }

        auto as_ptr() const USTL_NOEXCEPT -> T const *
        {  return static_cast<T const *>(inner_.address());  }

        auto as_ref_mut() USTL_NOEXCEPT -> T &
        {  return *this->ptr();  }

        auto as_ref() const USTL_NOEXCEPT -> T const &
        {  return *this->ptr();  }
    
    private:
        // Be tagged 'mutable' to avoid const_cast<> in 'as_ptr()'
        mutable AlignedImpl<sizeof(T), Alignment> inner_;
    };

} // namespace ustl::mem

#endif // #ifndef USTL_MEM_ALIGNED_H