#include "reio/buffers/owning_buffer.hpp"

namespace reio {

    ///
    /// @brief      Default-initialize the buffer.
    /// @param      alloc           Optional custom allocator to use.
    /// @throw      io_exception    If @c alloc is NULL.
    ///
    owning_buffer::owning_buffer(owning_buffer::alloc_ptr alloc)
        : m_begin{ nullptr }, m_end{ nullptr }
        , m_alloc_end{ nullptr }, m_allocator{ alloc }
        , m_growth{ k_default_growth_factor }
    {
        REIO_ASSERT(alloc != nullptr, "owning buffer can't have null allocator");
    }

    ///
    /// @brief      Initialize the buffer by preallocating some memory.
    /// @param      capacity        Amount of memory to preallocate.
    /// @param      alloc           Optional custom allocator to use.
    /// @throw      io_exception    If @c alloc is NULL, or preallocation fails.
    ///
    owning_buffer::owning_buffer(owning_buffer::size_type capacity,
                                 owning_buffer::alloc_ptr alloc)
        : m_allocator{ alloc }
        , m_growth{ k_default_growth_factor }
    {
        REIO_ASSERT(alloc != nullptr, "owning buffer can't have null allocator");

        pointer allocation = nullptr;
        if (capacity > 0u) {
            allocation = alloc->allocate(capacity);
            REIO_ASSERT(allocation != nullptr, "owning buffer failed to pre-allocate");
        }

        m_begin = m_end = allocation;
        m_alloc_end = allocation + capacity;
    }

    ///
    /// @brief      Initialize the buffer with a repetition of a single value.
    /// @param      length          Number of bytes to initialize.
    /// @param      value           Byte value.
    /// @param      alloc           Optional custom allocator to use.
    /// @throw      io_exception    If @c alloc is NULL, or allocation fails.
    ///
    owning_buffer::owning_buffer(owning_buffer::size_type length,
                                 owning_buffer::value_type value,
                                 owning_buffer::alloc_ptr alloc)
        : owning_buffer{ length, alloc }
    {
        std::fill(m_begin, m_alloc_end, value);
        m_end = m_alloc_end;
    }

    ///
    /// @brief      Initialize the buffer by copying in a chunk of memory.
    /// @param      copy            Chunk of memory to copy.
    /// @param      alloc           Optional custom allocator to use.
    /// @throw      io_exception    If @c alloc is NULL, or allocation fails.
    ///
    owning_buffer::owning_buffer(weak_buffer copy, owning_buffer::alloc_ptr alloc)
        : owning_buffer{ copy.length(), alloc }
    {
        std::copy_n(copy.data(), copy.length(), m_begin);
        m_end = m_alloc_end;
    }

    owning_buffer::~owning_buffer() noexcept
    {
        if (m_alloc_end != nullptr) {
            m_allocator->deallocate(m_begin);
        }
    }

    owning_buffer::owning_buffer(owning_buffer &&other) noexcept
        : m_begin{ std::exchange(other.m_begin, nullptr) }
        , m_end{ std::exchange(other.m_end, nullptr) }
        , m_alloc_end{ std::exchange(other.m_alloc_end, nullptr) }
        , m_allocator{ std::exchange(other.m_allocator, nullptr) }
        , m_growth{ std::exchange(other.m_growth, k_default_growth_factor) }
    {

    }

    owning_buffer&
    owning_buffer::operator=(owning_buffer &&other) noexcept
    {
        if (this != &other) {
            m_begin = std::exchange(other.m_begin, nullptr);
            m_end = std::exchange(other.m_end, nullptr);
            m_alloc_end = std::exchange(other.m_alloc_end, nullptr);
            m_allocator = std::exchange(other.m_allocator, nullptr);
            m_growth = std::exchange(other.m_growth, k_default_growth_factor);
        }
        return *this;
    }

    ///
    /// @brief      Get pointer to the start of the owned bytes.
    /// @return     Pointer to the start of the owned memory block.
    ///
    owning_buffer::pointer
    owning_buffer::data() const noexcept
    {
        return m_begin;
    }

    ///
    /// @brief      Get number of the owned bytes in use.
    /// @return     Length (in bytes) of the used memory block.
    ///
    owning_buffer::size_type
    owning_buffer::length() const noexcept
    {
        return static_cast<size_type>(m_end - m_begin);
    }

    ///
    /// @brief      Get number of the owned bytes in total (possibly beyond what's currently used).
    /// @return     Length (in bytes) of the owned memory block.
    ///
    owning_buffer::size_type
    owning_buffer::capacity() const noexcept
    {
        return static_cast<size_type>(m_alloc_end - m_begin);
    }

    ///
    /// @brief      Get the expansion policy.
    /// @return     Current expansion policy.
    ///
    growth_factor
    owning_buffer::growth() const noexcept
    {
        return m_growth;
    }

    ///
    /// @brief      Get the allocator.
    /// @return     Immutable allocator instance used by the buffer.
    ///
    owning_buffer::alloc_ptr
    owning_buffer::allocator() const noexcept
    {
        return m_allocator;
    }

    ///
    /// @brief      Update the buffer expansion policy.
    /// @param      factor    New buffer expansion policy.
    ///
    void
    owning_buffer::set_growth(growth_factor factor) noexcept
    {
        m_growth = factor;
    }

    ///
    /// @brief      Get a byte within the buffer, without bounds check.
    /// @param      index    Index of the byte.
    /// @return     Non-const reference to a byte at @c index.
    ///
    owning_buffer::reference
    owning_buffer::operator[](owning_buffer::size_type index) noexcept
    {
        return m_begin[index];
    }

    ///
    /// @brief      Get a byte within the buffer, without bounds check.
    /// @param      index    Index of the byte.
    /// @return     Copy of a byte at @c index.
    ///
    owning_buffer::value_type
    owning_buffer::operator[](owning_buffer::size_type index) const noexcept
    {
        return m_begin[index];
    }

    ///
    /// @brief      Get a byte within the buffer, doing a bounds check.
    ///
    /// @param      index           Index of the byte.
    /// @throw      io_exception    When subscript is out of range.
    ///
    /// @return     Copy of a byte at @c index.
    ///
    owning_buffer::value_type
    owning_buffer::at(owning_buffer::size_type index) const
    {
        REIO_ASSERT(index < length(), "subscript out of buffer range");
        return m_begin[index];
    }

    ///
    /// @brief      Get a view of the entire buffer (up to length).
    /// @return     Weak buffer providing a view over the buffer.
    ///
    weak_buffer
    owning_buffer::view() const noexcept
    {
        const auto length_ = static_cast<size_type>(m_end - m_begin);
        return { m_begin, length_ };
    }

    ///
    /// @brief      Get a view over a section of the active memory block.
    ///
    /// @param      offset          Number of bytes from the start of this buffer.
    /// @param      size            Number of bytes in the resulting view.
    /// @throw      io_exception    When @c offset or @c offset + @c size are out of bounds.
    ///
    /// @return     A view over @c size bytes in this buffer starting at @c offset.
    ///
    weak_buffer
    owning_buffer::subview(owning_buffer::size_type offset, owning_buffer::size_type size) const
    {
        REIO_ASSERT(m_begin + offset <= m_end, "subview offset out of buffer bounds");
        REIO_ASSERT(m_begin + offset + size <= m_end, "subview size bigger than buffer length");
        return { m_begin + offset, size };
    }

    ///
    /// @brief      Get a view over a starting section of the active memory block.
    ///
    /// @param      size            Number of bytes in the resulting view.
    /// @throw      io_exception    When @c size is over the buffer's length.
    ///
    /// @return     A view over the first @c size bytes in this buffer.
    ///
    weak_buffer
    owning_buffer::first(owning_buffer::size_type size) const
    {
        REIO_ASSERT(m_begin + size <= m_end, "subview size bigger than buffer length");
        return { m_begin, size };
    }

    ///
    /// @brief      Get a view over an ending section of the active memory block.
    ///
    /// @param      size            Number of bytes in the resulting view.
    /// @throw      io_exception    When @c size is over the buffer's length.
    ///
    /// @return     A view over the last @c size bytes in this buffer.
    ///
    weak_buffer
    owning_buffer::last(owning_buffer::size_type size) const
    {
        REIO_ASSERT(m_begin + size <= m_end, "subview size bigger than buffer length");
        const auto my_length = static_cast<size_type>(m_end - m_begin);
        return { m_begin + my_length - size, size };
    }

    ///
    /// @brief      Get a view over an ending section of the active memory block.
    ///
    /// @param      offset          Number of bytes from the start of this buffer.
    /// @throw      io_exception    When @c offset is out of bounds.
    ///
    /// @return     A view over all the bytes past @c offset in this buffer.
    ///
    weak_buffer
    owning_buffer::last_from(owning_buffer::size_type offset) const
    {
        REIO_ASSERT(m_begin + offset <= m_end, "subview offset out of buffer bounds");
        const auto my_length = static_cast<size_type>(m_end - m_begin);
        return { m_begin + offset, my_length - offset };
    }

    owning_buffer::iterator owning_buffer::begin() noexcept { return m_begin; }
    owning_buffer::iterator owning_buffer::end() noexcept { return m_end; }
    owning_buffer::const_iterator owning_buffer::begin() const noexcept { return m_begin; }
    owning_buffer::const_iterator owning_buffer::end() const noexcept { return m_end; }
    owning_buffer::const_iterator owning_buffer::cbegin() const noexcept { return m_begin; }
    owning_buffer::const_iterator owning_buffer::cend() const noexcept { return m_end; }
    owning_buffer::iterator owning_buffer::alloc_end() noexcept { return m_alloc_end; }
    owning_buffer::const_iterator owning_buffer::alloc_end() const noexcept { return m_alloc_end; }
    owning_buffer::const_iterator owning_buffer::alloc_cend() const noexcept { return m_alloc_end; }

    owning_buffer::size_type
    owning_buffer::next_capacity(owning_buffer::size_type over) const
    {
        switch (m_growth) {
            case growth_factor::none:
            {
                REIO_FAIL("owning buffer can't expand with 'none' growth factor",
                          __FILE__, __LINE__, _REIO_FUNC_);
            }
            case growth_factor::tight:
            {
                return over;
            }
            case growth_factor::mult2x:
            {
                size_type next = std::max<size_type>(1u, capacity());
                while (next < over)
                {
                    next *= 2u;
                }
                return next;
            }
            default:
            {
                REIO_FAIL("unhandled switch case", __FILE__, __LINE__, _REIO_FUNC_);
            }
        }
    }

    void
    owning_buffer::do_realloc(owning_buffer::size_type new_capacity)
    {
        const auto old_length = length();
        const auto old_capacity = capacity();

        if (new_capacity > old_capacity) {
            pointer allocation = m_allocator->allocate(new_capacity);
            REIO_ASSERT(allocation != nullptr, "owning buffer failed to reallocate");

            std::copy(m_begin, m_end, allocation);
            m_allocator->deallocate(m_begin);

            m_begin = allocation;
            m_end = allocation + old_length;
            m_alloc_end = allocation + new_capacity;
        }
    }

}
