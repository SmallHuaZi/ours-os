# Interface protocol in `mem` module 
## 1. Normal function interface

## 2. Type interface
Type: `class ArchVmAspace` <br>
Provider: `<arch/vm_aspace.hpp>` <br>
Requirement: Forced <br>
Interfaces:
1.  __static auto switch_context(ArchVmAspace *from, ArchVmAspace *to) -> void__ <br>
Description:

2. __auto init(VmasFlags flags) -> Result<>__ <br>
Description:

3. __auto auto map(VirtAddr va, PhysAddr pa, usize n, MmuFlags flags, MapControl control) -> ustl::Result<usize, Status>__ <br>
Description:
    From `|pa|` map `|n|` page frames to `va`, through specifing `|control|`

4. __auto map_bulk(VirtAddr, PhysAddr *, usize n, MmuFlags flags, MapControl control) -> Status__ <br>
Description:

5. __auto unmap(VirtAddr, usize n, UnmapControl control) -> Status__ <br>
Description:

6. __auto protect(VirtAddr, usize n, MmuFlags) -> Status__ <br>
Description:

7. __auto query(VirtAddr, ai_out PhysAddr *, ai_out MmuFlags *) -> Status <br>
Description:

8. __auto mark_accessed(VirtAddr, usize) -> Status <br>
Description:

9. __auto harvest_accessed(VirtAddr va, usize n, HarvestControl action) -> Status <br>
Description:

Show you a simple example code:
```c++
class ArchVmAspace
{
public:
    ArchVmAspace(VirtAddr base, usize size, VmasFlags flags);

    static auto switch_context(Self *from, Self *to) -> void;

    auto init() -> Status;

    auto map(VirtAddr, PhysAddr, usize n, MmuFlags flags, MapControl control) -> ustl::Result<usize, Status>;

    auto map_bulk(VirtAddr, PhysAddr *, usize n, MmuFlags flags, MapControl control) -> Status;

    auto unmap(VirtAddr, usize n, UnmapControl control) -> Status;

    auto protect(VirtAddr, usize n, MmuFlags) -> Status;

    auto query(VirtAddr, ai_out PhysAddr *, ai_out MmuFlags *) -> Status;

    auto mark_accessed(VirtAddr, usize) -> Status
    {  return Status::Unsupported;  }

    auto harvest_accessed(VirtAddr va, usize n, HarvestControl action) -> Status;
};
```
