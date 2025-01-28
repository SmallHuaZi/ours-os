#ifndef USTL_VIEWS_STRING_VIEW_H
#define USTL_VIEWS_STRING_VIEW_H

#include <string_view>

namespace ustl::views {
    template <typename T>
    using BasicStringView = std::basic_string_view<T>;

    typedef BasicStringView<char>   StringView;

    typedef BasicStringView<wchar_t>   WStringView;

} // namespace ustl::views

#endif // #ifndef USTL_COLLECTIONS_STRING_VIEW_H