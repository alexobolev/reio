#ifndef REIO_MEMORY_STREAMS_HPP
#define REIO_MEMORY_STREAMS_HPP

#include "../asserts.hpp"
#include "../types.hpp"
#include "../buffers/owning_buffer.hpp"
#include "../buffers/weak_buffer.hpp"
#include "./streams.hpp"


namespace reio
{

    class memory_input_stream final
        : public input_stream
        , public non_copyable
    {
    private:

        owning_buffer   m_buffer;
        int64_t         m_position;

    public:

        explicit memory_input_stream(weak_buffer source_view, base_allocator* alloc = default_allocator::get_default());
        explicit memory_input_stream(owning_buffer&& source_buffer);

        ~memory_input_stream() override;

        memory_input_stream(memory_input_stream&& other) noexcept;
        memory_input_stream& operator=(memory_input_stream&& other) noexcept;

        [[nodiscard]] weak_buffer view() const noexcept;

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


    class memory_output_stream final
        : public output_stream
        , public non_copyable
    {
    private:

        owning_buffer   m_buffer;
        int64_t         m_position;

    public:

        explicit memory_output_stream(base_allocator* alloc = default_allocator::get_default());
        explicit memory_output_stream(uint64_t capacity, base_allocator* alloc = default_allocator::get_default());
        explicit memory_output_stream(uint64_t capacity, growth_factor growth, base_allocator* alloc = default_allocator::get_default());

        ~memory_output_stream() override;

        memory_output_stream(memory_output_stream&& other) noexcept;
        memory_output_stream& operator=(memory_output_stream&& other) noexcept;

        [[nodiscard]] weak_buffer view() const noexcept;

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
