#include <ours/mem/vm_cow_pages.hpp>

namespace ours::mem {
    auto VmCowPages::commit_range_locked(PgOff pgoff, usize n, ai_out usize *nr_commited) -> Status {
        return Status::Ok;
    }

} // namespace ours::mem