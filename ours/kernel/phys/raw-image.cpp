#include <ours/phys/kernel-image.hpp>

namespace ours::phys {
    auto RawImage::init(ustl::views::Span<u8> file) -> Status {
        file_ = file;
        header_ = *reinterpret_cast<omitl::ObiRawExeHeader *>(file_.data());
        return Status::Ok;
    } 

    auto RawImage::load_at(PhysAddr phys_addr) -> ustl::Result<PhysAddr, Status> {
        auto const pkgsize = file_.size();
        auto const pkgaddr = file_.data();
        auto x = *reinterpret_cast<omitl::ObiRawExeHeader *>(pkgaddr);

        ustl::algorithms::copy_n(pkgaddr, pkgsize, reinterpret_cast<u8 *>(phys_addr));
        return ustl::ok(header_.entry_point);
    }
} // namespace ours::phys