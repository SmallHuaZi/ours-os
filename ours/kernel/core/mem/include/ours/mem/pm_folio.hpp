#ifndef OURS_MEM_PM_FOLIO_HPP
#define OURS_MEM_PM_FOLIO_HPP 1

#include <ours/mem/pm_frame.hpp>

namespace ours::mem {
    class PmFolio
        : public PmFrame
    {

    private:
        usize order_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_FOLIO_HPP