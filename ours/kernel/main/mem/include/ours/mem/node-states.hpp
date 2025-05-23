/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///
#ifndef OURS_MEM_NODE_STATES_HPP
#define OURS_MEM_NODE_STATES_HPP 1

#include <ours/mem/node-mask.hpp>
#include <ustl/array.hpp>

namespace ours::mem {
    struct NodeStates {
        enum Type {
            Dma = usize(ZoneType::Dma),

            Dma32 = usize(ZoneType::Dma32),

            Normal = usize(ZoneType::Normal),

            /// Indicate that the node hold memories.
            Memory,

            // When a node created but not initialized, it's state is possible,
            // Meaning that it become a online probablely during system runtime.
            Possible,

            // When a node created and initialized, it's state is online.
            // Meaning that it is active in system, so the de/allocation requests
            // on it is feasible.
            Online,
            MaxNumStateType,
        };

        FORCE_INLINE
        auto is_state(NodeId nid, Type state) -> bool {
            return states_[state].test(nid);
        }

        FORCE_INLINE
        auto is_online(NodeId nid) -> bool {
            return is_state(nid, Online);
        }

        FORCE_INLINE
        auto is_offline(NodeId nid) -> bool {
            return !is_online(nid);
        }

        FORCE_INLINE
        auto is_possible(NodeId nid) -> bool {
            return is_state(nid, Possible);
        }

        FORCE_INLINE
        auto set_state(NodeId nid, Type state, bool value = true) -> void {
            states_[state].set(nid, value);
        }

        FORCE_INLINE
        auto set_online(NodeId nid, bool online) -> void {
            set_state(nid, Online, online);
        }

        FORCE_INLINE
        auto set_possible(NodeId nid) -> void {
            set_state(nid, Possible);
        }

        template <typename F>
            CXX20_REQUIRES(ustl::traits::Invocable<F, NodeId>)
        FORCE_INLINE
        auto for_each_online(F &&functor) -> void {
            states_[Online].for_each(functor);
        }

        template <typename F>
            CXX20_REQUIRES(ustl::traits::Invocable<F, NodeId>)
        FORCE_INLINE
        auto for_each_possible(F &&functor) -> void {
            states_[Possible].for_each(functor);
        }

        template <typename F>
            CXX20_REQUIRES(ustl::traits::Invocable<F, NodeId>)
        FORCE_INLINE
        auto for_each_possible_if_no_err(F &&functor) -> Status {
            return states_[Possible].for_each(functor);
        }

        template <typename F>
            CXX20_REQUIRES(ustl::traits::Invocable<F, NodeId>)
        FORCE_INLINE
        auto for_each_state(Type type, F &&functor) -> void {
            states_[type].for_each(functor);
        }

        template <typename F>
            CXX20_REQUIRES(ustl::traits::Invocable<F, NodeId>)
        FORCE_INLINE
        auto find_possible(F &&functor) -> NodeId {
            return states_[Possible].find(functor);
        }

        ustl::Array<NodeMask, MaxNumStateType> states_;
    };

    FORCE_INLINE CXX11_CONSTEXPR
    auto global_node_states() -> NodeStates & {
        static NodeStates g_node_states;
        return g_node_states;
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto node_possible_mask() -> NodeMask & {
        return global_node_states().states_[NodeStates::Online];
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto num_possible_nodes() -> usize {
        return node_possible_mask().count();
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto node_online_mask() -> NodeMask & {
        return global_node_states().states_[NodeStates::Online];
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto num_online_nodes() -> usize {
        return node_online_mask().count();
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto node_is_state(NodeId nid, NodeStates::Type state) -> bool {
        return global_node_states().states_[state].test(nid);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto set_node_state(NodeId nid, NodeStates::Type state, bool online) -> void {
        global_node_states().set_state(nid, state, online);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto set_node_online(NodeId nid, bool online) -> void {
        set_node_state(nid, NodeStates::Online, online);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto set_node_dma(NodeId nid, bool online) -> void {
        set_node_state(nid, NodeStates::Dma, online);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto set_node_dma32(NodeId nid, bool online) -> void {
        set_node_state(nid, NodeStates::Dma32, online);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto set_node_normal(NodeId nid, bool online) -> void {
        set_node_state(nid, NodeStates::Normal, online);
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_NODE_STATES_HPP