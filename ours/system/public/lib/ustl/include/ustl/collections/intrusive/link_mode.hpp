#ifndef COLLECTIONS_INTRUSIVE_LINK_MODE_HPP
#define COLLECTIONS_INTRUSIVE_LINK_MODE_HPP

#include <boost/intrusive/link_mode.hpp>
#include <boost/intrusive/options.hpp>

namespace ustl::collections::intrusive {
    constexpr
    static auto const AutoUnlink = boost::intrusive::auto_unlink;

    constexpr
    static auto const NormalLink = boost::intrusive::normal_link;

    constexpr
    static auto const SafeLink = boost::intrusive::safe_link;

    enum class LinkModeType {
        //!If this linking policy is specified in a value_traits class
        //!as the link_mode, containers
        //!configured with such value_traits won't set the hooks
        //!of the erased values to a default state. Containers also won't
        //!check that the hooks of the new values are default initialized.
        NormalLink,

        //!If this linking policy is specified in a value_traits class
        //!as the link_mode, containers
        //!configured with such value_traits will set the hooks
        //!of the erased values to a default state. Containers also will
        //!check that the hooks of the new values are default initialized.
        SafeLink,

        //!Same as "safe_link" but the user type is an auto-unlink
        //!type, so the containers with constant-time size features won't be
        //!compatible with value_traits configured with this policy.
        //!Containers also know that the a value can be silently erased from
        //!the container without using any function provided by the containers.
        AutoUnlink
    };

    template <LinkModeType Mode>
    using LinkMode = boost::intrusive::link_mode<boost::intrusive::link_mode_type(Mode)>;
}

#endif // COLLECTIONS_INTRUSIVE_LINK_MODE_HPP
