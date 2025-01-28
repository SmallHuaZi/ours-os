# Interface protocol in `mem` module 
## 1. Normal function interface

## 2. Type interface
Type: `[enum] MmuFlags` <br>
Provider: `<arch/mmu_flags.hpp>` <br>
Requirement: Forced <br>
Interfaces:
1. __Present__ <br>
Description: Indentifies whether a block of memory is present. <br>

2. __Writable__ <br>
Description: Indentifies whether a block of memory is writable. <br>

3. __Readable__ <br>
Description: Indentifies whether a block of memory is readable. <br>

4. __Executable__ <br>
Description: Indentifies whether a block of memory is executable. <br>

5. __Discache__ <br>
Description: Don't premit a block of memory putted into CPU cache. <br>

6. __Dirty__ <br>
Description: Indentifies that a block of memory has been modified. <br>

7. __Supervisor__ <br>
Description: <br>

8. __Accessed__ <br>
Description: <br>

Show you a simple example from x86 architecture code:
```c++
enum X86MmuFLags {
    Present       = 0x0001, // Interface
    Readable      = 0x0002, // Interface
    Supervisor    = 0x0004, // Interface
    WriteThrough  = 0x0008,
    Discache      = 0x0010, // Interface
    Accessed      = 0x0020, // Interface
    Dirty         = 0x0040, // Interface
    PAGE_SIZE  = 0x0080,
    Pat4k     = 0x0080,
    Pat       = 0x0080,
    Global    = 0x0080,
};

using MmuFlags = X86MmuFLags;

```
<br>

Type: `class ArchVmAspace` <br>
Provider: `<arch/vm_aspace.hpp>` <br>
Requirement: Forced <br>
Interfaces:
1.  __static auto context_switch(ArchVmAspace *from, ArchVmAspace *to) -> void__ <br>
Description:

2. __auto init(VmAspace *high) -> Result<>__ <br>
Description:

3. __auto map_to(PhysAddr, VirtAddr, usize n, u64 flags) -> Result<>__ <br>
Description:

4. __auto unmap(VirtAddr, usize n, u64 flags) -> Result<>__ <br>
Description:

5. __auto unmap(VirtAddr, usize n, u64 flags) -> Result<>__ <br>
Description:

Show you a simple example code:
```c++
class ArchVmAspace
{
    typedef ArchVmAspace  Self;

public:
    static auto context_switch(Self *from, Self *to) -> void;

    auto init(gktl::Range<VirtAddr>, u64 flags) -> Result;

    auto map(VirtAddr, usize n, u64 flags) -> Result;

    auto map_to(PhysAddr, VirtAddr, usize n, u64 flags, PhysAlloc alloc) -> Result;

    auto unmap(VirtAddr, usize n, u64 flags) -> Result;

    auto which_cpu() const -> usize;

private:
    X86PageTable page_table_;
};
```
<br>

Type: `struct PagingConfig` <br>
Provider: `<arch/paging_config.hpp>` <br>
Requirement: Optional <br>
Interfaces:
1. __static auto index_of(VirtAddr adr, usize level) -> usize__ <br>
2. __PAGE_SIZE__: <br>
Show you a simple example code:
```c++
struct PagingConfig {paging_config
    enum :u64 { PAGE_SIZE = 4096 };

    static auto index_of(VirtAddr virt_addr, usize level) -> usize
    {
        switch (level) {
            case 0: return virt_addr & PTE_L0_MASK;
            case 1: return virt_addr & PTE_L1_MASK;
            case 2: return virt_addr & PTE_L2_MASK;
            case 3: return virt_addr & PTE_L3_MASK;
            default: panic("Invalid paging level");
        }
    }
};
```
