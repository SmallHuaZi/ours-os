#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/scope.hpp>
#include <ours/arch/aspace_layout.hpp>
#include <ours/mem/object-cache.hpp>

#include <ustl/lazy_init.hpp>
#include <ustl/sync/lockguard.hpp>

#include <logz4/log.hpp>
#include <heap/scope.hpp>

namespace ours::mem {
    /// Manage the lifetime manually.
    /// The global-unique address sapce.
    static ustl::Rc<ObjectCache>        s_aspace_cache;

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

    auto VmAspace::create(VmasFlags flags, char const *name) 
        -> ustl::Rc<VmAspace>
    {
        VirtAddr base;
        usize size;
        if (bool(flags & VmasFlags::User)) {
            base = USER_ASPACE_BASE;
            size = USER_ASPACE_SIZE;
        }

        return Self::create(base, size, flags, name);
    }

    auto VmAspace::create(VirtAddr base, usize size, VmasFlags flags, char const *name) 
        -> ustl::Rc<VmAspace>
    {
        auto aspace = s_aspace_cache->allocate<Self>(kGafKernel, base, size, flags, name);
        if (!aspace) {
            return nullptr;
        }

        aspace->init();
        {
            // Entrollment into the global list.
            ustl::sync::LockGuard guard(Self::all_aspace_list_mutex_);
            Self::all_aspace_list_.push_back(*aspace);
        }
        return ustl::make_rc<VmAspace>(aspace);
    }

    INIT_CODE
    auto VmAspace::init_kernel_aspace() -> void {
        CXX11_CONSTEXPR 
        auto const flags = VmasFlags::Kernel;

        auto caches = ObjectCache::create<VmAspace>("aspace-cache", OcFlags::Folio);
        if (!caches) {
            panic("Failed to create object cache for VmAspace");
        }

        auto kaspace = Self::create(KERNEL_ASPACE_BASE, KERNEL_ASPACE_SIZE, flags, "K:Aspace");
        if (!kaspace) {
            panic("Failed to create kernel VmAspace");
        }

        Self::kernel_aspace_ = kaspace.take();
    }

    auto VmAspace::sync_kernel_aspace() -> void {
    }

    auto VmAspace::switch_aspace(Self *, Self *) -> void {
    }

    auto VmAspace::clone(VmasFlags flags) -> ustl::Rc<VmAspace> {
        return nullptr;
    }

    auto VmAspace::init() -> Status {
        auto status = this->arch_.init();
        if (status != Status::Ok) {
            return status;
        }
        root_vma_ = VmArea::create(this, base_, size_, {}, {}, "RootVma");

        return Status::Ok;
    }

    auto VmAspace::fault(VirtAddr virt_addr, VmfCause cause) -> void {
        if (!fault_cache_) [[likely]] {
            if (fault_cache_->contains(virt_addr)) {
            }
        } else {
            if (auto fault = root_vma_.find_subvma(virt_addr)) {
                fault_cache_ = ustl::move(fault);
            }
        }
        if (fault_cache_) {
            VmFault vmf;
            fault_cache_->fault(&vmf);
        }
    }
}