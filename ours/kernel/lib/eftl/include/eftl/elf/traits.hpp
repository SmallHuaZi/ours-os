#ifndef EFTL_ELF_TRAITS_HPP
#define EFTL_ELF_TRAITS_HPP

#include <eftl/elf/details/types.hpp>
#include <eftl/elf/details/structures.hpp>

namespace eftl::elf {
    enum class ElfClass {
        None, 
        Elf32,
        Elf64,
        Native = sizeof(void *) == 8 ? Elf64 : Elf32,
    };

    template <ElfClass Class>
    struct ElfTraitsDispatcher;

    struct ElfSiginfo;
    struct ElfClassTraitsBase {
        CXX11_CONSTEXPR
        static const char kElfMagic[] = {'\x7f', 'E', 'L', 'F'};

        typedef ElfSiginfo      Siginfo;    // Signal information
    };

    struct Elf32Ehdr;
    struct Elf32Phdr;
    struct Elf32Shdr;
    struct Elf32Sym;
    struct Elf32Auxv;
    struct Elf32Dyn;
    struct Elf32Rel;
    struct Elf32Rela;
    struct Elf32Verneed;
    struct Elf32Verdef;
    struct Elf32Vernaux;
    struct Elf32Prstatus;
    struct Elf32FileEntry;
    struct Elf32Timeval;

    struct ElfTraits32: public ElfClassTraitsBase {
        typedef Elf32Addr   Addr;
        typedef Elf32Off    Off;
        typedef Elf32Half   Half;
        typedef Elf32Word   Word;
        typedef Elf32Sword  Sword;

        typedef Elf32Ehdr       Header;     // Elf header
        typedef Elf32Phdr       Segment;    // Program header
        typedef Elf32Shdr       Section;    // Section header
        typedef Elf32Sym        Symbol;     // Symbol table entry
        typedef Elf32Auxv       Auxv;       // Auxiliary vector
        typedef Elf32Dyn        Dyn;        // Dynamic table entry
        typedef Elf32Rel        Rel;        // Relocation entry
        typedef Elf32Rela       Rela;       // Relocation entry with explicit addend
        typedef Elf32Verneed    Verneed;    // Version needed
        typedef Elf32Verdef     Verdef;     // Version definition
        typedef Elf32Vernaux    Vernaux;    // Version auxiliary
        typedef Elf32Prstatus   Prstatus;   // Prstatus structure
        typedef Elf32FileEntry  FileEntry; // File entry
        typedef Elf32Timeval    Timeval;    // Time value
    };

    template <>
    struct ElfTraitsDispatcher<ElfClass::Elf32> {
        typedef ElfTraits32 Type;
    };

    struct Elf64Ehdr;
    struct Elf64Phdr;
    struct Elf64Shdr;
    struct Elf64Sym;
    struct Elf64Auxv;
    struct Elf64Dyn;
    struct Elf64Rel;
    struct Elf64Rela;
    struct Elf64Verneed;
    struct Elf64Verdef;
    struct Elf64Vernaux;
    struct Elf64Prstatus;
    struct Elf64FileEntry;
    struct Elf64Timeval;

    struct ElfTraits64: public ElfClassTraitsBase {
        typedef Elf64Addr   Addr;
        typedef Elf64Off    Off;
        typedef Elf64Half   Half;
        typedef Elf64Word   Word;
        typedef Elf64Sword  Sword;

        typedef Elf64Ehdr       Header;     // Elf header
        typedef Elf64Phdr       Segment;    // Program header
        typedef Elf64Shdr       Section;    // Section header
        typedef Elf64Sym        Symbol;     // Symbol table entry
        typedef Elf64Auxv       Auxv;       // Auxiliary vector
        typedef Elf64Dyn        Dyn;        // Dynamic table entry
        typedef Elf64Rel        Rel;        // Relocation entry
        typedef Elf64Rela       Rela;       // Relocation entry with explicit addend
        typedef Elf64Verneed    Verneed;    // Version needed
        typedef Elf64Verdef     Verdef;     // Version definition
        typedef Elf64Vernaux    Vernaux;    // Version auxiliary
        typedef Elf64Prstatus   Prstatus;   // Prstatus structure
        typedef Elf64FileEntry  FileEntry; // File entry
        typedef Elf64Timeval    Timeval;    // Time value
    };

    template <>
    struct ElfTraitsDispatcher<ElfClass::Elf64> {
        typedef ElfTraits64 Type;
    };

    using ElfTraitsNative = ElfTraitsDispatcher<ElfClass::Native>::Type;

} // namespace eftl::elf


#endif // EFTL_ELF_TRAITS_HPP
