#ifndef USTL_VIEWS_FILTER_VIEW_H
#define USTL_VIEWS_FILTER_VIEW_H

#include <ranges>

namespace ustl::views {
    using std::ranges::views::filter;
    template <typename T, typename Iterator>
    using FilterView = std::ranges::filter_view<T, Iterator>;
}

#endif // #ifndef USTL_VIEWS_FILTER_VIEW_H