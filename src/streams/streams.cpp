#include "reio/streams/streams.hpp"

namespace reio
{
    int64_t
    input_stream::read_byte()
    {
        byte value = 0u;
        static_assert(sizeof value == 1u);

        const auto buffer = weak_buffer{ &value, sizeof value };
        const auto read = read_bytes(buffer);

        if (read != sizeof value)
        {
            return -1;
        }

        return static_cast<int64_t>(value);
    }

    void
    input_stream::read_bytes_or_fail(weak_buffer output)
    {
        [[maybe_unused]] const auto read = read_bytes(output);
        [[maybe_unused]] const auto expected = static_cast<int64_t>(output.length());
        REIO_ASSERT(read == expected, "failed to read required number of bytes");
    }

    bool
    output_stream::write_byte(byte value)
    {
        static_assert(sizeof value == 1u);

        const auto buffer = weak_buffer{ &value, sizeof value };
        const auto written = write_bytes(buffer);

        return written == sizeof value;
    }

    void
    output_stream::write_bytes_or_fail(weak_buffer input)
    {
        [[maybe_unused]] const auto written = write_bytes(input);
        [[maybe_unused]] const auto expected = static_cast<int64_t>(input.length());
        REIO_ASSERT(written == expected, "failed to write required number of bytes");
    }
}
