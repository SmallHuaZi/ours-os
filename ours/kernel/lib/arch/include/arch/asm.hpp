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
#ifndef ARCH_ASM_HPP
#define ARCH_ASM_HPP 1

#include <arch/x86/asm.hpp>

/// For functions that don't have an "end" or don't want .cfi_startproc
#define LOCAL_FUNCTION_NOCFI(x)     .type x,STT_FUNC; x:
#define FUNCTION_NOCFI(x)           .global x; .hidden x; LOCAL_FUNCTION_NOCFI(x)

#define LOCAL_FUNCTION(x)   LOCAL_FUNCTION_NOCFI(x) .cfi_startproc
#define FUNCTION(x)         .global x; .hidden x; LOCAL_FUNCTION(x)
#define FUNCTION_END(x)     .cfi_endproc; .size x, . - x

#define LOCAL_DATA(x)   .type x,STT_OBJECT; x:
#define DATA(x)         .global x; .hidden x; LOCAL_DATA(x)
#define DATA_END(x)     .size x, . - x

#define SIZE(...)       .quad  __VA_ARGS__

/// The following asm macros is from Zircon kernel.

// This macro is purged and redefined to indicate whether the assembly
// is currently between an entity-defining macro and its matching `.end_*`.
.macro _.entity.assert
    // Initial state: no entity being defined.
.endm

// Subroutine of _.entity.  Redefine the `.end_*` macros as errors.
.macro _.entity.reset purge=, text:vararg
    .ifnb \purge
        .purgem .end_function
        .purgem .end_object
    .endif
    .macro .end_function
        .error "unmatched .end_function directive \text"
    .endm  // .end_function
    .macro .end_object
        .error "unmatched .end_object directive \text"
    .endm  // .end_object
.endm  // _.entity.reset

// Subroutine of entity-defining macros below.  This resets the `.end_*`
// machinery so that it does `\epilogue` and then does `.size` for `\name`,
// via `_.entity.end`.  The \name, \scope, and \nosection arguments are
// passed through from caller and validated here; the rest are provided by
// each entity-defining macro.
.macro _.entity name, scope, align, nosection, retain, entity, type, epilogue
  // First make sure that entity and `.end_*` pairs are matched (no nesting).
    _.entity.assert
    .purgem _.entity.assert
    .macro _.entity.assert
        .error "missing `.end_\entity` for `.\entity \name`"
    .endm

    // Clear old `.end_*` macros and make the one other than `.end_\entity`
    // an error until this entity is finished.
    _.entity.reset purge, inside definition of `.\entity \name`

    // Now redefine the `.end_\entity` macro to pair with this entity.
    // This definition captures the parameters to pass on to `_.entity.end`.
    .purgem .end_\entity
    .macro .end_\entity
        _.entity.end \entity, \name, \nosection, \epilogue
    .endm

    // Unless given the `nosection` flag argument, enter a per-entity section.
    .ifb \nosection
        .ifnb \retain
            .ifnc \retain,R
                .error "retain argument to \entity directive must be `R` or empty"
            .endif
        .endif
        // The specific section name and details depend on the entity type.
        _.entity.pushsection.\type \name, \retain
    .else
        .ifnc \nosection, nosection
            .error "final argument to \entity directive must be exactly `nosection`"
        .endif
    .endif

    // Align within the section.
    .ifnb \align
        .balign \align
    .endif

    // Finally, define the actual label.  The entity's own prologue comes next.
    .label \name, \scope, \entity
.endm  // _.entity

// Subroutines of _.entity selected by the \type argument.

.macro _.entity.pushsection.function name, retain
#ifdef __ELF__
  // The function goes into the .text section in its own section group.
  // This lets any metadata associated with the function travel in its
  // group by using `.pushsection .metadata-section, "...?", ...`.
    .pushsection .text, "axG\retain", %progbits, \name
#else
    .text
#endif
.endm

.macro _.entity.pushsection.bss name, retain
#ifdef __ELF__
    .pushsection .bss.\name, "aw\retain", %nobits
#else
    .bss
#endif
.endm

.macro _.entity.pushsection.data name, retain
#ifdef __ELF__
    .pushsection .data.\name, "aw\retain", %progbits
#else
    .data
#endif
.endm

.macro _.entity.pushsection.relro name, retain
#ifdef __ELF__
    .pushsection .data.rel.ro.\name, "aw\retain", %progbits
#else
    .section .rdata, "dr"
#endif
.endm

.macro _.entity.pushsection.rodata name, retain
#ifdef __ELF__
    .pushsection .rodata.\name, "a\retain", %progbits
#else
    .section .rdata, "dr"
#endif
.endm

// Subroutine of `.end_\entity` macros defined by `_.entity`, above.
.macro _.entity.end entity, name, nosection, epilogue
    // First, reset the assertion machinery.
    .purgem _.entity.assert
    .macro _.entity.assert
    .endm

    // Now redefine `.end_*` so it's an error with no matching entity.
    _.entity.reset purge

    // Do the epilogue for the entity, e.g. .cfi_endproc.
    \epilogue

#ifdef __ELF__
    // Set the ELF symbol's `st_size`.
    .size \name, . - \name

    // Leave the per-entity section, if any.
    .ifb \nosection
        .popsection
    .endif
#endif
.endm  // _.entity.end

// Subroutines of .function, start/end pairs for each `cfi` mode.

.macro _.function.start.abi
    .cfi_startproc
#ifdef __arm__
    .fnstart
#endif
    .endm
    .macro _.function.end.abi
#ifdef __arm__
    .fnend
#endif
    .cfi_endproc
.endm

.macro _.function.start.custom
    .cfi_startproc simple
#ifdef __arm__
    .fnstart
#endif
.endm
.macro _.function.end.custom
#ifdef __arm__
    .fnend
#endif
    .cfi_endproc
.endm

.macro _.function.start.none
.endm
.macro _.function.end.none
.endm

// Catch stray `.end_*` before any entity-defining macros.
_.entity.reset

/// Defines an ELF symbol at the current assembly position (or with an
/// arbitrary value), with specified scope and (optional) type.
///
/// Parameters
///
///   * name
///     - Required: Symbol name to define.
///
///   * scope
///     - Optional: One of these strings:
///       - `local`: The symbol is visible only within this assembly file.
///       - `global`: The symbol is visible throughout this linked module.
///       - `export`: The symbol is exported for dynamic linking (user mode).
///       - `weak`: Like `export`, but can be overridden.
///     - Default: `local`
///
///   * type
///     - Optional: ELF symbol type.  This only has practical effect when
///     dynamic linking is involved, but it's convention to set it consistently
///     to `function` or `object` for named entities with an st_size field,
///     and leave it the default `notype` only for labels within an entity.
///     - Default: `notype`
///
///   * value
///     - Optional: Expression for the value of the symbol.  Usually this is
///     just `.` (the default if it's omitted), but it can be another value.
///     - Default: `.`
///
///
/// This is only really useful when the scope and/or type is set to a
/// non-default value.  `.label name, local` is just `name:`.
.macro .label name, scope=local, type=notype, value:vararg
#ifdef __ELF__
    // Set ELF symbol type.
    .type \name, %\type
#endif

    // Set ELF symbol visibility and binding, which represent scope.
    .ifnb \scope
        .ifnc \scope, local
            .ifc \scope, weak
                .weak \name
            .else
                .globl \name
                .ifc \scope, global
#ifdef __ELF__
                    .hidden \name
#endif
                .else
                    .ifnc \scope, export
                        .error "`scope` argument `\scope` not `local`, `global`, `export`, or `weak`"
                    .endif
                .endif
            .endif
        .endif
    .endif

    // Define the label itself.
    .ifb \value
        \name\():
    .else
        \name = \value
    .endif
.endm  // .label

/// Define a function that extends until `.end_function`.
///
/// Parameters
///
///   * name
///     - Required: Symbol name to define.
///
///   * scope
///     - Optional: See `.label`.
///     - Default: `local`
///
///   * cfi
///     - Optional: One of the strings:
///       - `abi`: This is a function with the standard C++ ABI.
///       - `custom`: This function includes `.cfi_*` directives that
///       describe its unwinding requirements completely.
///       - `none`: Don't emit normal function CFI for this function.
///     - Default: `abi`
///
///   * align
///     - Optional: Minimum byte alignment for this function's code.
///     - Default: none
///
///   * nosection
///     - Optional: Must be exactly `nosection` to indicate this function goes
///     into the assembly's current section rather than a per-function section.
///
///   * retain
///     - Optional: `R` for SHF_GNU_RETAIN, empty for not.
///     - Default: ``
///
.macro .function name, scope=local, cfi=abi, align=, nosection=, retain=
    // Validate the \cfi argument.  The valid values correspond to
    // the `_.function.cfi.{start,end}.\cfi` subroutine macros.
    .ifnc \cfi, abi
        .ifnc \cfi, custom
            .ifnc \cfi, none
                .error "`cfi` argument `\cfi` not `abi`, `custom`, or `none`"
            .endif
        .endif
    .endif

    _.entity \name, \scope, \align, \nosection, \retain, function, function, _.function.end.\cfi
    _.function.start.\cfi
.endm  // .function

/// Define a data object that extends until `.end_object`.
///
/// This starts the definition of a data object and is matched by
/// `.end_object` to finish that object's definition.  `.end_object` must
/// appear before any other `.object` or `.function` directive.
///
/// Parameters
///
///   * name
///     - Required: Symbol name to define.
///
///   * type
///     - Optional: One of the strings:
///       - `bss`: Define a zero-initialized (.bss) writable data object.
///       This is usually followed by just a `.skip` directive and then
///       `.end_object`.
///       - `data`: Define an initialized writable data object.  This is
///       followed by data-emitting directives (`.int` et al) to provide
///       the initializer, and then `.end_object`.
///       - `relro`: Define a read-only initialized data object requiring
///       dynamic relocation.  Use this instead of `rodata` if initializer
///       data includes any absolute address constants.
///       - `rodata`: Define a pure read-only initialized data object.
///     - Default: `data`
///
///   * scope
///     - Optional: See `.label`.
///     - Default: `local`
///
///   * align
///     - Optional: Minimum byte alignment for this function's code.
///     - Default: none
///
///   * nosection
///     - Optional: Must be exactly `nosection` to indicate this object goes
///     into the assembly's current section rather than a per-object section.
///
///   * retain
///     - Optional: `R` for SHF_GNU_RETAIN, empty for not.
///     - Default: ``
///
.macro .object name, type=data, scope=local, align=, nosection=, retain=
    .ifnc \type, bss
        .ifnc \type, data
            .ifnc \type, relro
                .ifnc \type, rodata
                    .error "`type` argument `\type` not `bss`, `data, `relro`, or `rodata`"
                .endif
            .endif
        .endif
    .endif
    _.entity \name, \scope, \align, \nosection, \retain, object, \type
.endm  // .start_object

#endif // #ifndef ARCH_ASM_HPP