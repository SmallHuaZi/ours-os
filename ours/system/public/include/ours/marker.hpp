#ifndef OURS_MARKER_HPP
#define OURS_MARKER_HPP 1

#include <ours/config.hpp>
#include <ustl/initializer_list.hpp>
#include <ustl/traits/integral_constant.hpp>

#define OURS_IMPL_MARKER_FOR(MARKER, TYPE)  \
    template <>                             \
    struct ours::marker::MARKER<TYPE>       \
        : public ustl::traits::TrueType     \
    {};

namespace ours::marker {
    template <typename T,
              template<typename U> class... MarkerList>
    struct HasImplementedMarker;

    template <typename T>
    struct HasImplementedMarker<T>
    {
        CXX11_CONSTEXPR
        static bool const VALUE = true;
    };

    template <typename T,
             template<typename U> class Marker,
             template<typename U> class... MarkerList>
    struct HasImplementedMarker<T, Marker, MarkerList...>
    {
        typedef HasImplementedMarker<T, MarkerList...>   RequiredOtherMarkers;

        CXX11_CONSTEXPR
        static bool const VALUE = {
            RequiredOtherMarkers::VALUE &&
            Marker<T>::VALUE
        };
    };

    template <typename T, template<typename U> class... MarkerList>
    CXX11_CONSTEXPR
    bool const HasImplementedV = HasImplementedMarker<T, MarkerList...>::VALUE;

    /// The \c UserObjectTraits and \c KernelObjectTraits
    /// is two exclusive marker.
    template <typename T>
    struct UserObject
        : public ::ustl::traits::FalseType
    {};

    template <typename T>
    struct KernelObject
        : public ::ustl::traits::FalseType
    {};

    /// If being implemented this marker, the object must be thread-safe.
    /// So that we can use it without mutex in multi-thread environment.
    template <typename T>
    struct Send
        : public ::ustl::traits::FalseType
    {};

    /// If being implemented this marker, the object must be thread-safe.
    /// So that we can use it without mutex in multi-thread environment.
    template <typename T>
    struct Sync
        : public ::ustl::traits::FalseType
    {};

} // namespace ours::marker

#endif // #ifndef OURS_MARKER_HPP