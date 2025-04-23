#include <ours/mem/vm_object.hpp>

namespace ours::mem {
    VmObject::VmObject(Type type, VmoFLags vmof)
        : canary_(),
          type_(type),
          vmof_(vmof),
          mappings_(),
          children_(),
          children_hook_()
    {}
}