#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_mapping.hpp>
#include <ours/mem/object-cache.hpp>

#include <ours/arch/aspace_layout.hpp>

#include <ustl/lazy_init.hpp>
#include <ustl/sync/lockguard.hpp>

#include <logz4/log.hpp>
#include <gktl/init_hook.hpp>
#include <ktl/new.hpp>

namespace ours::mem {
    /// Manage the lifetime manually.
    /// The global-unique address sapce.
    static ObjectCache *s_aspace_cache;

    VmAspace::VmAspace(VirtAddr base, usize size, VmasFlags flags, char const *name)
        : Base(),
          base_(base),
          size_(size),
          flags_(flags),
          arch_(base, size, flags),
          users_(),
          refcnt_(1),
          root_vma_()
    {}

    VmAspace::~VmAspace()
    {}

    auto VmAspace::create(VmasFlags flags, char const *name, ustl::Rc<VmAspace> *out) 
        -> Status {
        VirtAddr base;
        usize size;
        if (bool(flags & VmasFlags::User)) {
            base = USER_ASPACE_BASE;
            size = USER_ASPACE_SIZE;
        }

        return Self::create(base, size, flags, name, out);
    }

    auto VmAspace::create(VirtAddr base, usize size, VmasFlags flags, char const *name, 
                          ustl::Rc<VmAspace> *out) -> Status 
    {
        auto aspace = new (*s_aspace_cache, kGafKernel) VmAspace(base, size, flags, name);
        if (!aspace) {
            return Status::OutOfMem;
        }

        // In the normal case, the root VMA always ownes RWX permissions so that the sub-VMA derived from
        // it can be created with arbitrary permissions combination.
        CXX11_CONSTEXPR
        static auto const kRootVmaFlags = VmaFlags::PermMask | VmaFlags::Share;
        aspace->init();
        {
            // Entrollment into the global list.
            ustl::sync::LockGuard guard(Self::all_aspace_list_mutex_);
            Self::all_aspace_list_.push_back(*aspace);
        }
        *out = ustl::make_rc<VmAspace>(aspace);
        return Status::Ok;
    }

    INIT_CODE
    auto VmAspace::init_kernel_aspace() -> void {
        CXX11_CONSTEXPR 
        auto const flags = VmasFlags::Kernel;

        auto caches = ObjectCache::create<VmAspace>("aspace-cache", OcFlags::Folio);
        if (!caches) {
            panic("Failed to create object cache for VmAspace");
        }
        s_aspace_cache = caches;

        ustl::Rc<Self> aspace;
        auto status = Self::create(KERNEL_ASPACE_BASE, KERNEL_ASPACE_SIZE, flags, "K:Aspace", &aspace);
        ASSERT(Status::Ok == status, "Failed to create kernel VmAspace");

        Self::kernel_aspace_ = aspace.take();
    }

    auto VmAspace::sync_kernel_aspace() -> void {
    }

    auto VmAspace::switch_aspace(Self *prev, Self *next) -> void {
        ArchVmAspace::switch_context(&prev->arch_, &next->arch_);
    }

    auto VmAspace::clone(VmasFlags flags, ustl::Rc<VmAspace> *out) -> Status {
        return Status::Unimplemented;
    }

    auto VmAspace::init() -> Status {
        auto status = this->arch_.init();
        if (status != Status::Ok) {
            return status;
        }

        status = VmArea::create_root(base_, size_, VmaFlags::PermMask | VmaFlags::Share, this, "RootVma", &root_vma_);
        if (status != Status::Ok) {
            return Status::OutOfMem;
        }

        return Status::Ok;
    }

    /// Because of cyclic dependency between VmAspace and VmArea, there is not yet
    /// a better way to inline it.
    auto VmAspace::root_vma() -> ustl::Rc<VmArea> {
        return root_vma_;
    }

    auto VmAspace::fault(VirtAddr virt_addr, VmfCause cause) -> void {
        if (!fault_cache_) [[likely]] {
            // if (fault_cache_->contains(virt_addr)) {
            // }
        } else {
            // if (auto fault = root_vma_.find_subvma(virt_addr)) {
            //     fault_cache_ = ustl::move(fault);
            // }
        }
        if (fault_cache_) {
            VmFault vmf;
            fault_cache_->fault(&vmf);
        }
    }
}