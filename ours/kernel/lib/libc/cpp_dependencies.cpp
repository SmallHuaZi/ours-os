#include <ours/panic.hpp>

#include <version>
#include <stdexcept>
#include <exception>
#include <functional>
#include <boost/assert/source_location.hpp>

void *__dso_handle;

extern "C" {
    auto __cdecl _purecall() -> void {}
    auto __cxa_pure_virtual() -> void {}
    auto __cxa_atexit(void (*dtor) (void *), void *arg, void *dso) -> void {}
    auto __cxa_finalize(void *f) -> void {}

    /* guard variables */
    /* The ABI requires a 64-bit type.  */
    __extension__ typedef int guard __attribute__((mode(__DI__)));

    auto __cxa_guard_acquire(guard *g) -> int { return 0; }
    void __cxa_guard_release(guard *g) {}
    void __cxa_guard_abort(guard *) {}
    
    int atexit(void (*func)(void)) noexcept { return 0; }
}

#include <new>

[[gnu::weak]]
auto operator new(std::size_t) -> void *
{  ours::panic(__func__);  }

[[gnu::weak]]
auto operator new(std::size_t, std::align_val_t) -> void *
{  ours::panic(__func__);  }

[[gnu::weak]]
auto operator delete(void *) -> void
{}

[[gnu::weak]]
auto operator delete(void *, std::size_t) -> void
{}

[[gnu::weak]]
auto operator delete(void *, std::size_t, std::align_val_t) -> void
{}
