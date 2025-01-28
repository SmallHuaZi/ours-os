#ifndef USTL_DETAIL_USTL_H
#define USTL_DETAIL_USTL_H 1

#define USTL_GENERAL_ASSOCIATION_TYPE(TYPE) \
    typedef TYPE    ValueType;\
    typedef TYPE *  PtrMut;\
    typedef TYPE &  RefMut;\
    typedef TYPE const &  Ref;\
    typedef TYPE const *  Ptr;

#define USTL_GENERAL_ASSOCIATION_TYPE_WITH_BASE(TYPE, BASE) \
    typedef BASE    Base;\
    USTL_GENERAL_ASSOCIATION_TYPE(TYPE)

#define USTL_NOT_COPYABLE(TYPE) \
    public:\
    TYPE& operator=(TYPE const &) = delete;\
    TYPE(TYPE const &) = delete;

#define USTL_NOT_MOVEABLE(TYPE) \
    public:\
    TYPE& operator=(TYPE &&) = delete;\
    TYPE(TYPE &&) = delete;

#define USTL_NOT_COPYABLE_AND_MOVEABLE(TYPE) \
    USTL_NOT_MOVEABLE(TYPE)\
    USTL_NOT_COPYABLE(TYPE)

#endif // USTL_DETAIL_USTL_H