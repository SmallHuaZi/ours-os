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
#ifndef OURS_CONFIG_IF_CFG_HPP
#define OURS_CONFIG_IF_CFG_HPP 1

#define OURS_INTERNAL_SELECTOR_0(then, else)  else 
#define OURS_INTERNAL_SELECTOR_1(then, else)  then
#define OURS_INTERNAL_UNWRAP(selector, then, else) selector(then, else) 
#define __OURS_IF_CFG_ELSE(option, then, else) \
    OURS_INTERNAL_UNWRAP(OURS_INTERNAL_SELECTOR_##option, then, else)
#define _OURS_IF_CFG_ELSE(option, then, else) \
    __OURS_IF_CFG_ELSE(option, then, else)
#define OURS_IF_CFG_ELSE(option, then, else) \
    _OURS_IF_CFG_ELSE(OURS_CONFIG_##option, then, else)

#define __OURS_INPLACE_HOLDER
#define _OURS_INPLACE_HOLDER  __OURS_INPLACE_HOLDER
#define OURS_INPLACE_HOLDER  _OURS_INPLACE_HOLDER

#define OURS_IF_CFG(option, then) \
    _OURS_IF_CFG_ELSE(OURS_CONFIG_##option, then, OURS_INPLACE_HOLDER)

#define OURS_IF_NOT_CFG(option, then) \
    _OURS_IF_CFG_ELSE(OURS_CONFIG_##option, OURS_INPLACE_HOLDER, else)

#endif // #ifndef OURS_CONFIG_IF_CFG_HPP
