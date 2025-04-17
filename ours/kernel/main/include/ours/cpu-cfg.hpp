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
#ifndef OURS_CPU_CFG_HPP
#define OURS_CPU_CFG_HPP 1

#ifdef OURS_CONFIG_MAX_CPU_NUM 
#   define MAX_CPU OURS_CONFIG_MAX_CPU_NUM 
#else
#   define MAX_CPU 32 
#endif // #ifndef OURS_MAX_CPU_NUM

#endif // #ifndef OURS_CPU_CFG_HPP