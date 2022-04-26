#include "reio/streams/memory_streams.hpp"

namespace reio
{

    template<seek_origin Origin>
    [[nodiscard]] constexpr int64_t
    DoCalcPosition(
            [[maybe_unused]] int64_t length,
            [[maybe_unused]] int64_t position,
            [[maybe_unused]] int64_t offset)
    {
        if constexpr (Origin == seek_origin::begin)
        {
            REIO_ASSERT(offset >= 0, "can't seek negative offset from the beginning of the underlying buffer");
            REIO_ASSERT(offset < length, "can't seek offset from the beginning beyond the underlying buffer");
            return offset;
        }
        if constexpr (Origin == seek_origin::current)
        {
            const auto new_position = position + offset;
            REIO_ASSERT(new_position >= 0, "can't seek offset below the underlying buffer's start");
            REIO_ASSERT(new_position <= length, "can't seek offset beyond the underlying buffer's end");
            return new_position;
        }
        if constexpr (Origin == seek_origin::end)
        {
            REIO_ASSERT(offset <= 0, "can't seek positive offset from the end of the underlying buffer");
            REIO_ASSERT(offset > -length, "can't seek offset from the beginning beyond the underlying buffer");
            return length + offset;
        }
        REIO_FAIL("unhandled seek origin", __FILE__, __LINE__, _REIO_FUNC_);
//        return std::numeric_limits<int64_t>::min();
    }

    ///
    /// @brief      Initialize stream by copying a block of data into it.
    ///
    /// @param      source_view    View of the data block to copy into the underlying buffer
    /// @param      alloc          Allocator which should be used by the buffer.
    ///
    memory_input_stream::memory_input_stream(weak_buffer source_view, base_allocator* alloc)
        : m_buffer{ source_view, alloc }
        , m_position{ 0u }
    {
        REIO_ASSERT(source_view.length() != 0u, "can't initialize memory input stream with an empty view");
        REIO_ASSERT(alloc != nullptr, "can't initialize memory input stream with a null allocator");
        m_buffer.set_growth(growth_factor::none);
    }

    ///
    /// @brief      Initialize stream by acquiring ownership of an owning_buffer.
    /// @param      source_buffer    Buffer to move into the stream.
    ///
    memory_input_stream::memory_input_stream(owning_buffer &&source_buffer)
        : m_buffer{ std::move(source_buffer) }
        , m_position{ 0u }
    {

    }

    memory_input_stream::~memory_input_stream() = default;

    memory_input_stream::memory_input_stream(memory_input_stream &&other) noexcept
    {
        m_buffer = std::move(other.m_buffer);
        m_position = std::exchange(other.m_position, 0);
    }

    memory_input_stream &
    memory_input_stream::operator=(memory_input_stream &&other) noexcept
    {
        if (this != &other) {
            m_buffer = std::move(other.m_buffer);
            m_position = std::exchange(other.m_position, 0);
        }

        return *this;
    }

    weak_buffer
    memory_input_stream::view() const noexcept
    {
        return m_buffer.view();
    }

    int64_t
    memory_input_stream::capacity() const noexcept
    {
        return static_cast<int64_t>(m_buffer.capacity());
    }

    growth_factor
    memory_input_stream::growth() const noexcept
    {
        return m_buffer.growth();
    }

    int64_t
    memory_input_stream::position()
    {
        return m_position;
    }

    int64_t
    memory_input_stream::length()
    {
        return static_cast<int64_t>(m_buffer.length());
    }

    void
    memory_input_stream::seek_begin(int64_t offset)
    {
        const auto length_ = static_cast<int64_t>(m_buffer.length());
        m_position = DoCalcPosition<seek_origin::begin>(length_, m_position, offset);
    }

    void
    memory_input_stream::seek_current(int64_t offset)
    {
        const auto length_ = static_cast<int64_t>(m_buffer.length());
        m_position = DoCalcPosition<seek_origin::current>(length_, m_position, offset);
    }

    void
    memory_input_stream::seek_end(int64_t offset)
    {
        const auto length_ = static_cast<int64_t>(m_buffer.length());
        m_position = DoCalcPosition<seek_origin::end>(length_, m_position, offset);
    }

    int64_t
    memory_input_stream::read_bytes(weak_buffer output)
    {
        REIO_ASSERT(output.data() != nullptr, "can't read from input streams into nullptr");
        REIO_ASSERT(output.length() > 0, "can't read zero bytes from input streams");

        const auto remaining_length = m_buffer.length() - m_position;
        const auto read_length = std::min<int64_t>(output.length(), remaining_length);

        std::copy_n(m_buffer.data() + m_position, read_length, output.data());
        m_position += read_length;

        return read_length;
    }


    memory_output_stream::memory_output_stream(base_allocator *alloc)
        : m_buffer{ alloc }
        , m_position{ 0 }
    {
        m_buffer.set_growth(k_default_growth_factor);
    }

    memory_output_stream::memory_output_stream(uint64_t capacity, base_allocator *alloc)
        : m_buffer{ static_cast<owning_buffer::size_type>(capacity), alloc }
        , m_position{ 0 }
    {
        REIO_ASSERT(capacity != 0u, "don't use preallocating constructor for zero capacity");
        m_buffer.set_growth(k_default_growth_factor);
    }

    memory_output_stream::memory_output_stream(uint64_t capacity, growth_factor growth, base_allocator *alloc)
        : m_buffer{ static_cast<owning_buffer::size_type>(capacity), alloc }
        , m_position{ 0 }
    {
        REIO_ASSERT(capacity != 0u, "don't use preallocating constructor for zero capacity");
        m_buffer.set_growth(growth);
    }

    memory_output_stream::memory_output_stream(memory_output_stream &&other) noexcept
    {
        m_buffer = std::move(other.m_buffer);
        m_position = std::exchange(other.m_position, 0);
    }

    memory_output_stream &
    memory_output_stream::operator=(memory_output_stream &&other) noexcept
    {
        if (this != &other) {
            m_buffer = std::move(other.m_buffer);
            m_position = std::exchange(other.m_position, 0);
        }

        return *this;
    }

    memory_output_stream::~memory_output_stream() = default;

    weak_buffer
    memory_output_stream::view() const noexcept
    {
        return m_buffer.view();
    }

    int64_t
    memory_output_stream::capacity() const noexcept
    {
        return static_cast<int64_t>(m_buffer.capacity());
    }

    growth_factor
    memory_output_stream::growth() const noexcept
    {
        return m_buffer.growth();
    }

    int64_t
    memory_output_stream::position()
    {
        return m_position;
    }

    int64_t
    memory_output_stream::length()
    {
        return static_cast<int64_t>(m_buffer.length());
    }


    void
    memory_output_stream::seek_begin(int64_t offset)
    {
        const auto length_ = static_cast<int64_t>(m_buffer.length());
        m_position = DoCalcPosition<seek_origin::begin>(length_, m_position, offset);
    }

    void
    memory_output_stream::seek_current(int64_t offset)
    {
        const auto length_ = static_cast<int64_t>(m_buffer.length());
        m_position = DoCalcPosition<seek_origin::current>(length_, m_position, offset);
    }

    void
    memory_output_stream::seek_end(int64_t offset)
    {
        const auto length_ = static_cast<int64_t>(m_buffer.length());
        m_position = DoCalcPosition<seek_origin::end>(length_, m_position, offset);
    }

    int64_t
    memory_output_stream::write_bytes(weak_buffer input)
    {
        REIO_ASSERT(input.data() != nullptr, "can't write to output streams from nullptr");
        REIO_ASSERT(input.length() > 0, "can't write zero bytes to output streams");

        // artificially limit the write length for fixed-size streams
        // to change the behaviour on overflow from failure to partial write
        if (m_buffer.growth() == growth_factor::none)
        {
            const auto remaining_capacity = m_buffer.capacity() - m_position;
            const auto write_length = std::min<int64_t>(input.length(), remaining_capacity);

            m_buffer.overwrite(input.begin(), input.begin() + write_length, m_buffer.begin() + m_position);
            m_position += write_length;

            return write_length;
        }

        m_buffer.overwrite(input.begin(), input.end(), m_buffer.begin() + m_position);
        m_position += input.length();

        return input.length();
    }
}
