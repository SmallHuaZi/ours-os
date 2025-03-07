#include <cstdint>
#include <ours/config.hpp>
#include <ours/types.hpp>

NO_MANGLE
auto memcpy(void *d, void const *s, size_t len) -> void * {
    auto md = static_cast<char *>(d);
    auto ms = static_cast<char const *>(s);

    while (len--) {
        *md++ = *ms++;
    }

    return d;
}

NO_MANGLE
auto memmove(void *d, const void *s, size_t n) -> void * {
    auto md = static_cast<char *>(d);
    auto ms = static_cast<char const *>(s);

    if (size_t(ms) > size_t(md)) {
        while (n--) {
            *md++ = *ms++;
        }
    } else {
        for (ms += n, md += n; n--;) {
            *--md = *--ms;
        }
    }

    return d;
}

NO_MANGLE
auto memset(void *s, int c, size_t n) -> void * {
    auto md = static_cast<size_t *>(s);
    while (n-- != 0) {
        *md = c;
        ++md;
    }

    return s;
}

NO_MANGLE
auto memcmp(void const *x, void const *y, size_t len) -> int {
    auto mx = static_cast<char const *>(x);
    auto my = static_cast<char const *>(y);

    while (len-- != 0 && *mx == *my) {
        (void)++mx, (void)++my;
    }

    return len ? (*mx > *my ? 1 : -1) : 0;
}

NO_MANGLE
auto memchr(void const *x, char const f, size_t len) -> void * {
    auto mx = static_cast<char const *>(x);
    auto range_end = static_cast<char const *>(x) + len;

    while (f != *mx && mx != range_end) {
        ++mx;
    }

    return (void *)(mx == range_end ? 0 : mx);
}

NO_MANGLE
auto strlen(const char *p) -> size_t {
    size_t n = 0;
    while (*p++ != 0) {
        n += 1;
    }
    return n;
}