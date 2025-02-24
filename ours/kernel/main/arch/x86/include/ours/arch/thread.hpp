#ifndef OURS_ARCH_X86_THREAD_HPP
#define OURS_ARCH_X86_THREAD_HPP 1

namespace ours::task {
    class ArchThread
    {
        typedef ArchThread  Self;

    public:
        static auto switch_context(Self &prev, Self &next) -> void;
    
    private:
        friend class Thread;
    };

} // namespace ours::task

#endif // #ifndef OURS_ARCH_X86_THREAD_HPP