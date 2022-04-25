#ifndef REIO_STREAMS_HPP
#define REIO_STREAMS_HPP

#include "../asserts.hpp"
#include "../types.hpp"
#include "../buffers/weak_buffer.hpp"


namespace reio
{
    enum struct seek_origin : int
    {
        begin = 1,
        current = 2,
        end = 3
    };


    class base_stream
    {
    public:

        virtual ~base_stream() noexcept = default;
        virtual int64_t position() = 0;
        virtual int64_t length() = 0;
        virtual void seek_begin(int64_t offset) = 0;
        virtual void seek_current(int64_t offset) = 0;
        virtual void seek_end(int64_t offset) = 0;
    };


    ///
    /// @brief      Base interface and common logic for stateful
    ///             abstractions of primitive deserialization.
    ///
    class input_stream : public base_stream
    {
    public:

        ///
        /// @brief      Get up to a certain number of bytes from the data source and advance internal cursor.
        /// @param      output    View of memory to read into; defines the number of bytes to read.
        /// @return     Number of bytes successfully read.
        ///
        virtual int64_t read_bytes(weak_buffer output) = 0;

        ///
        /// @brief      Get a single byte from the underlying data source and advance internal cursor.
        ///
        /// By default, it is implemented as a call to @c read_bytes with a single-byte view,
        /// but that might be not the most efficient way of retrieving a single byte.
        ///
        /// @return     Byte value on success, @c -1 on failure.
        ///
        virtual int64_t read_byte();

        ///
        /// @brief      Get a fixed number of bytes, and hard-fail if not enough is available.
        /// @param      output    View of memory to read into; defines the number of bytes to read.
        ///
        void read_bytes_or_fail(weak_buffer output);

        ///
        /// @brief      Read a numeric value from stream, doing endianness conversion if needed.
        /// @tparam     T       Type of the numeric value to read.
        /// @tparam     E       Endianness which was used to encode the value.
        /// @param      out    Value reference which should receive the read number.
        /// @return     Whether a sufficient number of bytes was read.
        ///
        template<numeric_type T, std::endian E = std::endian::native>
        bool read_numeric(T& out)
        {
            const auto view = weak_buffer{ reinterpret_cast<byte*>(&out), sizeof(T) };
            const auto read = read_bytes(view);

            if (read != sizeof(T))
            {
                return false;
            }

            if constexpr (E != std::endian::native)
            {
                out = bswap(out);
            }

            return true;
        }

        ///
        /// @brief      Read a numeric of value from stream, doing endianness conversion if needed.
        ///
        /// An alternative (wrapper) for @c read_numeric which can be used
        /// when you can't be bothered to do manual error checking.
        ///
        /// @tparam     T               Type of the numeric value to read.
        /// @tparam     E               Endianness which was used to encode the value.
        /// @throws     io_exception    If there isn't enough bytes.
        ///
        /// @return     Value of a requested type.
        ///
        template<numeric_type T, std::endian E = std::endian::native>
        T read_numeric_or_fail()
        {
            auto value = T{ 0u };
            auto success = read_numeric<T, E>(value);

            REIO_ASSERT(success, "failed to read enough bytes for a numeric value");
            return value;
        }
    };


    ///
    /// @brief      Base interface and common logic for stateful
    ///             abstractions of primitive serialization.
    ///
    class output_stream : public base_stream
    {
    public:

        ///
        /// @brief      Put up to a number of bytes into the underlying data sink and advance internal cursor.
        /// @param      input    View of memory to write; implicitly defines the written number of bytes.
        /// @return     Number of bytes successfully written.
        ///
        virtual int64_t write_bytes(weak_buffer input) = 0;

        ///
        /// @brief      Put a single byte into the underlying data sink and advance internal cursor.
        ///
        /// Much like @c input_stream::read_byte, this falls back to write_bytes with a single-byte
        /// input buffer by default, but also leaves custom implementations with an ability
        /// to use a more efficient strategy if possible.
        ///
        /// @param      value    Byte to write.
        /// @return     Whether the byte was successfully written.
        ///
        virtual bool write_byte(byte value);

        ///
        /// @brief      Write a fixed number of bytes to stream, and hard-fail if not all are written.
        ///
        /// @param      input           View of memory to write; implicitly defines the written number of bytes.
        /// @throw      io_exception    If insufficient number of bytes was written.
        ///
        void write_bytes_or_fail(weak_buffer input);

        ///
        /// @brief      Write a numeric value onto stream, doing endianness conversion if needed.
        ///
        /// @tparam     T       Type of the numeric value to write.
        /// @tparam     E       Endianness which is expected to be used to decode the value.
        /// @param      value   Written number.
        ///
        /// @return     Whether the value was written onto stream in full.
        ///
        template<numeric_type T, std::endian E = std::endian::native>
        bool write_numeric(T value)
        {
            if constexpr (E != std::endian::native)
            {
                value = bswap(value);
            }

            const auto buffer = weak_buffer{ reinterpret_cast<byte*>(&value), sizeof(T) };
            const auto written = write_bytes(buffer);

            return written == sizeof(T);
        }

        ///
        /// @brief      Write a numeric value onto stream, doing endianness conversion if needed.
        ///
        /// Alternative to (wrapper of) the @c output_stream::write_numeric
        /// which spares the programmer from necessity to do manual error checking.
        ///
        /// @tparam     T       Type of the numeric value to write.
        /// @tparam     E       Endianness which is expected to be used to decode the value.
        /// @param      value   Written number.
        ///
        template<numeric_type T, std::endian E = std::endian::native>
        void write_numeric_or_fail(T value)
        {
            const auto success = write_numeric<T, E>(value);
            REIO_ASSERT(success, "failed to write enough bytes for a numeric type");
        }

    };

}

#endif //REIO_STREAMS_HPP
