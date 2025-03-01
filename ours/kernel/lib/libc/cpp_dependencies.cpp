//
// Created by smallhuazi on 11/14/24.
//
#include "ours/types.hpp"
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

    int __udivdi3(ours::u64, ours::u32) noexcept 
    { 
        ours::panic(__func__);
        return 0; 
    }
}


namespace std {
    // Must explicit instanilization
    template class basic_string<char>;

    exception::~exception() noexcept
    {}

    auto exception::what()  const noexcept -> char const *
    {  return "ours not exception";  }

#ifdef __GLIBCXX__
    __cow_string::__cow_string()
    {}

    __cow_string::~__cow_string()
    {}
#endif

    runtime_error::runtime_error(char const *)
#ifdef _LIBCPP_VERSION
        : __imp_("")
#endif
    {}

    runtime_error::runtime_error(string const &)
#ifdef _LIBCPP_VERSION
        : __imp_("")
#endif
    {}

    runtime_error::~runtime_error() noexcept
    {}

    auto runtime_error::what() const noexcept -> char const *
    {  return "ours not exception";  }

    auto __throw_out_of_range_fmt(char const *fmt, ...) -> void
    { ours::panic(fmt);  }

#ifdef _LIBCPP_STD_VER
inline namespace __1 {
#define TO_CHARS_ATTRS \
    _LIBCPP_AVAILABILITY_TO_CHARS_FLOATING_POINT _LIBCPP_EXPORTED_FROM_ABI 

    __libcpp_refstring::__libcpp_refstring(char const *)
    {}

    __libcpp_refstring::~__libcpp_refstring()
    {}
#define GLIBCXX_NOEXCEPT
#else
#define TO_CHARS_ATTRS 
#define GLIBCXX_NOEXCEPT noexcept
#endif
    // The functions bottom should never be used, because kernel be unable to use float point number.
    TO_CHARS_ATTRS 
    to_chars_result to_chars(char* __first, char* __last, float __value) GLIBCXX_NOEXCEPT  
    { ours::panic(__func__);  }

    TO_CHARS_ATTRS 
    to_chars_result to_chars(char* __first, char* __last, double __value) GLIBCXX_NOEXCEPT  
    { ours::panic(__func__);  }

    TO_CHARS_ATTRS 
    to_chars_result to_chars(char* __first, char* __last, long double __value) GLIBCXX_NOEXCEPT  
    { ours::panic(__func__);  }

    TO_CHARS_ATTRS 
    to_chars_result to_chars(char* __first, char* __last, float __value, chars_format __fmt) GLIBCXX_NOEXCEPT  
    { ours::panic(__func__);  }

    TO_CHARS_ATTRS 
    to_chars_result to_chars(char* __first, char* __last, double __value, chars_format __fmt) GLIBCXX_NOEXCEPT  
    { ours::panic(__func__);  }

    TO_CHARS_ATTRS 
    to_chars_result to_chars(char* __first, char* __last, long double __value, chars_format __fmt)GLIBCXX_NOEXCEPT   
    { ours::panic(__func__);  }

    TO_CHARS_ATTRS 
    to_chars_result to_chars(char* __first, char* __last, float __value, chars_format __fmt, int __precision)GLIBCXX_NOEXCEPT    
    { ours::panic(__func__);  }

    TO_CHARS_ATTRS 
    to_chars_result to_chars(char* __first, char* __last, double __value, chars_format __fmt, int __precision)GLIBCXX_NOEXCEPT     
    { ours::panic(__func__);  }

    TO_CHARS_ATTRS 
    to_chars_result to_chars(char* __first, char* __last, long double __value, chars_format __fmt, int __precision)GLIBCXX_NOEXCEPT      
    { ours::panic(__func__);  }

    [[noreturn]]
    void __libcpp_verbose_abort(char const* format, ...)
    { 
        ours::panic(format); 
    }
#ifdef _LIBCPP_STD_VER
}
#endif
namespace placeholders {
#ifdef _LIBCPP_STD_VER
_LIBCPP_EXPORTED_FROM_ABI const __ph<1> _1;
_LIBCPP_EXPORTED_FROM_ABI const __ph<2> _2;
_LIBCPP_EXPORTED_FROM_ABI const __ph<3> _3;
_LIBCPP_EXPORTED_FROM_ABI const __ph<4> _4;
_LIBCPP_EXPORTED_FROM_ABI const __ph<5> _5;
_LIBCPP_EXPORTED_FROM_ABI const __ph<6> _6;
_LIBCPP_EXPORTED_FROM_ABI const __ph<7> _7;
_LIBCPP_EXPORTED_FROM_ABI const __ph<8> _8;
_LIBCPP_EXPORTED_FROM_ABI const __ph<9> _9;
_LIBCPP_EXPORTED_FROM_ABI const __ph<10> _10;
#endif
}
}

#include <new>

[[gnu::weak]]
auto operator new(std::size_t) -> void *
{  
    ours::panic(__func__);
}

[[gnu::weak]]
auto operator delete(void *) -> void
{
    using std::to_chars;
}

namespace boost {
namespace container { 
   __attribute__((noreturn)) void throw_bad_alloc()
   { ours::panic(__func__); }

   __attribute__((noreturn)) void throw_out_of_range(const char* str)
   { ours::panic(__func__); }

   __attribute__((noreturn)) void throw_length_error(const char* str)
   { ours::panic(__func__); }

   __attribute__((noreturn)) void throw_logic_error(const char* str)
   { ours::panic(__func__); }

   __attribute__((noreturn)) void throw_runtime_error(const char* str)
   { ours::panic(__func__); }

   __attribute__((noreturn)) void throw_runtime_error(std::exception const & e)
   { ours::panic(__func__); }
}

   __attribute__((noreturn)) void throw_exception( std::exception const & e )
   { ours::panic(__func__); }

   __attribute__((noreturn)) void throw_exception( std::exception const & e, boost::source_location const & loc ) // user defined
   { ours::panic(__func__); } 

    void assertion_failed(char const * expr, char const * function, char const * file, long line) // user defined
    { ours::panic(__func__); }

    void assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line) // user defined
    { ours::panic(__func__); }
}