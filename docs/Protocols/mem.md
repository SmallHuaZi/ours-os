# Interface protocol in `mem` module 
## 1. Normal function interface

## 2. Type interface
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
