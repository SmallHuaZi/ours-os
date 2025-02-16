//
// Created by smallhuazi on 11/14/24.
//
#ifdef _MSC_VER
extern "C" auto __cdecl _purecall() -> void {}
#endif

extern "C" auto __cxa_pure_virtual() -> void
{}

void *__dso_handle;

extern "C" auto __cxa_atexit(void (*dtor) (void *), void *arg, void *dso) -> void
{}

extern "C" auto __cxa_finalize(void *f) -> void
{}

/* guard variables */
/* The ABI requires a 64-bit type.  */
__extension__ typedef int guard __attribute__((mode(__DI__)));

extern "C" int __cxa_guard_acquire(guard *g)
{}

extern "C" void __cxa_guard_release(guard *g)
{}

extern "C" void __cxa_guard_abort(guard *)
{}

extern "C" void atexit()
{}
