#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_mapping.hpp>
#include <ours/mem/new.hpp>

#include <ours/kernel_layout.hpp>
#include <ours/cpu_local.hpp>

#include <ustl/lazy/lazy_init.hpp>

using ustl::lazy::LazyInit;

namespace ours::mem {
    /// Manage the lifetime manually.
    /// The global-unique address sapce.
    static LazyInit<VmAspace>       S_KERNEL_ASPACE;

    auto VmAspace::init_kernel_aspace() -> void
    {
        CXX11_CONSTEXPR auto const range = gktl::Range<VirtAddr> {/* VM_KERNEL_START, VM_KERNEL_END */};
        CXX11_CONSTEXPR auto const flags = VmAspaceFlags::Active | VmAspaceFlags::Kernel;

        // The tow following placement new is to call the constructor of VmAspace and VmRootArea.
        auto kernel_aspace = S_KERNEL_ASPACE.data();
        new (kernel_aspace) Self(range, flags, "Kernel Aspace", nullptr);
        kernel_aspace->init();

        for (decltype(auto) area: G_KERNEL_LAYOUT) {
            // kernel_aspace->protect_vma(area.name, { area.start_addr, area.end_addr });
        }

        Self::KERNEL_ASPACE_ = kernel_aspace;
    }

    auto VmAspace::sync_kernel_aspace() -> void
    {}

    auto VmAspace::clone(ustl::Rc<VmAspace>) -> ustl::Rc<VmAspace>
    {}

    auto VmAspace::create(gktl::Range<VirtAddr> range, VmAspaceFlags flags, char const *name, ustl::Rc<VmAspace> parent) 
        -> ustl::Rc<VmAspace>
    {
        if (auto aspace = new Self(range, flags, name, parent)) {
            return ustl::make_rc<VmAspace>(aspace);
        }
        return nullptr;
    }

    VmAspace::VmAspace(gktl::Range<VirtAddr> range, VmAspaceFlags flags, char const *name, ustl::Rc<VmAspace> parent)
        : flags_(flags),
          users_(),
          parent_(parent),
          refcnt_(1)
    {}

    VmAspace::~VmAspace()
    {}

    auto VmAspace::init() -> Status 
    {
        auto const arch_flags = 0;
        auto const res = this->arch_.init(range_, arch_flags);
        if (res != Status::Ok) {
            return res;
        }

        // Entrollment into the global list.
        Self::ALL_ASPACE_LIST_.push_back(*this);

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
            fault_cache_->handle_vm_fault(&vmf);
        }
    }
}