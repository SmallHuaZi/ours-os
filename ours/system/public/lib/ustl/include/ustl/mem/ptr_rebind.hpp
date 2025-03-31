#ifndef USTL_MEM_PTR_REBIND_H
#define USTL_MEM_PTR_REBIND_H 1

namespace ustl::mem {
namespace details {

    struct NormalTag    { char _[8]; };
    struct StdTag       { char _[4]; };
    struct StdLegacyTag { char _[2]; };
    struct NoneTag      { char _[1]; };
    
    enum class RebindMode {
        None        = sizeof(NoneTag),
        Normal      = sizeof(NormalTag),
        Std         = sizeof(StdTag),
        StdLegacy   = sizeof(StdLegacyTag),
    };
    
    template <typename Ptr, typename T>
    struct GetPtrRebindMode
    {
        template <typename U>
        static NormalTag test(int, typename U::template Rebind<T> *);
    
        template <typename U>
        static StdTag test(int, typename U::template rebind<T> *);
    
        template <typename U>
        static StdLegacyTag test(int, typename U::template rebind<T>::other *);
    
        template <typename U>
        static NoneTag test(int, void *);
    
        static auto const VALUE = RebindMode(sizeof(test<Ptr>(0, 0)));
    };
    
    template <typename PtrBox, typename T, RebindMode Mode = GetPtrRebindMode<PtrBox, T>::VALUE>
    struct PtrRebindImpl;
    
    template <typename PtrBox, typename T>
    struct PtrRebindImpl<PtrBox, T, RebindMode::None>
    {
        typedef T *     Type; 
    };
    
    template <typename PtrBox, typename T>
    struct PtrRebindImpl<PtrBox, T, RebindMode::Normal>
    {
        typedef typename PtrBox::template Rebind<T> Type;
    };
    
    template <typename PtrBox, typename T>
    struct PtrRebindImpl<PtrBox, T, RebindMode::Std>
    {
        typedef typename PtrBox::template rebind<T> Type;
    };
    
    template <typename PtrBox, typename T>
    struct PtrRebindImpl<PtrBox, T, RebindMode::StdLegacy>
    {
        typedef typename PtrBox::template rebind<T>::other Type;
    };
        
    } // namespace ustl::mem::details
    
    //! @tparam T
    //! @tparam PtrBox 
    template <typename PtrBox, typename T>
    struct PtrRebind
        : details::PtrRebindImpl<PtrBox, T>
    {};
    
} // namespace ustl::mem

#endif // #ifndef USTL_MEM_PTR_REBIND_H