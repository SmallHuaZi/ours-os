#ifndef USTL_MEM_BOX_H
#define USTL_MEM_BOX_H 1

#include <memory>

namespace ustl::mem {
    template <typename T>
    using Box = ::std::unique_ptr<T>;

} // namespace ustl::mem

#endif // #ifndef USTL_MEM_BOX_H