#include "reio/buffers/owning_buffer.hpp"

namespace reio {

    owning_buffer::owning_buffer(owning_buffer::alloc_ptr alloc)
        : m_begin{ nullptr }, m_end{ nullptr }
        , m_alloc_end{ nullptr }, m_allocator{ alloc }
        , m_growth{ growth_factor::mult2x }
    {
        REIO_ASSERT(alloc != nullptr, "owning buffer can't have null allocator");
    }

    owning_buffer::owning_buffer(owning_buffer::size_type capacity,
                                 owning_buffer::alloc_ptr alloc)
        : m_allocator{ alloc }
        , m_growth{ growth_factor::mult2x }
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

    owning_buffer::owning_buffer(owning_buffer::size_type length,
                                 owning_buffer::value_type value,
                                 owning_buffer::alloc_ptr alloc)
        : owning_buffer{ length, alloc }
    {
        std::fill(m_begin, m_alloc_end, value);
        m_end = m_alloc_end;
    }

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
        , m_growth{ std::exchange(other.m_growth, growth_factor::mult2x) }
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
            m_growth = std::exchange(other.m_growth, growth_factor::mult2x);
        }
        return *this;
    }

    owning_buffer::pointer
    owning_buffer::data() const noexcept
    {
        return m_begin;
    }

    owning_buffer::size_type
    owning_buffer::length() const noexcept
    {
        return static_cast<size_type>(m_end - m_begin);
    }

    owning_buffer::size_type
    owning_buffer::capacity() const noexcept
    {
        return static_cast<size_type>(m_alloc_end - m_begin);
    }

    growth_factor
    owning_buffer::growth() const noexcept
    {
        return m_growth;
    }

    void
    owning_buffer::set_growth(growth_factor factor) noexcept
    {
        m_growth = factor;
    }

    owning_buffer::reference
    owning_buffer::operator[](owning_buffer::size_type index) noexcept
    {
        return m_begin[index];
    }

    owning_buffer::value_type
    owning_buffer::operator[](owning_buffer::size_type index) const noexcept
    {
        return m_begin[index];
    }

    owning_buffer::value_type
    owning_buffer::at(owning_buffer::size_type index) const
    {
        REIO_ASSERT(index < length(), "subscript out of buffer range");
        return m_begin[index];
    }

    weak_buffer
    owning_buffer::view() const noexcept
    {
        const auto length_ = static_cast<size_type>(m_end - m_begin);
        return { m_begin, length_ };
    }

    weak_buffer
    owning_buffer::subview(owning_buffer::size_type offset, owning_buffer::size_type size) const
    {
        REIO_ASSERT(m_begin + offset <= m_end, "subview offset out of buffer bounds");
        REIO_ASSERT(m_begin + offset + size <= m_end, "subview size bigger than buffer length");
        return { m_begin + offset, size };
    }

    weak_buffer
    owning_buffer::first(owning_buffer::size_type size) const
    {
        REIO_ASSERT(m_begin + size <= m_end, "subview size bigger than buffer length");
        return { m_begin, size };
    }

    weak_buffer
    owning_buffer::last(owning_buffer::size_type size) const
    {
        REIO_ASSERT(m_begin + size <= m_end, "subview size bigger than buffer length");
        const auto my_length = static_cast<size_type>(m_end - m_begin);
        return { m_begin + my_length - size, size };
    }

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
