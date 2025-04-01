#include <ours/panic.hpp>
#include <boost/assert/source_location.hpp>

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