# Workaround for the core library __ustl__

> **State**: Active.

> **Date**: 2025/3/11.

> **Owner**: SmallHuaZi

## 1. Issue Description 
Ours foundational library __ustl__(production edition) is currently also under active development. The version used in the project primarily serves as an interface layer library to standard library and Boost components with only a limited subset of features being original implementations. This transitional architecture imposes significant constraints that demand particular attention when utilize its components.

## 2. Constraints on some components 
### 2.1.1 The functional object
1. The most edition of implementations to std::function aliased __ustl::function::Fn__, including libcxx, msvc-stl and libstdcxx is all to wrapper the function pointer, member method pointer and the possible fixed-size lambda object into an inplacing storage. And over the size will trigger dynamic memory allocations. From this, the size of lambda object was limited by **sizeof(ustl::function::Fn<void>)** or under.

### 2.1.2 The collection objects excluding intrusives.
1. The non-intrusive collections generally manage the lifetime of inside objects, actually to the memory area a object placed. This result in that them is not available until the VMM is initialized and the allocator should be overrided to __ktl::Allocator__ in the meantime.

2. The standard library and Boost for supporting exception mechanism was embeed so many try-catch statements, but those all should be inactive and disabled in kernel development. Very unfortunately, there is no way to disable them at all aspects. We have to use then carefully without triggering them.