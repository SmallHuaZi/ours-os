#include <ours/types.hpp>
#include <ours/config.hpp>

using namespace ours;

NO_MANGLE 
auto memcpy(void *d, void const *s, usize len) -> void *
{
    auto md = static_cast<char *>(d);
    auto ms = static_cast<char const *>(s);

    while (len--) {
        *md++ = *ms++;
    }

    return d;
}

NO_MANGLE 
auto memmove(void *d, const void *s, usize n) -> void *
{
    auto md = static_cast<char *>(d);
    auto ms = static_cast<char const *>(s);

    if (usize(ms) > usize(md)) {
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
auto memset(void *s, int c, usize n) -> void *
{
    auto md = static_cast<usize *>(s);
    while (n-- != 0) {
        *md = c;
        ++md;
    }

    return s;
}

NO_MANGLE 
auto memcmp(void const *x, void const *y, usize len) -> int
{
    auto mx = static_cast<u8 const *>(x);
    auto my = static_cast<u8 const *>(y);

    while (len-- != 0 && *mx == *my) {
         (void)++mx, (void)++my;
    }

    return len ? (*mx > *my ? 1 : -1) : 0;
}

NO_MANGLE 
auto memchr(void const *x, char const f, usize len) -> void *
{
    auto mx = static_cast<char const *>(x);
    auto range_end = static_cast<char const *>(x) + len;

    while (f != *mx && mx != range_end) {
        ++mx;
    }

    return (void *)(mx == range_end ? 0 : mx);
}

NO_MANGLE 
auto strlen(const char* p) -> usize
{
    usize n = 0;
    while (*p++ != 0) {
        n += 1;
    }
    return n;
}