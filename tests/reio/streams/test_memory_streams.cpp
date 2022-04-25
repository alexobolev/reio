#include <algorithm>
#include <array>

#include "reio/streams/memory_streams.hpp"
using namespace reio;


TEST_CASE( "memory input stream can be initialized", "[streams][memory_streams][input_streams]" )
{
    std::array<uint8_t, 19u> junk = { 0u };

    SECTION( "from memory view" )
    {
        // this will copy the data using underlying owning_buffer logic
        weak_buffer view{ junk.data(), junk.size() };
        memory_input_stream stream{ view };

        CHECK( stream.capacity() >= junk.size() );
        CHECK( stream.position() == 0 );
        CHECK( stream.length() == junk.size() );
    }

    SECTION( "from owning_buffer by moving from it" )
    {
        // this will also copy the data, but only due to test structure
        weak_buffer view{ junk.data(), junk.size() };
        owning_buffer buffer{ view };
        memory_input_stream stream{ std::move(buffer) };

        CHECK( stream.capacity() >= junk.size() );
        CHECK( stream.position() == 0 );
        CHECK( stream.length() == junk.size() );

        CHECK( buffer.capacity() == 0 );
        CHECK( buffer.data() == nullptr );
        CHECK( buffer.length() == 0 );
    }

    SECTION( "by move-constructor" )
    {
        memory_input_stream origin{ weak_buffer{ junk.data(), junk.size() } };
        origin.seek_begin(3);

        memory_input_stream target{ std::move(origin) };

        CHECK( target.capacity() >= junk.size() );
        CHECK( target.position() == 3 );
        CHECK( target.length() == junk.size() );

        CHECK( origin.capacity() == 0 );
        CHECK( origin.position() == 0 );
        CHECK( origin.length() == 0 );
    }

    SECTION( "by move-assignment" )
    {
        memory_input_stream origin{ weak_buffer{ junk.data(), junk.size() } };
        origin.seek_begin(3);

        memory_input_stream target = std::move(origin);

        CHECK( target.capacity() >= junk.size() );
        CHECK( target.position() == 3 );
        CHECK( target.length() == junk.size() );

        CHECK( origin.capacity() == 0 );
        CHECK( origin.position() == 0 );
        CHECK( origin.length() == 0 );
    }
}


TEST_CASE( "memory input stream can be seeked", "[streams][memory_streams][input_streams]" )
{
    std::array<uint8_t, 19u> junk = { 0u };
    memory_input_stream stream{ weak_buffer{ junk.data(), junk.size() } };

    SECTION( "from the beginning" )
    {
        stream.seek_begin(0);
        CHECK( stream.position() == 0 );

        stream.seek_begin(3);
        CHECK( stream.position() == 3 );

        CHECK_THROWS_AS( stream.seek_begin(-1), io_exception );
        CHECK_THROWS_AS( stream.seek_begin(100), io_exception );
    }

    SECTION( "from the current position" )
    {
        stream.seek_current(0);
        CHECK( stream.position() == 0 );

        stream.seek_current(10);
        CHECK( stream.position() == 10 );

        stream.seek_current(5);
        CHECK( stream.position() == 15 );

        stream.seek_current(-12);
        CHECK( stream.position() == 3 );

        CHECK_THROWS_AS( stream.seek_begin(-100), io_exception );
        CHECK_THROWS_AS( stream.seek_begin(100), io_exception );
    }

    SECTION( "from the end" )
    {
        stream.seek_end(0);
        CHECK( stream.position() == 19u );

        stream.seek_end(-5);
        CHECK( stream.position() == 14u );

        CHECK_THROWS_AS( stream.seek_end(1), io_exception );
        CHECK_THROWS_AS( stream.seek_begin(-100), io_exception );
    }
}


TEST_CASE( "memory input stream deserializes primitives", "[streams][memory_streams][input_streams]" )
{
    std::array<uint8_t, 19u> junk = {
        0x01, 0x02, 0x03, 0x04,
        0x0C,
        0xA8, 0x61,
        0x34, 0x21, 0x6F, 0x7E,
        0x4E, 0xF3, 0x30, 0xA6, 0x4B, 0x9B, 0xB6, 0x01
    };

    memory_input_stream stream{ weak_buffer{ junk.data(), junk.size() } };

    SECTION( "of arbitrary buffers" )
    {
        // normally read a buffer

        uint32_t data = 0u;

        const auto buffer = weak_buffer{ reinterpret_cast<byte*>(&data), 4u };
        const auto read = stream.read_bytes(buffer);

        CHECK( read == 4u );
        CHECK( data == 0x4030201 );
        CHECK( stream.position() == 4 );

        // partially read a buffer which is well beyond the stream's contents

        std::array<byte, 100> data_2 = { 0u };

        const auto buffer_2 = weak_buffer{ data_2.data(), data_2.size() };
        const auto read_2 = stream.read_bytes(buffer_2);

        CHECK( read_2 == 15u );
        CHECK( std::equal(junk.begin() + 4u, junk.end(), data_2.begin(), data_2.begin() + read_2) );

        // try to read into an invalid view

        CHECK_THROWS_AS( stream.read_bytes(weak_buffer{ nullptr, 4u }), io_exception );
        CHECK_THROWS_AS( stream.read_bytes(weak_buffer{ buffer.data(), 0u }), io_exception );
    }

    SECTION( "of arbitrary buffers (without manual error checking)" )
    {
        // normally read a buffer

        uint32_t data = 0u;
        const auto buffer = weak_buffer{ reinterpret_cast<byte*>(&data), 4u };

        CHECK_NOTHROW( stream.read_bytes_or_fail(buffer) );

        CHECK( data == 0x4030201 );
        CHECK( stream.position() == 4 );

        // fail to partially read an overflowing buffer

        byte data_2[100] = { 0u };
        const auto buffer_2 = weak_buffer{ &data_2[0], 100 };

        CHECK_THROWS_AS( stream.read_bytes_or_fail(buffer_2), io_exception );

        // fail to read into an invalid view

        CHECK_THROWS_AS( stream.read_bytes_or_fail(weak_buffer{ nullptr, 4u }), io_exception );
        CHECK_THROWS_AS( stream.read_bytes_or_fail(weak_buffer{ buffer.data(), 0u }), io_exception );
    }

    SECTION( "of single bytes" )
    {
        CHECK( 1 == stream.read_byte() );
        CHECK( 2 == stream.read_byte() );
        CHECK( 3 == stream.read_byte() );
        CHECK( 4 == stream.read_byte() );

        CHECK( stream.position() == 4 );

        stream.seek_end(0);
        CHECK( -1 == stream.read_byte() );
    }

    SECTION( "of numbers in little-endian" )
    {
        stream.seek_begin(4);

        uint8_t num8;
        uint16_t num16;
        uint32_t num32;
        uint64_t num64;

        CHECK( stream.read_numeric<uint8_t, std::endian::little>(num8) );
        CHECK( num8 == 12u );

        CHECK( stream.read_numeric<uint16_t, std::endian::little>(num16) );
        CHECK( num16 == 25000u );

        CHECK( stream.read_numeric<uint32_t, std::endian::little>(num32) );
        CHECK( num32 == 2121212212u );

        CHECK( stream.read_numeric<uint64_t, std::endian::little>(num64) );
        CHECK( num64 == 123456789012345678u );

        CHECK( stream.position() == 19 );

        uint32_t num_junk;

        CHECK( !stream.read_numeric<uint32_t, std::endian::little>(num_junk) );
    }

    SECTION( "of numbers in little-endian (no manual error checking)" )
    {
        stream.seek_begin(4);

        CHECK( stream.read_numeric_or_fail<uint8_t, std::endian::little>() == 12u );
        CHECK( stream.read_numeric_or_fail<uint16_t, std::endian::little>() == 25000u );
        CHECK( stream.read_numeric_or_fail<uint32_t, std::endian::little>() == 2121212212u );
        CHECK( stream.read_numeric_or_fail<uint64_t, std::endian::little>() == 123456789012345678u );

        CHECK( stream.position() == 19 );

        CHECK_THROWS( stream.read_numeric_or_fail<uint32_t, std::endian::little>() );
    }

    SECTION( "of numbers in big-endian" )
    {
        stream.seek_begin(4);

        uint8_t num8;
        uint16_t num16;
        uint32_t num32;
        uint64_t num64;

        CHECK( stream.read_numeric<uint8_t, std::endian::big>(num8) );
        CHECK( num8 == 12u );

        CHECK( stream.read_numeric<uint16_t, std::endian::big>(num16) );
        CHECK( num16 == 43105u );

        CHECK( stream.read_numeric<uint32_t, std::endian::big>(num32) );
        CHECK( num32 == 874606462u );

        CHECK( stream.read_numeric<uint64_t, std::endian::big>(num64) );
        CHECK( num64 == 5688944245090268673u );

        CHECK( stream.position() == 19 );

        uint32_t num_junk;

        CHECK( !stream.read_numeric<uint32_t, std::endian::big>(num_junk) );
    }

    SECTION( "of numbers in big-endian (no manual error checking)" )
    {
        stream.seek_begin(4);

        CHECK( stream.read_numeric_or_fail<uint8_t, std::endian::big>() == 12u );
        CHECK( stream.read_numeric_or_fail<uint16_t, std::endian::big>() == 43105u );
        CHECK( stream.read_numeric_or_fail<uint32_t, std::endian::big>() == 874606462u );
        CHECK( stream.read_numeric_or_fail<uint64_t, std::endian::big>() == 5688944245090268673u );

        CHECK( stream.position() == 19 );

        CHECK_THROWS( stream.read_numeric_or_fail<uint32_t, std::endian::big>() );
    }
}





TEST_CASE( "memory output stream can be initialized", "[streams][memory_streams][output_streams]" )
{
    std::array<uint8_t, 20u> junk = { 0u };
    default_allocator allocator;

    SECTION( "with default-initialization" )
    {
        memory_output_stream stream{};
        CHECK( stream.capacity() == 0 );
        CHECK( stream.position() == 0 );
        CHECK( stream.length() == 0 );

        memory_output_stream stream_2{ &allocator };
        CHECK( stream_2.capacity() == 0 );
        CHECK( stream_2.position() == 0 );
        CHECK( stream_2.length() == 0 );
    }

    SECTION( "with capacity" )
    {
        memory_output_stream stream{ 20 };
        CHECK( stream.capacity() == 20 );
        CHECK( stream.position() == 0 );
        CHECK( stream.length() == 0 );

        memory_output_stream stream_2{ 10, &allocator };
        CHECK( stream_2.capacity() == 10 );
        CHECK( stream_2.position() == 0 );
        CHECK( stream_2.length() == 0 );
    }

    SECTION( "with capacity and growth factor" )
    {
        memory_output_stream stream{ 20, growth_factor::tight };
        CHECK( stream.capacity() == 20 );
        CHECK( stream.growth() == growth_factor::tight );
        CHECK( stream.position() == 0 );
        CHECK( stream.length() == 0 );

        memory_output_stream stream_2{ 10, growth_factor::none, &allocator };
        CHECK( stream_2.capacity() == 10 );
        CHECK( stream_2.growth() == growth_factor::none );
        CHECK( stream_2.position() == 0 );
        CHECK( stream_2.length() == 0 );
    }

    SECTION( "by move-constructor" )
    {
        memory_output_stream origin{ 20, growth_factor::tight };
        memory_output_stream target{ std::move(origin) };

        CHECK( target.capacity() >= junk.size() );
        CHECK( target.growth() == growth_factor::tight );
//        CHECK( target.position() == 0 );
//        CHECK( target.length() == 0 );

        CHECK( origin.capacity() == 0 );
        CHECK( origin.growth() == k_default_growth_factor );
//        CHECK( origin.position() == 0 );
//        CHECK( origin.length() == 0 );
    }

    SECTION( "by move-assignment" )
    {
        memory_output_stream origin{ 20, growth_factor::tight };
        memory_output_stream target = std::move(origin);

        CHECK( target.capacity() >= junk.size() );
        CHECK( target.growth() == growth_factor::tight );
//        CHECK( target.position() == 0 );
//        CHECK( target.length() == 0 );

        CHECK( origin.capacity() == 0 );
        CHECK( origin.growth() == k_default_growth_factor );
//        CHECK( origin.position() == 0 );
//        CHECK( origin.length() == 0 );
    }
}

TEST_CASE( "memory output stream serializes primitives", "[streams][memory_streams][output_streams]" )
{
    std::array<uint8_t, 19u> junk = {
        0x01, 0x02, 0x03, 0x04,
        0x0C,
        0xA8, 0x61,
        0x34, 0x21, 0x6F, 0x7E,
        0x4E, 0xF3, 0x30, 0xA6, 0x4B, 0x9B, 0xB6, 0x01
    };

    // create a fixed-sized output stream which can't expand
    // to properly test write failures
    memory_output_stream stream{ 19, growth_factor::none };

    SECTION( "of arbitrary buffers" )
    {
        // write a small buffer which will fit

        std::array<uint8_t, 4> data{ 0x01, 0x02, 0x03, 0x04 };

        CHECK( 4 == stream.write_bytes(weak_buffer{ reinterpret_cast<byte*>(&data), sizeof data }) );
        CHECK( 4 == stream.position() );

        // write an overflowing buffer

        std::array<uint8_t, 20u> data_2{ 0u };
        std::ranges::generate(data_2, [n = 0]() mutable { return ++n; });

        CHECK( 15 == stream.write_bytes(weak_buffer{ data_2.data(), data_2.size() }) );
        CHECK( 19 == stream.position() );

        const auto expected = std::array<uint8_t, 19>{
            0x01, 0x02, 0x03, 0x04,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
        };
        CHECK( std::ranges::equal(stream.view(), expected) );

        // fail to write beyond underlying buffer's end

        CHECK( 0 == stream.write_bytes(weak_buffer{ data_2.data(), data_2.size() }) );
        CHECK( 19 == stream.position() );

        // fail to write from invalid buffers

        CHECK_THROWS_AS( stream.write_bytes(weak_buffer{ nullptr, 19u }), io_exception );
        CHECK_THROWS_AS( stream.write_bytes(weak_buffer{ data_2.data(), 0u }), io_exception );
    }

    SECTION( "of arbitrary buffers (no manual error checking)" )
    {
        // write a small buffer which will fit

        std::array<uint8_t, 4> data{ 0x01, 0x02, 0x03, 0x04 };

        CHECK_NOTHROW( stream.write_bytes_or_fail(weak_buffer{ reinterpret_cast<byte*>(&data), sizeof data }) );
        CHECK( stream.position() == 4 );

        // write an overflowing buffer

        std::array<uint8_t, 20u> data_2{ 0u };
        std::ranges::generate(data_2, [n = 0]() mutable { return ++n; });

        CHECK_THROWS_AS( stream.write_bytes_or_fail(weak_buffer{ data_2.data(), data_2.size() }), io_exception );

        // fail to write from invalid buffers

        CHECK_THROWS_AS( stream.write_bytes_or_fail(weak_buffer{ nullptr, 19u }), io_exception );
        CHECK_THROWS_AS( stream.write_bytes_or_fail(weak_buffer{ data_2.data(), 0u }), io_exception );
    }

    SECTION( "of single bytes" )
    {
        CHECK( stream.write_byte(1u) );
        CHECK( stream.write_byte(2u) );

        for (auto i = 0u; i < 17; i++)
        {
            const auto ok = stream.write_byte(0xDD);
            if (!ok)
            {
                FAIL( "internal test logic error: no reason for write_byte to fail here" );
            }
        }

        CHECK( !stream.write_byte(20u) );
    }

    SECTION( "of numbers in little-endian" )
    {
        if (!stream.write_numeric<uint32_t>(0))
        {
            FAIL( "internal test logic error: no reason for write_numeric to fail here" );
        }

        CHECK( stream.write_numeric<uint8_t, std::endian::little>(12u) );
        CHECK( stream.write_numeric<uint16_t, std::endian::little>(25000u) );
        CHECK( stream.write_numeric<uint32_t, std::endian::little>(2121212212u) );
        CHECK( stream.write_numeric<uint64_t, std::endian::little>(123456789012345678u) );

        CHECK( stream.position() == 19 );
        CHECK( std::ranges::equal(stream.view(), std::array<uint8_t, 19u>{
            0x00, 0x00, 0x00, 0x00,
            0x0C,
            0xA8, 0x61,
            0x34, 0x21, 0x6F, 0x7E,
            0x4E, 0xF3, 0x30, 0xA6, 0x4B, 0x9B, 0xB6, 0x01
        }) );

        CHECK( !stream.write_numeric<uint16_t, std::endian::little>(1u) );
    }

    SECTION( "of numbers in little-endian (no manual error checking)" )
    {
        stream.write_numeric_or_fail<uint32_t>(0);

        CHECK_NOTHROW( stream.write_numeric_or_fail<uint8_t, std::endian::little>(12u) );
        CHECK_NOTHROW( stream.write_numeric_or_fail<uint16_t, std::endian::little>(25000u) );
        CHECK_NOTHROW( stream.write_numeric_or_fail<uint32_t, std::endian::little>(2121212212u) );
        CHECK_NOTHROW( stream.write_numeric_or_fail<uint64_t, std::endian::little>(123456789012345678u) );

        CHECK( stream.position() == 19 );
        CHECK( std::ranges::equal(stream.view(), std::array<uint8_t, 19u>{
                0x00, 0x00, 0x00, 0x00,
                0x0C,
                0xA8, 0x61,
                0x34, 0x21, 0x6F, 0x7E,
                0x4E, 0xF3, 0x30, 0xA6, 0x4B, 0x9B, 0xB6, 0x01
        }) );

        CHECK_THROWS( stream.write_numeric_or_fail<uint16_t, std::endian::little>(1u) );
    }

    SECTION( "of numbers in big-endian" )
    {
        if (!stream.write_numeric<uint32_t>(0))
        {
            FAIL( "internal test logic error: no reason for write_numeric to fail here" );
        }

        CHECK( stream.write_numeric<uint8_t, std::endian::big>(12u) );
        CHECK( stream.write_numeric<uint16_t, std::endian::big>(43105u) );
        CHECK( stream.write_numeric<uint32_t, std::endian::big>(874606462u) );
        CHECK( stream.write_numeric<uint64_t, std::endian::big>(5688944245090268673u) );

        CHECK( stream.position() == 19 );
        CHECK( std::ranges::equal(stream.view(), std::array<uint8_t, 19u>{
                0x00, 0x00, 0x00, 0x00,
                0x0C,
                0xA8, 0x61,
                0x34, 0x21, 0x6F, 0x7E,
                0x4E, 0xF3, 0x30, 0xA6, 0x4B, 0x9B, 0xB6, 0x01
        }) );

        CHECK( !stream.write_numeric<uint16_t, std::endian::big>(1u) );
    }

    SECTION( "of numbers in big-endian (no manual error checking)" )
    {
        stream.write_numeric_or_fail<uint32_t>(0);

        CHECK_NOTHROW( stream.write_numeric_or_fail<uint8_t, std::endian::big>(12u) );
        CHECK_NOTHROW( stream.write_numeric_or_fail<uint16_t, std::endian::big>(43105u) );
        CHECK_NOTHROW( stream.write_numeric_or_fail<uint32_t, std::endian::big>(874606462u) );
        CHECK_NOTHROW( stream.write_numeric_or_fail<uint64_t, std::endian::big>(5688944245090268673u) );

        CHECK( stream.position() == 19 );
        CHECK( std::ranges::equal(stream.view(), std::array<uint8_t, 19u>{
                0x00, 0x00, 0x00, 0x00,
                0x0C,
                0xA8, 0x61,
                0x34, 0x21, 0x6F, 0x7E,
                0x4E, 0xF3, 0x30, 0xA6, 0x4B, 0x9B, 0xB6, 0x01
        }) );

        CHECK_THROWS( stream.write_numeric_or_fail<uint16_t, std::endian::big>(1u) );
    }

}

TEST_CASE( "memory output stream can be seeked", "[streams][memory_streams][output_streams]" )
{
    std::array<uint8_t, 19u> junk{ 0u };
    memory_output_stream stream{ 19u, growth_factor::none };

    const auto write_buffer = weak_buffer{ junk.data(), junk.size() };
    stream.write_bytes_or_fail(write_buffer);

    SECTION( "from the beginning" )
    {
        stream.seek_begin(0);
        CHECK( stream.position() == 0 );

        stream.seek_begin(3);
        CHECK( stream.position() == 3 );

        CHECK_THROWS_AS( stream.seek_begin(-1), io_exception );
        CHECK_THROWS_AS( stream.seek_begin(100), io_exception );
    }

    SECTION( "from the current position" )
    {
        stream.seek_current(0);
        CHECK( stream.position() == 19 );

        stream.seek_current(-10);
        CHECK( stream.position() == 9 );

        stream.seek_current(5);
        CHECK( stream.position() == 14 );

        stream.seek_current(-12);
        CHECK( stream.position() == 2 );

        CHECK_THROWS_AS( stream.seek_begin(-100), io_exception );
        CHECK_THROWS_AS( stream.seek_begin(100), io_exception );
    }

    SECTION( "from the end" )
    {
        stream.seek_end(0);
        CHECK( stream.position() == 19u );

        stream.seek_end(-5);
        CHECK( stream.position() == 14u );

        CHECK_THROWS_AS( stream.seek_end(1), io_exception );
        CHECK_THROWS_AS( stream.seek_begin(-100), io_exception );
    }
}
