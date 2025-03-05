#ifndef USTL_UTIL_EBO_OPTIMIZER_H
#define USTL_UTIL_EBO_OPTIMIZER_H

#include <ustl/config.hpp>

namespace ustl {

    template <typename T, bool = 0 == sizeof(T)>
    struct EboOptimizer;

    template <typename T>
    struct EboOptimizer<T, true>
        : public T
    {
        USTL_GENERAL_ASSOCIATION_TYPE_WITH_BASE(T, T);
    
    public:
        EboOptimizer() = default;

        EboOptimizer(Ref o)
            : Base(o)
        {}

        PtrMut as_ptr()
        {  return static_cast<PtrMut>(this);  }

        Ptr as_ptr() const
        {  return static_cast<Ptr>(this);  }

        RefMut as_ref()
        {  return *this;  }

        Ref as_ref() const
        {  return *this;  }
    };

    template <typename T>
    struct EboOptimizer<T, false>
    {
        USTL_GENERAL_ASSOCIATION_TYPE(T);

    public:
        EboOptimizer() = default;

        EboOptimizer(Ref o)
            : _data(o)
        {}

        PtrMut as_ptr()
        {  return static_cast<PtrMut>(&this->_data);  }

        Ptr as_ptr() const
        {  return static_cast<Ptr>(&this->_data);  }

        RefMut as_ref()
        {  return this->_data;  }

        Ref as_ref() const
        {  return this->_data;  }

    private:
        ValueType  _data;
    };

} // namespace ustl


#endif // #ifndef USTL_UTIL_EBO_OPTIMIZER_H