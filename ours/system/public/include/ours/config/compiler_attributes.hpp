// ours/config OURS/CONFIG_COMPILER_ATTRIBUTES_HPP
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

#ifndef OURS_CONFIG_COMPILER_ATTRIBUTES_HPP
#define OURS_CONFIG_COMPILER_ATTRIBUTES_HPP 1

#define PACKED	                    [[gnu::packed]]
#define ALIGNED(x)	                [[gnu::aligned]]

#define NOINLINE	                [[gnu::noinline]]
#define NO_RETURN	                [[noreturn]]
#define FORCE_INLINE	            [[gnu::always_inline]] inline

#define WEAK                        [[gnu::weak]]
#define LINK_NAME(name)             asm(name)
#define LINK_SECTION(name)          [[gnu::section(name)]]

/// TSA: Thread safety analysis
#define TSA_GUARD(REQUIRE)          __attribute__((guarded_by(REQUIRE)))

#endif // #ifndef OURS_CONFIG_COMPILER_ATTRIBUTES_HPP