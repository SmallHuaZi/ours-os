#ifndef USTL_UTIL_TUPLE_H
#define USTL_UTIL_TUPLE_H 1

#include <ustl/util/move.hpp>
#include <ustl/util/types_list.hpp>

namespace ustl {
namespace details {
    template <unsigned int I, typename T>
    struct TupleDataNode
    {
        typedef T           ValueType;
        typedef T const *   Ptr;
        typedef T *         PtrMut;
        typedef T const &   Ref;
        typedef T &         RefMut;

    private:
        typedef TupleDataNode    Self;

    public:
        USTL_FORCEINLINE USTL_CONSTEXPR static RefMut _S_value(Self &self) USTL_NOEXCEPT 
        { return self._value; }

        USTL_FORCEINLINE USTL_CONSTEXPR static Ref _S_value(Self const &self) USTL_NOEXCEPT
        { return self._value; }

        USTL_FORCEINLINE USTL_CONSTEXPR void _M_copy(Self const &other)
        { this->_value = other._value; }

        USTL_FORCEINLINE USTL_CONSTEXPR void _M_move(Self &&other)
        { this->_value = ustl::move(other._value); }

        USTL_FORCEINLINE USTL_CONSTEXPR void _M_swap(Self &other) 
        {
            ValueType tmp(other._value);
            other._value = this->_value;
            this->_value = tmp;
        }

    protected:
        USTL_FORCEINLINE USTL_CONSTEXPR TupleDataNode() USTL_NOEXCEPT = default;

        USTL_FORCEINLINE USTL_CONSTEXPR TupleDataNode(Ref e)
            : _value(e) 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR TupleDataNode(ValueType &&e)
            : _value(forward(e)) 
        {}

    protected:
        ValueType _value;
    };


    template <unsigned int I, typename T, typename... Ts>
    struct TupleImpl
        : public TupleImpl<I+ 1, Ts...>,
          public TupleDataNode<I, T>
    {
        typedef T           ValueType;
        typedef T const *   Ptr;
        typedef T *         PtrMut;
        typedef T const &   Ref;
        typedef T &         RefMut;

    private:
        typedef TupleImpl   Self;
        typedef TupleDataNode<I, T> DataNode;
        typedef TupleImpl<I + 1, Ts...> Base;

    public:
        USTL_FORCEINLINE USTL_CONSTEXPR static RefMut _S_value(DataNode &self) USTL_NOEXCEPT
        { return DataNode::_S_value(self); }

        USTL_FORCEINLINE USTL_CONSTEXPR static Ref _S_value(DataNode const &self) USTL_NOEXCEPT
        { return DataNode::_S_value(self); }

        USTL_FORCEINLINE USTL_CONSTEXPR static Base &_S_to_base(Self &self) USTL_NOEXCEPT
        { return self; }

        USTL_FORCEINLINE USTL_CONSTEXPR static Base const &_S_to_base(Self const &self) USTL_NOEXCEPT
        { return self; }

        USTL_FORCEINLINE USTL_CONSTEXPR void _M_copy(Self const &other)
        {
            _S_value(*this) = _S_value(other);
            Base::_M_copy(_S_to_base(other));
        }

        USTL_FORCEINLINE USTL_CONSTEXPR void _M_move(Self &&other)
        {
            _S_value(*this) = ustl::move(_S_value(other));
            Base::_M_move(ustl::move(_S_to_base(other)));
        }

        USTL_FORCEINLINE USTL_CONSTEXPR void _M_swap(Self &self)
        {
            DataNode::_M_swap(self);
            Base::_M_swap(_S_to_base(self));
        }

    public:
        USTL_FORCEINLINE USTL_CONSTEXPR TupleImpl() USTL_NOEXCEPT
            : Base(), DataNode() 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR TupleImpl(Ref h, Ts const &...t)
            : Base(t...), DataNode(h) 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR TupleImpl(ValueType &&h, Ts &&...t)
            : Base(forward(t)...), 
              DataNode(forward(h)) 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR TupleImpl(Self const &other)
        { _M_copy(other); }

        USTL_FORCEINLINE USTL_CONSTEXPR TupleImpl(Self &&other)
        { _M_move(other); }
    };


    template <unsigned int I, typename T>
    struct TupleImpl<I, T>
        : public TupleDataNode<I, T>
    {
        typedef T           ValueType;
        typedef T const *   Ptr;
        typedef T *         PtrMut;
        typedef T const &   Ref;
        typedef T &         RefMut;

    private:
        typedef TupleImpl   Self;
        typedef TupleDataNode<I, T>    DataNode;

    public:
        USTL_FORCEINLINE USTL_CONSTEXPR static Ref 
        _S_value(DataNode const &self) USTL_NOEXCEPT
        { return DataNode::_S_value(self); }

        USTL_FORCEINLINE USTL_CONSTEXPR static 
        RefMut _S_value(DataNode &self) USTL_NOEXCEPT
        { return DataNode::_S_value(self); }

        USTL_FORCEINLINE USTL_CONSTEXPR void 
        _M_copy(Self const &other)
        { _S_value(*this) = _S_value(other); }

        USTL_FORCEINLINE USTL_CONSTEXPR void 
        _M_move(Self &&other)
        { _S_value(*this) = ustl::move(_S_value(other)); }

        USTL_FORCEINLINE USTL_CONSTEXPR void 
        _M_swap(Self &other)
        { DataNode::_M_swap(other); }

    public:
        USTL_FORCEINLINE USTL_CONSTEXPR 
        TupleImpl() USTL_NOEXCEPT
            : DataNode() 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        TupleImpl(Ref h)
            : DataNode(h) 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        TupleImpl(ValueType &&h) 
            : DataNode(forward(h)) 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        TupleImpl(Self const &other)
        {  this->_M_copy(other);  }

        USTL_FORCEINLINE USTL_CONSTEXPR 
        TupleImpl(Self &&other)
        {  this->_M_move(other);  }
    };
} // namespace ustl::details

    template <typename... Ts>
    class Tuple 
        : public details::TupleImpl<0, Ts...> 
    {
    private:
        USTL_GENERAL_ASSOCIATION_TYPE(Tuple);

        typedef Tuple Self;
        typedef details::TupleImpl<0, Ts...>    Base;

    public:
        USTL_FORCEINLINE USTL_CONSTEXPR 
        Tuple() USTL_NOEXCEPT
            : Base() 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        Tuple(Ts const &...args)
            : Base(args...) 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        Tuple(Ts &&...args)
            : Base(forward(args)...) 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        Tuple(Self const &other)
            : Base(other) 
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        Tuple(Self &&other)
            : Base(forward(other))
        {}

    public:
        USTL_FORCEINLINE USTL_CONSTEXPR Self &
        operator=(Ref t)
        {
            Base::_M_copy(t);
            return *this;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR Self &
        operator=(Tuple &&t)
        {
            Base::_M_move(forward<Tuple>(t));
            return *this;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR void 
        swap(Self &t)
        {  Base::_M_swap(t);  }

        USTL_FORCEINLINE USTL_CONSTEXPR usize 
        size() USTL_NOEXCEPT
        {  return TypeList<Ts...>::size();  }
    };


    template <usize I, typename... Ts>
    USTL_FORCEINLINE USTL_CONSTEXPR
    TypeListAtT<I, TypeList<Ts...>> &
    get(Tuple<Ts...> &t) USTL_NOEXCEPT
    { 
        typedef TypeListAtT<I, TypeList<Ts...>> ValueType;
        return details::TupleImpl<I, ValueType>::_S_value(t); 
    }

    template <usize I, typename... Ts>
    USTL_FORCEINLINE USTL_CONSTEXPR
    TypeListAtT<I, TypeList<Ts...>> const &
    get(Tuple<Ts...> const &t) USTL_NOEXCEPT
    { 
        typedef TypeListAtT<I, TypeList<Ts...>> ValueType;
        return details::TupleImpl<I, ValueType>::_S_value(t); 
    }


    template <unsigned int I, typename... Ts>
    struct TupleEqualImpl
    {
        typedef Tuple<Ts...>     TupleType;

        USTL_FORCEINLINE USTL_CONSTEXPR
        bool operator()(TupleType const &x, TupleType const &y)
        {
            return get<I>(x) == get<I>(y) && TupleEqualImpl<I - 1, Ts...>{}(x, y);
        }
    };

    template <typename... Ts>
    struct TupleEqualImpl<0, Ts...>
    {
        typedef Tuple<Ts...>     TupleType;

        USTL_FORCEINLINE USTL_CONSTEXPR
        bool operator()(TupleType const &x, TupleType const &y)
        {
            return get<0>(x) == get<0>(y);
        }
    };


    template <typename... Ts>
    USTL_FORCEINLINE USTL_CONSTEXPR bool operator==(Tuple<Ts...> const &x, Tuple<Ts...> const &y)
    {
        USTL_CONSTEXPR long const LENGTH = TypeList<Ts...>::size();
        static_assert(LENGTH != 0, "[ustl-error] Empty tuple cause undefined behavior.\n");

        return TupleEqualImpl<LENGTH - 1, Ts...>{}(x, y);
    }

    template <typename... Ts>
    USTL_FORCEINLINE USTL_CONSTEXPR bool operator!=(Tuple<Ts...> const &x, Tuple<Ts...> const &y)
    {  return !(x == y);  }

    template <typename... Ts>
    USTL_FORCEINLINE USTL_CONSTEXPR Tuple<Ts...> make_tuple(Ts&&... elems)
    {  return Tuple<Ts...> {forward<Ts>(elems)...};  }

    template <typename F, typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR void apply(F &&f, T &&t) 
    {

    }

} // namespace ustl


#endif // #ifndef USTL_UTIL_TUPLE_H