#ifndef USTL_UTIL_NONCONSTRUCTIBLE_H
#define USTL_UTIL_NONCONSTRUCTIBLE_H 1

namespace ustl {
    struct NonConstructible
    {
        NonConstructible() = delete;
        ~NonConstructible() = delete;
    };
     
} // namespace ustl


#endif // #ifndef USTL_UTIL_NONCONSTRUCTIBLE_H