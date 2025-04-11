#ifndef USTL_VIEWS_TRANSFORM_VIEW_H
#define USTL_VIEWS_TRANSFORM_VIEW_H

#include <ranges>

namespace ustl::views {
    using std::ranges::views::transform;
    template <typename T, typename F>
    using TransformView= std::ranges::transform_view<T, F>;
}

#endif // #ifndef USTL_VIEWS_FILTER_VIEW_H