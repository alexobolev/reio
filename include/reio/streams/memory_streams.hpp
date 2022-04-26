#ifndef REIO_MEMORY_STREAMS_HPP
#define REIO_MEMORY_STREAMS_HPP

#include "../asserts.hpp"
#include "../types.hpp"
#include "../buffers/owning_buffer.hpp"
#include "../buffers/weak_buffer.hpp"
#include "./streams.hpp"


namespace reio
{

    ///
    /// @brief      Implementation of @c input_stream using
    ///             an @c owning_buffer as a data source.
    ///
    /// @ingroup    streams
    ///
    class memory_input_stream final
        : public input_stream
        , public non_copyable
    {
    private:

        owning_buffer   m_buffer;
        int64_t         m_position;

    public:

        ///
        /// @brief      Initialize stream by copying a block of data into it.
        ///
        /// @param      source_view    View of the data block to copy into the underlying buffer
        /// @param      alloc          Allocator which should be used by the buffer.
        ///
        explicit memory_input_stream(weak_buffer source_view, base_allocator* alloc = default_allocator::get_default());

        ///
        /// @brief      Initialize stream by acquiring ownership of an owning_buffer.
        /// @param      source_buffer    Buffer to move into the stream.
        ///
        explicit memory_input_stream(owning_buffer&& source_buffer);

        ~memory_input_stream() override;

        memory_input_stream(memory_input_stream&& other) noexcept;
        memory_input_stream& operator=(memory_input_stream&& other) noexcept;

        [[nodiscard]] weak_buffer view() const noexcept;
        [[nodiscard]] int64_t capacity() const noexcept;
        [[nodiscard]] growth_factor growth() const noexcept;

        //* Implementation of base_stream.
        //* ========================================

        int64_t position() override;
        int64_t length() override;
        void seek_begin(int64_t offset) override;
        void seek_current(int64_t offset) override;
        void seek_end(int64_t offset) override;

        //* Implementation of input_stream.
        //* ========================================

        int64_t read_bytes(weak_buffer output) override;

    };


    ///
    /// @brief      Implementation of @c input_stream using
    ///             an @c owning_buffer as a data sink.
    ///
    /// @ingroup    streams
    ///
    class memory_output_stream final
        : public output_stream
        , public non_copyable
    {
    private:

        owning_buffer   m_buffer;
        int64_t         m_position;

    public:

        ///
        /// @brief      Initialize empty stream.
        /// @param      alloc    Optional custom allocator for use by the stream.
        ///
        explicit memory_output_stream(base_allocator* alloc = default_allocator::get_default());

        ///
        /// @brief      Initialize the stream to have some initial capacity.
        ///
        /// @param      capacity    Number of bytes to preallocate.
        /// @param      alloc       Optional custom allocator for use by the stream.
        ///
        explicit memory_output_stream(uint64_t capacity, base_allocator* alloc = default_allocator::get_default());

        ///
        /// @brief      Initialize the stream to have some initial capacity and a growth policy.
        ///
        /// Can be used to e.g. create a fixed-size memory stream.
        ///
        /// @param      capacity    Number of bytes to preallocate.
        /// @param      growth      Expansion factor for the underlying buffer.
        /// @param      alloc       Optional custom allocator for use by the stream.
        ///
        explicit memory_output_stream(uint64_t capacity, growth_factor growth, base_allocator* alloc = default_allocator::get_default());

        ~memory_output_stream() override;

        memory_output_stream(memory_output_stream&& other) noexcept;
        memory_output_stream& operator=(memory_output_stream&& other) noexcept;

        [[nodiscard]] weak_buffer view() const noexcept;
        [[nodiscard]] int64_t capacity() const noexcept;
        [[nodiscard]] growth_factor growth() const noexcept;

        //* Implementation of base_stream.
        //* ========================================

        int64_t position() override;
        int64_t length() override;
        void seek_begin(int64_t offset) override;
        void seek_current(int64_t offset) override;
        void seek_end(int64_t offset) override;

        //* Implementation of output_stream.
        //* ========================================

        int64_t write_bytes(weak_buffer input) override;
    };

}

#endif //REIO_MEMORY_STREAMS_HPP
