#include <cstring>
#include <ours/config.hpp>
#include <ours/types.hpp>

NO_MANGLE WEAK
auto memcpy(void *d, void const *s, size_t n) -> void * {
    auto md = static_cast<char *>(d);
    auto ms = static_cast<char const *>(s);
    for (size_t i = 0; i < n; ++i) {
        md[i] = ms[i];
    }

    return d;
}

NO_MANGLE WEAK
auto memmove(void *d, void const *s, size_t n) -> void * {
    auto md = static_cast<char *>(d);
    auto ms = static_cast<char const *>(s);

    if (size_t(ms) > size_t(md)) {
        for (size_t i = 0; i < n; ++i) {
            md[i]= ms[i];
        }
    } else {
        for (size_t i = n; i > 0; --i) {
            md[i - 1]= ms[i - 1];
        }
    }

    return d;
}

NO_MANGLE WEAK
auto memset(void *s, int c, size_t n) -> void * {
    auto md = static_cast<u8 *>(s);
    for (auto i = 0; i < n; ++i) {
        md[i] = c;
    }

    return s;
}

NO_MANGLE WEAK
auto memcmp(void const *x, void const *y, size_t n) -> int {
    auto p1 = static_cast<char const *>(x);
    auto p2 = static_cast<char const *>(y);

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return (p1[i] - p2[i]);
        }
    }

    return 0;
}

WEAK
auto memchr(void const *str, int c, size_t n) noexcept -> void const * {
    auto p = static_cast<char const *>(str);

    for (size_t i = 0; i < n; i++) {
        if (p[i] == (char)c) {
            return (void *)p;
        }
    }

    return NULL;
}

NO_MANGLE WEAK
auto strlen(char const *p) -> size_t {
    size_t n = 0;
    while (*p++ != 0) {
        n += 1;
    }
    return n;
}