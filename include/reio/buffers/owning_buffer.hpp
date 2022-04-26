#ifndef REIO_OWNING_BUFFER_HPP
#define REIO_OWNING_BUFFER_HPP

#include "../allocators.hpp"
#include "../asserts.hpp"
#include "../types.hpp"
#include "../buffers/weak_buffer.hpp"


namespace reio
{

    ///
    /// Buffer expansion policy for use in owning_buffer,
    /// possible derivatives, and in-memory streams.
    ///
    /// @see        k_default_growth_factor
    ///
    enum class growth_factor : uint32_t
    {
        none = 1,               //< Buffer cannot expand anymore.
        tight = 2,              //< Buffer expands only as much as necessary.
        mult2x = 3              //< Buffer expands linearly, with double coefficient.
    };

    static constexpr auto k_default_growth_factor = growth_factor::mult2x;


    ///
    /// @brief      Dynamically-sized contiguous byte sequence
    ///             which manages its own allocations.
    ///
    /// The go-to buffer for dynamic blob allocations that could be seen
    /// as a simpler in-codebase alternative to vector<byte>. @n
    ///
    /// @ingroup    buffers
    ///
    class owning_buffer final : public non_copyable
    {
    public:

        using alloc_ptr         = base_allocator*;

        using size_type         = std::size_t;
        using difference_type   = std::ptrdiff_t;

        using value_type        = byte;
        using pointer           = value_type*;
        using reference         = value_type&;

        using iterator          = value_type*;
        using const_iterator    = const value_type*;

    private:

        pointer                 m_begin;
        pointer                 m_end;
        pointer                 m_alloc_end;
        alloc_ptr               m_allocator;
        growth_factor           m_growth;

    public:

        explicit owning_buffer(alloc_ptr alloc = default_allocator::get_default());
        explicit owning_buffer(size_type capacity, alloc_ptr alloc = default_allocator::get_default());
        owning_buffer(size_type length, value_type value, alloc_ptr alloc = default_allocator::get_default());
        explicit owning_buffer(weak_buffer copy, alloc_ptr alloc = default_allocator::get_default());
        ~owning_buffer() noexcept;

        owning_buffer(owning_buffer&& other) noexcept;
        owning_buffer& operator=(owning_buffer&& other) noexcept;

        [[nodiscard]] pointer data() const noexcept;
        [[nodiscard]] size_type length() const noexcept;
        [[nodiscard]] size_type capacity() const noexcept;
        [[nodiscard]] growth_factor growth() const noexcept;
        [[nodiscard]] alloc_ptr allocator() const noexcept;

        void set_growth(growth_factor factor) noexcept;

        void resize_to_zero() noexcept;
        void resize_to_capacity() noexcept;

        [[nodiscard]] reference operator[](size_type index) noexcept;
        [[nodiscard]] value_type operator[](size_type index) const noexcept;

        [[nodiscard]] value_type at(size_type index) const;

        [[nodiscard]] weak_buffer view() const noexcept;
        [[nodiscard]] weak_buffer subview(size_type offset, size_type size) const;

        [[nodiscard]] weak_buffer first(size_type size) const;
        [[nodiscard]] weak_buffer last(size_type size) const;
        [[nodiscard]] weak_buffer last_from(size_type offset) const;

        [[nodiscard]] iterator begin() noexcept;
        [[nodiscard]] iterator end() noexcept;
        [[nodiscard]] const_iterator begin() const noexcept;
        [[nodiscard]] const_iterator end() const noexcept;
        [[nodiscard]] const_iterator cbegin() const noexcept;
        [[nodiscard]] const_iterator cend() const noexcept;
        [[nodiscard]] iterator alloc_end() noexcept;
        [[nodiscard]] const_iterator alloc_end() const noexcept;
        [[nodiscard]] const_iterator alloc_cend() const noexcept;

        template<std::contiguous_iterator ContigIt>
        iterator overwrite(ContigIt src_begin, ContigIt src_end, const_iterator dest_begin);

        template<std::contiguous_iterator ContigIt>
        iterator insert(ContigIt src_begin, ContigIt src_end, const_iterator dest_begin);

        iterator erase(const_iterator first, const_iterator last);



    private:

        [[nodiscard]] inline bool iter_within(std::contiguous_iterator auto iter) const noexcept;

        [[nodiscard]] size_type next_capacity(size_type over) const;
        void do_realloc(size_type new_capacity);

    };


    ///
    /// @brief      Overwrite a block within the buffer, possibly extending it.
    ///
    /// Source and destination blocks MUST NOT overlap.
    ///
    /// @param      src_begin       Iterator before the first source byte.
    /// @param      src_end         Iterator past the last source byte.
    /// @param      dest_begin      Iterator before the first byte to overwrite.

    /// @throw      io_exception    When @c src_begin and @c src_end are misordered,
    ///                             or when @c dest_begin is out of bounds.
    ///
    /// @return     Iterator past the last overwritten byte.
    ///
    template<std::contiguous_iterator ContigIt>
    owning_buffer::iterator
    owning_buffer::overwrite(ContigIt src_begin, ContigIt src_end, owning_buffer::const_iterator dest_begin)
    {
        REIO_ASSERT(src_begin <= src_end, "source iterators are out of order");
        REIO_ASSERT(iter_within(dest_begin), "destination iterator is out of buffer bounds");

        const difference_type old_length = m_end - m_begin;
//        const difference_type old_capacity = m_alloc_end - m_begin;

        const difference_type space_available = m_alloc_end - dest_begin;
        const difference_type write_length = src_end - src_begin;
        const difference_type write_offset = dest_begin - cbegin();

        if (write_length > space_available) {
            const auto min_capacity = static_cast<size_type>(write_offset + write_length);
            const auto new_capacity = next_capacity(/*over:*/min_capacity);
            do_realloc(new_capacity);
        }

        m_end = m_begin + std::max<difference_type>(old_length, write_offset + write_length);
        return std::copy(src_begin, src_end, m_begin + write_offset);
    }

    ///
    /// @brief      Insert a block of bytes into the buffer, possibly extending it.
    ///
    /// Source and destination blocks MUST NOT overlap.
    ///
    /// @param      src_begin       Iterator before the first inserted byte.
    /// @param      src_end         Iterator past the last inserted byte.
    /// @param      dest_begin      Iterator before the first byte to be moved by insertion.
    ///
    /// @throw      io_exception    When @c src_begin and @c src_end are misordered,
    ///                             or when @c dest_begin is out of bounds.
    ///
    /// @return     Iterator past the last inserted byte (within this buffer).
    ///
    template<std::contiguous_iterator ContigIt>
    owning_buffer::iterator
    owning_buffer::insert(ContigIt src_begin, ContigIt src_end, owning_buffer::const_iterator dest_begin)
    {
        REIO_ASSERT(src_begin <= src_end, "source iterators are out of order");
        REIO_ASSERT(iter_within(dest_begin), "destination iterator is out of buffer bounds");

        const difference_type old_length = m_end - m_begin;

        const difference_type space_available = m_alloc_end - m_end;
        const difference_type write_length = src_end - src_begin;
        const difference_type write_offset = dest_begin - cbegin();

        if (write_length > space_available) {
            const auto min_capacity = static_cast<size_type>(old_length + write_length);
            const auto new_capacity = next_capacity(/*over:*/min_capacity);
            do_realloc(new_capacity);
        }

        // non-const copy of iterator
        iterator dest_iter = begin() + write_offset;

        std::shift_right(dest_iter, m_alloc_end, write_length);
        std::copy(src_begin, src_end, dest_iter);

//        m_end = m_begin + std::max<difference_type>(old_length, write_offset + write_length);
        m_end = m_begin + old_length + write_length;
        return dest_iter + write_length;
    }

    inline bool
    owning_buffer::iter_within(std::contiguous_iterator auto iter) const noexcept
    {
        return iter >= m_begin && iter <= m_end;
    }

}

#endif //REIO_OWNING_BUFFER_HPP
