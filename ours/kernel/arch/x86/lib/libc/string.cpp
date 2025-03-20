#include <cstring>
#include <ours/config.hpp>

NO_MANGLE
auto memmove(void *d, void const *s, size_t n) -> void * {
    auto p = static_cast<char const *>(s);
    auto q = static_cast<char *>(d);
	if (q < p) {
		__asm__  volatile("cld; rep; movsb"
			     : "+c" (n), "+S"(p), "+D"(q));
	} else {
		p += (n - 1);
		q += (n - 1);
		__asm__ volatile("std; rep; movsb; cld"
			     : "+c" (n), "+S"(p), "+D"(q));
	}

    return d;
}