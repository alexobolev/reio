#ifndef REIO_WEAK_BUFFER_HPP
#define REIO_WEAK_BUFFER_HPP

#include "../asserts.hpp"
#include "../types.hpp"


namespace reio
{

    ///
    /// @brief      Non-owning fixed-size view of a contiguous byte sequence.
    ///
    /// This buffer simply stores the start and the end of a binary
    /// range allocated somewhere else, which allows the programmer
    /// to pass it around to classes and methods which do not need
    /// to take ownership of the binary range, without having to
    /// avoid doing costly allocations on copy. @n
    ///
    /// Also, this better communicates the possible intent,
    /// effectively decoupling logic from the exact implementation
    /// used for storing input data on which it may operate. @n
    ///
    /// @ingroup    buffers
    ///
    class weak_buffer final
    {
    public:

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

    public:

        constexpr weak_buffer() = default;
        constexpr weak_buffer(pointer data, size_type length) noexcept;

        constexpr weak_buffer(const weak_buffer& other) = default;
        constexpr weak_buffer& operator=(const weak_buffer& other) = default;

        constexpr weak_buffer(weak_buffer&& other) noexcept;
        constexpr weak_buffer& operator=(weak_buffer&& other) noexcept;

        [[nodiscard]] constexpr pointer data() const noexcept;
        [[nodiscard]] constexpr size_type length() const noexcept;

        [[nodiscard]] constexpr reference operator[](size_type index) noexcept;
        [[nodiscard]] constexpr value_type operator[](size_type index) const noexcept;

        [[nodiscard]] constexpr value_type at(size_type index) const;

        [[nodiscard]] constexpr weak_buffer view() const noexcept;
        [[nodiscard]] constexpr weak_buffer subview(size_type offset, size_type size) const;

        [[nodiscard]] constexpr weak_buffer first(size_type size) const;
        [[nodiscard]] constexpr weak_buffer last(size_type size) const;
        [[nodiscard]] constexpr weak_buffer last_from(size_type offset) const;

        [[nodiscard]] constexpr iterator begin() noexcept;
        [[nodiscard]] constexpr iterator end() noexcept;
        [[nodiscard]] constexpr const_iterator begin() const noexcept;
        [[nodiscard]] constexpr const_iterator end() const noexcept;
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept;
        [[nodiscard]] constexpr const_iterator cend() const noexcept;

        template<std::contiguous_iterator ContigIt>
        constexpr iterator overwrite(ContigIt src_begin, ContigIt src_end, const_iterator dest_begin);

        template<std::contiguous_iterator ContigIt>
        constexpr pointer insert(ContigIt src_begin, ContigIt src_end, const_iterator dest_begin);

    private:

        constexpr bool iter_within(std::contiguous_iterator auto iter) const noexcept;

    };


    ///
    /// @brief      Initialize buffer with a view of a binary block.
    ///
    /// @param      data    Pointer to the viewed block.
    /// @param      length    Number of bytes in the viewed block.
    ///
    constexpr weak_buffer::weak_buffer(
            weak_buffer::pointer const data,
            weak_buffer::size_type length) noexcept
        : m_begin{ data }, m_end{ data + length } { }


    ///
    /// @brief      Move-constructor which resets the moved-from buffer.
    /// @param      other    Instance of @c weak_buffer to move from.
    ///
    /// Default generated move constructor and assignment operator
    /// leave the moved-from instance initialized, which is counter-
    /// productive because there are cases when the programmer may
    /// want to rely on the moved-from @c weak_buffer being empty,
    /// even though that's not particularly idiomatic.
    ///
    constexpr weak_buffer::weak_buffer(weak_buffer &&other) noexcept
        : m_begin{ std::exchange(other.m_begin, nullptr) }
        , m_end{ std::exchange(other.m_end, nullptr) } { }


    ///
    /// @brief      Move-assignment operator which resets the moved-from buffer.
    /// @param      other    Instance of @c weak_buffer to move from.
    /// @return     Current instance.
    ///
    /// @see        weak_buffer(weak_buffer&&)
    ///
    constexpr weak_buffer&
    weak_buffer::operator=(weak_buffer&& other) noexcept
    {
        if (this != &other) {
            m_begin = std::exchange(other.m_begin, nullptr);
            m_end = std::exchange(other.m_end, nullptr);
        }
        return *this;
    }


    ///
    /// @brief      Get pointer to the start of viewed range.
    /// @return     Pointer to the start of viewed range.
    ///
    constexpr weak_buffer::pointer
    weak_buffer::data() const noexcept
    {
        return m_begin;
    }


    ///
    /// @brief      Get length of the viewed range.
    /// @return     Length (in bytes) of the viewed range.
    ///
    constexpr weak_buffer::size_type
    weak_buffer::length() const noexcept
    {
        return m_end - m_begin;
    }

    ///
    /// @brief      Get a byte within the buffer, without bounds check.
    /// @param      index    Index of the byte.
    /// @return     Non-const reference to a byte at @c index.
    ///
    constexpr weak_buffer::reference
    weak_buffer::operator[](weak_buffer::size_type index) noexcept
    {
        return m_begin[index];
    }

    ///
    /// @brief      Get a byte within the buffer, without bounds check.
    /// @param      index    Index of the byte.
    /// @return     Copy of a byte at @c index.
    ///
    constexpr weak_buffer::value_type
    weak_buffer::operator[](weak_buffer::size_type index) const noexcept
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
    constexpr weak_buffer::value_type
    weak_buffer::at(weak_buffer::size_type index) const
    {
        REIO_ASSERT(index < length(), "subscript out of buffer range");
        return m_begin[index];
    }


    ///
    /// @brief      Get a "subview" over the entire viewed range.
    /// @return     Essentially, a copy of this buffer.
    ///
    constexpr weak_buffer
    weak_buffer::view() const noexcept
    {
        const auto length_ = static_cast<size_type>(m_end - m_begin);
        return { m_begin, length_ };
    }

    ///
    /// @brief      Get a view over a section of the viewed range.
    ///
    /// @param      offset          Number of bytes from the start of this buffer.
    /// @param      size            Number of bytes in the resulting view.
    /// @throw      io_exception    When @c offset or @c offset + @c size are out of bounds.
    ///
    /// @return     A view over @c size bytes in this buffer starting at @c offset.
    ///
    constexpr weak_buffer
    weak_buffer::subview(weak_buffer::size_type offset, weak_buffer::size_type size) const
    {
        REIO_ASSERT(m_begin + offset <= m_end, "subview offset out of buffer bounds");
        REIO_ASSERT(m_begin + offset + size <= m_end, "subview size bigger than buffer length");
        return { m_begin + offset, size };
    }


    ///
    /// @brief      Get a view over a starting section of the viewed range.
    ///
    /// @param      size            Number of bytes in the resulting view.
    /// @throw      io_exception    When @c size is over the buffer's length.
    ///
    /// @return     A view over the first @c size bytes in this buffer.
    ///
    constexpr weak_buffer
    weak_buffer::first(weak_buffer::size_type size) const
    {
        REIO_ASSERT(m_begin + size <= m_end, "subview size bigger than buffer length");
        return { m_begin, size };
    }


    ///
    /// @brief      Get a view over an ending section of the viewed range.
    ///
    /// @param      size            Number of bytes in the resulting view.
    /// @throw      io_exception    When @c size is over the buffer's length.
    ///
    /// @return     A view over the last @c size bytes in this buffer.
    ///
    constexpr weak_buffer
    weak_buffer::last(weak_buffer::size_type size) const
    {
        REIO_ASSERT(m_begin + size <= m_end, "subview size bigger than buffer length");
        const auto my_length = static_cast<size_type>(m_end - m_begin);
        return { m_begin + my_length - size, size };
    }


    ///
    /// @brief      Get a view over an ending section of the viewed range.
    ///
    /// @param      offset          Number of bytes from the start of this buffer.
    /// @throw      io_exception    When @c offset is out of bounds.
    ///
    /// @return     A view over all the bytes past @c offset in this buffer.
    ///
    constexpr weak_buffer
    weak_buffer::last_from(weak_buffer::size_type offset) const
    {
        REIO_ASSERT(m_begin + offset <= m_end, "subview offset out of buffer bounds");
        const auto my_length = static_cast<size_type>(m_end - m_begin);
        return { m_begin + offset, my_length - offset };
    }

    constexpr weak_buffer::iterator weak_buffer::begin() noexcept { return m_begin; }
    constexpr weak_buffer::iterator weak_buffer::end() noexcept { return m_end; }
    constexpr weak_buffer::const_iterator weak_buffer::begin() const noexcept { return m_begin; }
    constexpr weak_buffer::const_iterator weak_buffer::end() const noexcept { return m_end; }
    constexpr weak_buffer::const_iterator weak_buffer::cbegin() const noexcept { return m_begin; }
    constexpr weak_buffer::const_iterator weak_buffer::cend() const noexcept { return m_end; }

    ///
    /// @brief      Overwrite a range within the buffer with bytes in [src_begin; src_end).
    ///
    /// The buffer MUST have sufficient space for the input bytes.
    /// Source and destination ranges MUST NOT overlap.
    ///
    /// @param      src_begin       Iterator before the first replacement byte.
    /// @param      src_end         Iterator past the last replacement byte.
    /// @param      dest_begin      Iterator before the first byte to overwrite.
    ///
    /// @throw      io_exception    When @c src_begin and @c src_end are misordered,
    ///                             or @c dest_begin is out of bounds, or there's not
    ///                             enough place in the buffer to fit the source range.
    ///
    /// @return     Iterator past the last overwritten byte.
    ///
    template<std::contiguous_iterator ContigIt>
    constexpr weak_buffer::iterator
    weak_buffer::overwrite(ContigIt src_begin, ContigIt src_end, weak_buffer::const_iterator dest_begin)
    {
        REIO_ASSERT(src_begin <= src_end, "source iterators are out of order");
        REIO_ASSERT(iter_within(dest_begin), "destination iterator is out of buffer bounds");

        // non-const iterator copy without a const_cast
        iterator dest_iter = begin() + (dest_begin - cbegin());

        const difference_type space_available = cend() - dest_begin;
        const difference_type write_length = src_end - src_begin;

        REIO_ASSERT(write_length <= space_available, "overwrite would overflow the buffer");
        return std::copy(src_begin, src_end, dest_iter);
    }

    ///
    /// @brief      Insert bytes to position at @c dest_begin from an iterator range.
    ///
    /// Existing buffer contents and bytes of the input range
    /// which would overflow the buffer are discared. @n
    ///
    /// The buffer MUST have sufficient space for the input bytes.
    /// Source and destination ranges MUST NOT overlap. @n
    ///
    /// @param      src_begin       Iterator before the first inserted byte.
    /// @param      src_end         Iterator past the last inserted byte.
    /// @param      dest_begin      Iterator before the first byte to be moved by insertion.
    ///
    /// @throw      io_exception    When @c src_begin and @c src_end are misordered,
    ///                             or @c dest_begin is out of bounds, or there's not
    ///                             enough place in the buffer to fit the source range.
    ///
    /// @return     Iterator past the last inserted byte (within this buffer).
    ///
    template<std::contiguous_iterator ContigIt>
    constexpr weak_buffer::pointer
    weak_buffer::insert(ContigIt src_begin, ContigIt src_end, weak_buffer::const_iterator dest_begin)
    {
        REIO_ASSERT(src_begin <= src_end, "source iterators are out of order");
        REIO_ASSERT(iter_within(dest_begin), "destination iterator is out of buffer bounds");

        // non-const iterator copy without a const_cast
        iterator dest_iter = begin() + (dest_begin - cbegin());

        const difference_type space_available = cend() - dest_begin;
        const difference_type write_length = src_end - src_begin;

        REIO_ASSERT(write_length <= space_available, "insert would overflow the buffer");
        std::shift_right(dest_iter, m_end, write_length);
        return std::copy(src_begin, src_end, dest_iter);
    }

    constexpr bool
    weak_buffer::iter_within(std::contiguous_iterator auto iter) const noexcept
    {
        return iter >= m_begin && iter <= m_end;
    }

}

#endif //REIO_WEAK_BUFFER_HPP
