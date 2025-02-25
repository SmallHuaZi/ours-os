//
// Created by smallhuazi on 11/14/24.
//
#include <stdexcept>
namespace internal 
{
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
    { return 0; }

    extern "C" void __cxa_guard_release(guard *g)
    {}

    extern "C" void __cxa_guard_abort(guard *)
    {}

    extern "C" int atexit(void (*func)(void)) noexcept
    { return 0; }
}

namespace ours {
    [[noreturn]] auto panic() -> void;
}

#include <version>
#include <exception>
#include <boost/assert/source_location.hpp>

namespace std {
    exception::~exception() noexcept
    {}

    auto exception::what()  const noexcept -> char const *
    {  return "ours not exception";  }

    runtime_error::runtime_error(char const *)
        : __imp_("")
    {}

    runtime_error::runtime_error(string const &)
        : __imp_("")
    {}

    runtime_error::~runtime_error() noexcept
    {}

    auto runtime_error::what() const noexcept -> char const *
    {  return "ours not exception";  }

inline namespace __1 {
    __libcpp_refstring::__libcpp_refstring(char const *)
    {}

    __libcpp_refstring::~__libcpp_refstring()
    {}

    [[noreturn]]
    void __libcpp_verbose_abort(char const* format, ...)
    { 
        ours::panic(); 
    }
}

namespace placeholders {
// _LIBCPP_EXPORTED_FROM_ABI const __ph<1> _1;
// _LIBCPP_EXPORTED_FROM_ABI const __ph<2> _2;
// _LIBCPP_EXPORTED_FROM_ABI const __ph<3> _3;
// _LIBCPP_EXPORTED_FROM_ABI const __ph<4> _4;
// _LIBCPP_EXPORTED_FROM_ABI const __ph<5> _5;
// _LIBCPP_EXPORTED_FROM_ABI const __ph<6> _6;
// _LIBCPP_EXPORTED_FROM_ABI const __ph<7> _7;
// _LIBCPP_EXPORTED_FROM_ABI const __ph<8> _8;
// _LIBCPP_EXPORTED_FROM_ABI const __ph<9> _9;
// _LIBCPP_EXPORTED_FROM_ABI const __ph<10> _10;
}
}

namespace boost {
namespace container { 
   __attribute__((noreturn)) void throw_bad_alloc()
   { ours::panic(); }

   __attribute__((noreturn)) void throw_out_of_range(const char* str)
   { ours::panic(); }

   __attribute__((noreturn)) void throw_length_error(const char* str)
   { ours::panic(); }

   __attribute__((noreturn)) void throw_logic_error(const char* str)
   { ours::panic(); }

   __attribute__((noreturn)) void throw_runtime_error(const char* str)
   { ours::panic(); }

   __attribute__((noreturn)) void throw_runtime_error(std::exception const & e)
   { ours::panic(); }
}

   __attribute__((noreturn)) void throw_exception( std::exception const & e )
   { ours::panic(); }

   __attribute__((noreturn)) void throw_exception( std::exception const & e, boost::source_location const & loc ) // user defined
   { ours::panic(); } 

    void assertion_failed(char const * expr, char const * function, char const * file, long line) // user defined
    { ours::panic(); }

    void assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line) // user defined
    { ours::panic(); }
}