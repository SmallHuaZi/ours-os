#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/scope.hpp>
#include <ours/arch/aspace_layout.hpp>

#include <ustl/lazy_init.hpp>
#include <ustl/sync/lockguard.hpp>

#include <heap/scope.hpp>

namespace ours::mem {
    /// Manage the lifetime manually.
    /// The global-unique address sapce.
    static ustl::LazyInit<VmAspace>       s_kernel_aspace;

    INIT_CODE
    auto VmAspace::init_kernel_aspace() -> void {
        CXX11_CONSTEXPR 
        auto const flags = VmasFlags::Kernel;

        // The tow following placement new is to call the constructor of VmAspace and VmRootArea.
        auto kernel_aspace = s_kernel_aspace.data();
        new (kernel_aspace) Self(KERNEL_ASPACE_BASE, KERNEL_ASPACE_SIZE, flags, "K:Aspace");
        kernel_aspace->init();

        Self::kernel_aspace_ = kernel_aspace;
    }

    auto VmAspace::sync_kernel_aspace() -> void
    {}

    auto VmAspace::switch_aspace(Self *, Self *) -> void
    {}

    auto VmAspace::clone(VmasFlags flags) -> ustl::Rc<VmAspace>
    {  return nullptr;  }

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
        auto aspace = new Self(base, size, flags, name);
        aspace->init();
        {
            // Entrollment into the global list.
            ustl::sync::LockGuard guard(Self::all_aspace_list_mutex_);
            Self::all_aspace_list_.push_back(*aspace);
        }
        return ustl::make_rc<VmAspace>(aspace);
    }

    VmAspace::VmAspace(VirtAddr base, usize size, VmasFlags flags, char const *name)
        : Base(),
          base_(base),
          size_(size),
          flags_(flags),
          arch_(base, size, flags),
          users_(),
          refcnt_(1)
    {}

    VmAspace::~VmAspace()
    {}

    auto VmAspace::init() -> Status 
    {
        auto status = this->arch_.init();
        if (status != Status::Ok) {
            return status;
        }

        return Status::Ok;
    }

    auto VmAspace::fault(VirtAddr virt_addr, VmfCause cause) -> void
    {
        // if (fault_cache_ != nullptr) [[likely]] {
        //     if (fault_cache_->contains(virt_addr)) {
        //     }
        // } else {
        //     if (auto fault = root_area_->search_subvma(virt_addr)) {
        //         fault_cache_ = fault.value();
        //     }
        // }
        if (fault_cache_) {
            VmFault vmf;
            fault_cache_->fault(&vmf);
        }
    }
}