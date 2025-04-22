#include <ours/mem/vm_object_paged.hpp>

#include <logz4/log.hpp>

#include <ustl/mem/align.hpp>

using ustl::mem::align_up;
using ustl::mem::align_down;

namespace ours::mem {
    FORCE_INLINE
    auto VmObjectPaged::commit_range_internal(PgOff pgoff, usize n, CommitOptions option) -> Status {
        if (!!(option & CommitOptions::Pin)) {
            if (!n) {
                return Status::InvalidArguments;
            }
        }

        if (!n) {
            return Status::Ok;
        } 
        
        ustl::sync::LockGuard guard(mutex_);
        if (cow_pages_.num_pages_locked() < n) {
            return Status::OutOfRange;
        }

        while (n > 0) {
            usize nr_commited = 0;
            auto status = cow_pages_.commit_range_locked(pgoff, n, &nr_commited);

            if (status != Status::Ok && status != Status::ShouldWait) {
                return status;
            }
        }

        return Status::Ok;
    }

    auto VmObjectPaged::commit_range(PgOff pgoff, usize n, CommitOptions option) -> Status {
        canary_.verify();
        log::trace("VMO Commit Action: [pgoff: {:X}, n: {:X}]", pgoff, n);
        return commit_range_internal(pgoff, n, option);
    }

} // namespace ours::mem