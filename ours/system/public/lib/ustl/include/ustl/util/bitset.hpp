#ifndef USTL_UTIL_BITSET_H
#define USTL_UTIL_BITSET_H 1

namespace ustl {

    template <int N>
    class BitSetBasic
        : protected BitSetOps
    {
    protected:
        struct BitRef;

        typedef usize   size_type;
        typedef usize   word_type;

    protected:
        USTL_FORCEINLINE USTL_CONSTEXPR
        size_type
        _M_bits_size() const USTL_NOEXCEPT
        { return sizeof(_M_map) * BYTE_BITS; }

        USTL_FORCEINLINE USTL_CONSTEXPR
        size_type
        _M_byte_size() const USTL_NOEXCEPT
        { return sizeof(_M_map); }

        USTL_FORCEINLINE USTL_CONSTEXPR
        size_type
        _M_word_size() const USTL_NOEXCEPT
        { return sizeof(_M_map) / sizeof(_M_map[0]); }

        USTL_FORCEINLINE USTL_CONSTEXPR
        void
        _M_do_set_all(word_type val) USTL_NOEXCEPT
        {
            size_type const limit = _M_word_size();
            for (size_type i = 0; i < limit; i++) {
                this->_M_map[i] = val;
            }
        }

    protected:
        word_type   _M_map[N];
    };


    template <int N>
    struct BitSetBasic<N>::BitRef
    {
        USTL_FORCEINLINE USTL_CONSTEXPR
        size_type
        get_len() USTL_NOEXCEPT
        {
            return N;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        void
        increase() USTL_NOEXCEPT
        {
            ++_M_pos;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        void
        decrease() USTL_NOEXCEPT
        {
            --_M_pos;
        }

    private:
        size_type  _M_pos;
        word_type *_M_ptr;
    };

    template <usize NBits, usize NWords = align_up(NBits, WORD_BITS) / WORD_BITS>
    class BitSet
        : protected BitSetBasic<NWords>
    {
        USTL_GENERAL_ASSOCIATION_TYPE_HAD_BASE(BitSet, BitSetBasic<NWords>);

    public:
        typedef  typename Base::size_type       size_type;
        typedef  typename Base::word_type       word_type;
        typedef  typename Base::BitRef          value_type;
        typedef  typename Base::BitRef *        pointer;
        typedef  typename Base::BitRef &        reference;
        typedef  typename Base::BitRef const *  const_pointer;
        typedef  typename Base::BitRef const &  const_reference;
        typedef  typename iter::SliceIter<true, value_type>   iter_mut;
        typedef  typename iter::SliceIter<false, value_type>  iter_const;

    private:
        using  Base::_M_map;
        using  Base::_M_bits_size;
        using  Base::_M_byte_size;
        using  Base::_M_word_size;
        using  Base::_M_do_set_all;

    public:
        BitSet() = default;

        BitSet(word_type (&bits)[NWords]);

        template <typename Iterator>
        BitSet(Iterator first, Iterator last);

        USTL_FORCEINLINE USTL_CONSTEXPR
        void set(size_type const pos) USTL_NOEXCEPT
        { 
            Base::set(_M_map, pos); 
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        void set(size_type const pos, bool bit) USTL_NOEXCEPT
        { 
            if (bit) {
                return Self::set(pos);
            }

            Self::clr(pos);
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        void set_range(size_type const first, size_type const last) USTL_NOEXCEPT
        { 
            Base::set_range(_M_map, first, last); 
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        void set_all() USTL_NOEXCEPT
        { 
            _M_do_set_all(USIZE_MAX); 
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        size_type set_first() USTL_NOEXCEPT
        { 
            return Base::set_first(_M_map, _M_word_size()); 
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        void clr(size_type const pos) USTL_NOEXCEPT
        { 
            Base::clr(_M_map, pos); 
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        void clr_range(size_type const first, size_type const last) USTL_NOEXCEPT
        { 
            Base::clr_range(this->_M_map, first, last); 
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        void clr_all() USTL_NOEXCEPT
        { 
            _M_do_set_all(0); 
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        size_type clr_first() USTL_NOEXCEPT
        { 
            return Base::clr_first(_M_map, _M_word_size()); 
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        void  flip_all() USTL_NOEXCEPT
        { 
            Base::bit_neg(_M_map, _M_map + _M_word_size()); 
        }

        /// @brief 
        /// @param pos 
        USTL_FORCEINLINE USTL_CONSTEXPR
        void
        flip(size_type const pos) USTL_NOEXCEPT
        { 
            Base::flip(_M_map, pos); 
        }

        /// @brief 
        /// @param first 
        /// @param last 
        USTL_FORCEINLINE USTL_CONSTEXPR
        void
        flip_range(size_type const first, size_type const last) USTL_NOEXCEPT
        { 
            Base::flip_range(_M_map, first, last); 
        }

        /// @brief 
        /// @param bits 
        void
        assign(word_type (&bits)[NWords]) USTL_NOEXCEPT
        {
            for (size_type i = 0; i < NWords; ++i) {
                _M_map[i] = bits[i];
            }
        }

        /// @brief
        template <typename Iterator>
        size_type
        assign(Iterator first, Iterator last) USTL_NOEXCEPT
        {
            using namespace iter;
            using namespace traits;
            static_assert(IsConvertible<typename IterTraits<Iterator>::value_type, bool>::value, 
                          "");
            
            for (size_type i = 0; first != last; ++i, (void)++first) {
                Self::set(i, *first);
            }
        }

        /// @brief 
        /// @param pos
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        bool
        test(size_type const pos) const USTL_NOEXCEPT
        { 
            return Base::test(_M_map, pos); 
        }

        /// @brief 
        /// @param first
        /// @param last 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        bool
        test_range(size_type const first, size_type const last) const USTL_NOEXCEPT
        { 
            return Base::test_range(_M_map, first, last); 
        }

        /// @brief 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        bool
        any() const USTL_NOEXCEPT
        { 
            return Base::any(_M_map, _M_word_size()); 
        }

        /// @brief 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        bool
        all() const USTL_NOEXCEPT
        { 
            return Base::all(_M_map, _M_word_size()); 
        }

        /// @brief 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        bool
        none() const USTL_NOEXCEPT
        { 
            return Base::none(_M_map, _M_word_size()); 
        }

        /// @brief 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        size_type
        find_first() const USTL_NOEXCEPT
        { 
            return Base::find_first(_M_map, _M_word_size()); 
        }

        /// @brief 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        size_type
        find_next(size_type const prev) const USTL_NOEXCEPT
        { 
            return Base::find_next(_M_map, prev, _M_word_size()); 
        }

        /// @brief 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        size_type
        find_last() const USTL_NOEXCEPT
        { 
            return Base::find_last(_M_map, _M_word_size()); 
        }

        /// @brief 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        size_type
        size() const USTL_NOEXCEPT
        { 
            return _M_bits_size(); 
        }

        /// @brief 
        /// @param other 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Ref
        operator=(Self const &other) & USTL_NOEXCEPT
        {
            using algo::copy;
            copy(_M_map, _M_map + _M_word_size(), other._M_map);

            return *this;
        }

        /// @brief 
        /// @param other 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Ref
        operator&=(ConstRef other) & USTL_NOEXCEPT
        {
            Base::bit_and(_M_map, _M_map + _M_word_size(), other._M_map);
            return *this;
        }

        /// @brief 
        /// @param other 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Ref
        operator|=(ConstRef other) & USTL_NOEXCEPT
        {
            Base::bit_or(_M_map, _M_map + _M_word_size(), other._M_map);
            return *this;
        }

        /// @brief 
        /// @param other 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Ref
        operator^=(ConstRef other) & USTL_NOEXCEPT
        {
            Base::bit_xor(_M_map, _M_map + _M_word_size(), other._M_map);
            return *this;
        }

        /// @brief 
        /// @param n 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Ref 
        operator>>=(size_type const n) & USTL_NOEXCEPT
        {
            Base::right_shift(_M_map, n, _M_word_size());
            return *this;
        }

        /// @brief 
        /// @param n 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Ref
        operator<<=(size_type const n) & USTL_NOEXCEPT
        {
            Base::left_shift(_M_map, n, _M_word_size());
            return *this;
        }

        /// @brief 
        /// @param other 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Self
        operator&(ConstRef other) const USTL_NOEXCEPT
        {
            Self res {*this};
            res &= other;
            return res;
        }

        /// @brief 
        /// @param other 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Self
        operator|(ConstRef other) const USTL_NOEXCEPT
        {
            Self res {*this};
            res |= other;
            return res;
        }

        /// @brief
        /// @param other 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Self
        operator^(ConstRef other) const USTL_NOEXCEPT
        {
            Self res {*this};
            res ^= other;
            return res;
        }

        /// @brief
        /// @param n
        /// @return
        USTL_FORCEINLINE USTL_CONSTEXPR
        Self
        operator>>(size_type const n) const USTL_NOEXCEPT
        {
            Self res {*this};
            res >>= n;
            return res;
        }

        /// @brief 
        /// @param n 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Self
        operator<<(size_type const n) const USTL_NOEXCEPT
        {
            Self res {*this};
            res <<= n;
            return res;
        }

        /// @brief 
        /// @return 
        USTL_FORCEINLINE USTL_CONSTEXPR
        Self
        operator!() const USTL_NOEXCEPT
        {
            Self res {*this};
            Base::bit_neg(res._M_map, res._M_map + res._M_word_size());
            return res;
        }

        friend bool
        operator== <>(ConstRef x, ConstRef y);

        friend bool
        operator!= <>(ConstRef x, ConstRef y);

        friend bool
        operator< <>(ConstRef x, ConstRef y);

        friend bool
        operator>= <>(ConstRef x, ConstRef y);
    };

    template <usize NBits, usize NWords>
    USTL_CONSTEXPR
    static bool
    operator==(BitSet<NBits, NWords> const &x, BitSet<NBits, NWords> const &y) USTL_NOEXCEPT
    {
        int i = 0, j = x._M_word_size();
        for (; i < j && x._M_map[i] == y._M_map[i]; ++i);
        
        return i == j; 
    }

    template <usize NBits, usize NWords>
    USTL_CONSTEXPR
    static bool
    operator!=(BitSet<NBits, NWords> const &x, BitSet<NBits, NWords> const &y) USTL_NOEXCEPT
    {
        return !(x == y); 
    }
        
    template <usize NBits, usize NWords>
    USTL_CONSTEXPR
    static bool
    operator<(BitSet<NBits, NWords> const &x, BitSet<NBits, NWords> const &y) USTL_NOEXCEPT
    {
        int i = 0, j = x._M_word_size();
        for (; i < j && x._M_map[i] <= y._M_map[i]; ++i);

        return i == j;
    }

    template <usize NBits, usize NWords>
    USTL_CONSTEXPR
    static bool
    operator>=(BitSet<NBits, NWords> const &x, BitSet<NBits, NWords> const &y) USTL_NOEXCEPT
    {
        return !(x < y);
    }

} // namespace ustl

#endif // #ifndef USTL_UTIL_BITSET_H