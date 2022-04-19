#ifndef REIO_ALLOCATORS_HPP
#define REIO_ALLOCATORS_HPP

#include "./asserts.hpp"
#include "./types.hpp"


namespace reio {

    ///
    /// @brief      Base for allocator classes used by @c reio.
    ///
    class base_allocator {
    public:
        virtual ~base_allocator() = default;
        virtual byte* allocate(std::size_t size) = 0;
        virtual void deallocate(byte* ptr) = 0;
    };


    ///
    /// @brief      The most basic @c base_allocator implementation
    ///             which just wraps global new and delete operators.
    ///
    class default_allocator final
        : public base_allocator
    {
    public:

        byte* allocate(std::size_t size) override;
        void deallocate(byte* ptr) override;

        static inline default_allocator* get_default() noexcept {
            static default_allocator instance;
            return &instance;
        }
    };

}


#endif //REIO_ALLOCATORS_HPP
