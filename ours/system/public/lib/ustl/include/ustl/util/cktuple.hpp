#ifndef USTL_UTIL_CKMAP_HPP
#define USTL_UTIL_CKMAP_HPP

namespace ustl::tvmap {
    template <auto Key, typename Value>
    struct Entry {
        Value value;
    };

    template <typename... Entries> 
    struct CkTuple {

    };

} // namespace ustl::tvmap

#endif // USTL_UTIL_TVMAP_HPP
