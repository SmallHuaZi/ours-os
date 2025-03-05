#ifndef USTL_UTIL_ASSERT_H
#define USTL_UTIL_ASSERT_H 1

#include <ustl/config.hpp>

namespace ustl {
namespace details {
    struct AssertionMessage
    {
    private:
        typedef AssertionMessage    Self;

    public:
        AssertionMessage() USTL_NOEXCEPT 
            : _next(0), _msglen(0), _message()
        {}

        Self &operator<<(bool msg) USTL_NOEXCEPT;

        Self &operator<<(char msg) USTL_NOEXCEPT;

        Self &operator<<(isize msg) USTL_NOEXCEPT;

        Self &operator<<(usize msg) USTL_NOEXCEPT;

        Self &operator<<(char const *msg) USTL_NOEXCEPT;

        Self &operator<<(wchar_t const *msg) USTL_NOEXCEPT;

        Self &operator<<(Self const &msg) USTL_NOEXCEPT;

        char const *message() USTL_NOEXCEPT
        {  return _message;  }

    private:
        enum { MessageBufferSize = 64 };

        i32  _msglen;
        char _message[MessageBufferSize];
        AssertionMessage const *_next;
    };

    template <typename OutStream>
    OutStream &operator<<(OutStream &out, AssertionMessage const &msg)
    {
        AssertionMessage const *first = &msg;
        while (first) {
            out << first->message() << ' ';
            first = first->_next;
        }

        return out;
    }

    template <typename OStream>
    OStream &operator<<(OStream &, AssertionMessage const &);

    struct AssertionResult
    {
        typedef AssertionResult  Self;
        typedef AssertionMessage Message;

    public:
        USTL_FORCEINLINE AssertionResult() USTL_NOEXCEPT
            : _success(false), _message({})
        {}

        template <typename T>
        USTL_FORCEINLINE AssertionResult(T const &success) USTL_NOEXCEPT
            : _success(success)
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR operator bool() const USTL_NOEXCEPT
        {  return _success;  }

        USTL_FORCEINLINE USTL_CONSTEXPR Message const &message() USTL_NOEXCEPT
        {  return _message;  }

        USTL_FORCEINLINE Self operator!() const USTL_NOEXCEPT
        {  return Self {!this->_success};  }

        template <typename T>
        USTL_FORCEINLINE USTL_CONSTEXPR Self &operator<<(T const &value) USTL_NOEXCEPT
        {
            _message << value;
            return *this;
        }

    private:
        bool _success;
        Message _message;
    }; // struct AssertionResult

    USTL_FORCEINLINE  AssertionResult assertion_success() USTL_NOEXCEPT
    {  return AssertionResult{true};  }

    USTL_FORCEINLINE  AssertionResult assertion_failure() USTL_NOEXCEPT
    {  return AssertionResult{false};  }

    USTL_FORCEINLINE  AssertionResult assertion_failure(AssertionMessage const &msg) USTL_NOEXCEPT
    {  return AssertionResult{false} << msg;  }

} // namespace ustl::details

    template <typename >
    struct Assert
    {

    }; // struct Assert
    
} // namespace ustl


#endif // USTL_UTIL_ASSERT_H