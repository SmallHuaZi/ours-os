#include <ours/phys/kernel-package.hpp>
#include <ours/phys/print.hpp>
#include <ours/assert.hpp>
#include <ours/phys/init.hpp>
#include <ustl/mem/object.hpp>

namespace ours::phys {
    auto KernelPackage::init(Obi const &obi) -> Status {
        for (auto i = obi.begin(); i != obi.end(); ++i) {
            if (i->header->type == OMIT_KPACKAGE) {
                package_ = i;
            }
        }

        return unpack();
    }

    auto KernelPackage::unpack() -> Status {
        auto const mem = global_bootmem();
        auto const storage_size = omitl::unzipped_size(*package_->header);
        auto const output = mem->allocate<u8>(storage_size, PAGE_SIZE);
        if (!output) {
            return Status::OutOfMem;
        }

        package_load_start_ = arch::Tick::get();

        ustl::mem::construct_at(&package_loaded_, output, storage_size);
        // BUG(SmallHuaZi) Third argument is a placeholer now, but in near future we should
        // replace it to a real memory allocator.
        auto result = obi_.copy_item(package_loaded_, package_, {});
        println("Kernel package was loaded at {}", static_cast<void *>(package_loaded_.data()));
        if (result.is_err()) {
            panic("Fail to unzip main module.");
        }

        package_load_end_ = arch::Tick::get();
        return Status::Ok;
    }

} // namespace ours::phys