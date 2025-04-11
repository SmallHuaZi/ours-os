#include <arch/processor.hpp>
#include <arch/x86/cpuid.hpp>

#include <ustl/traits/char_traits.hpp>

namespace arch {
    auto X86CpuInfo::init() -> void {
        CpuId request;
        request.query(CpuIdLeaf::Vendor);

        dispatch_cpuid(features_.make_cpuid_observer());

        union {
            u32 data[3];
            char str[];
        } vendor_str;
        vendor_str.data[0] = request.ebx();
        vendor_str.data[1] = request.edx();
        vendor_str.data[2] = request.ecx();

        typedef ustl::traits::CharTraits<char>  CharTraits;
        for (auto i = 0; i < std::size(kVendorString); ++i) {
            if (!CharTraits::compare(vendor_str.str, kVendorString[i], sizeof(vendor_str))) {
                vendor_ = static_cast<Vendor>(i);
            }
        }
    }

} // namespace arch