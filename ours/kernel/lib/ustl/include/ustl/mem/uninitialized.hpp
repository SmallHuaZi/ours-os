#ifndef USTL_MEM_UNINITIALIZED_H
#define USTL_MEM_UNINITIALIZED_H 1

namespace ustl::mem {

    template <typename InputIterator>
    auto uninitialized_copy(InputIterator first, InputIterator last) -> void;
    
    template <typename ExecutionPolicy, typename InputIterator>
    auto uninitialized_copy(ExecutionPolicy policy, InputIterator first, InputIterator last) -> void;
    
    template <typename InputIterator, typename SizeType>
    auto uninitialized_copy_n(InputIterator i, SizeType n) -> void;
    
    template <typename ExecutionPolicy, typename InputIterator, typename SizeType>
    auto uninitialized_copy_n(ExecutionPolicy policy, InputIterator i, SizeType n) -> void;
    
    template <typename InputIterator>
    auto uninitialized_move(InputIterator first, InputIterator last) -> void;
    
    template <typename ExecutionPolicy, typename InputIterator>
    auto uninitialized_move(ExecutionPolicy policy, InputIterator first, InputIterator last) -> void; 
    
    template <typename InputIterator, typename SizeType>
    auto uninitialized_move_n(InputIterator i, SizeType n) -> void;
    
    template <typename ExecutionPolicy, typename InputIterator, typename SizeType>
    auto uninitialized_move_n(ExecutionPolicy policy, InputIterator i, SizeType n) -> void;
    
    template <typename InputIterator>
    auto uninitialized_fill(InputIterator first, InputIterator last) -> void;
    
    template <typename ExecutionPolicy, typename InputIterator>
    auto uninitialized_fill(ExecutionPolicy policy, InputIterator first, InputIterator last) -> void;
    
    template <typename InputIterator, typename SizeType>
    auto uninitialized_fill_n(InputIterator i, SizeType n) -> void;
    
    template <typename ExecutionPolicy, typename InputIterator, typename SizeType>
    auto uninitialized_fill_n(ExecutionPolicy policy, InputIterator i, SizeType n) -> void;
    
    template <typename Iterator>
    auto uninitialized_value_construct(Iterator first, Iterator last) -> void;
    
    template <typename ExecutionPolicy, typename Iterator>
    auto uninitialized_value_construct(ExecutionPolicy policy, Iterator first, Iterator last) -> void;
    
    template <typename Iterator, typename SizeType>
    auto uninitialized_value_construct_n(Iterator i, SizeType n) -> void;
    
    template <typename ExecutionPolicy, typename Iterator, typename SizeType>
    auto uninitialized_value_construct_n(ExecutionPolicy policy, Iterator i, SizeType n) -> void;
    
    template <typename Iterator>
    auto uninitialized_default_construct(Iterator first, Iterator last) -> void;
    
    template <typename ExecutionPolicy, typename Iterator>
    auto uninitialized_default_construct(ExecutionPolicy policy, Iterator first, Iterator last) -> void;
    
    template <typename Iterator, typename SizeType>
    auto uninitialized_default_construct_n(Iterator i, SizeType n) -> void;
    
    template <typename ExecutionPolicy, typename Iterator, typename SizeType>
    auto uninitialized_default_construct_n(ExecutionPolicy policy, Iterator i, SizeType n) -> void;
    
} // namespace ustl::mem

#endif // #ifndef USTL_MEM_UNINITIALIZED_H