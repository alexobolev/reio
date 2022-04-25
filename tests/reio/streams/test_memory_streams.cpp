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

        CHECK( stream.position() == 0 );
        CHECK( stream.length() == junk.size() );
    }

    SECTION( "from owning_buffer by moving from it" )
    {
        // this will also copy the data, but only due to test structure
        weak_buffer view{ junk.data(), junk.size() };
        owning_buffer buffer{ view };
        memory_input_stream stream{ std::move(buffer) };

        CHECK( stream.position() == 0 );
        CHECK( stream.length() == junk.size() );

        CHECK( buffer.data() == nullptr );
        CHECK( buffer.length() == 0 );
    }

    SECTION( "by move-constructor" )
    {
        memory_input_stream origin{ weak_buffer{ junk.data(), junk.size() } };
        origin.seek_begin(3);

        memory_input_stream target{ std::move(origin) };

        CHECK( target.position() == 3 );
        CHECK( target.length() == junk.size() );

        CHECK( origin.position() == 0 );
        CHECK( origin.length() == 0 );
    }

    SECTION( "by move-assignment" )
    {
        memory_input_stream origin{ weak_buffer{ junk.data(), junk.size() } };
        origin.seek_begin(3);

        memory_input_stream target = std::move(origin);

        CHECK( target.position() == 3 );
        CHECK( target.length() == junk.size() );

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
        uint32_t data = 0u;

        const auto buffer = weak_buffer{ reinterpret_cast<byte*>(&data), 4u };
        const auto read = stream.read_bytes(buffer);

        CHECK( read == 4u );
        CHECK( data == 0x4030201 );

        CHECK_THROWS_AS( stream.read_bytes(weak_buffer{ nullptr, 4u }), io_exception );
        CHECK_THROWS_AS( stream.read_bytes(weak_buffer{ buffer.data(), 0u }), io_exception );
    }

    SECTION( "of single bytes" )
    {
        CHECK( 1 == stream.read_byte() );
        CHECK( 1 == stream.read_byte() );
        CHECK( 3 == stream.read_byte() );
        CHECK( 4 == stream.read_byte() );
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
    }
}





//TEST_CASE( "memory output stream can be initialized", "[streams][memory_streams][output_streams]" ) { }
//TEST_CASE( "memory output stream can be seeked", "[streams][memory_streams][output_streams]" ) { }
//TEST_CASE( "memory output stream serializes primitives", "[streams][memory_streams][output_streams]" ) { }
