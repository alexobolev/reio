#include "reio/types.hpp"
using namespace reio;


TEST_CASE( "bswap swaps unsigned integers" ) {

    SECTION( "8-bits remain the same" ) {
        CHECK( bswap<uint8_t>(0x00) == 0x00 );
        CHECK( bswap<uint8_t>(0x32) == 0x32 );
        CHECK( bswap<uint8_t>(0xFF) == 0xFF );
    }

    SECTION( "16-bits swap correctly" ) {
        CHECK( bswap<uint16_t>(0x0000) == 0x0000 );
        CHECK( bswap<uint16_t>(0x1234) == 0x3412 );
        CHECK( bswap<uint16_t>(0xFFFF) == 0xFFFF );
    }

    SECTION( "32-bits swap correctly" ) {
        CHECK( bswap<uint32_t>(0x00000000) == 0x00000000 );
        CHECK( bswap<uint32_t>(0x89ABCDEF) == 0xEFCDAB89 );
        CHECK( bswap<uint32_t>(0xFFFFFFFF) == 0xFFFFFFFF );
    }

    SECTION( "64-bits swap correctly" ) {
        CHECK( bswap<uint64_t>(0x0000000000000000) == 0x0000000000000000 );
        CHECK( bswap<uint64_t>(0x1234567890ABCDEF) == 0xEFCDAB9078563412 );
        CHECK( bswap<uint64_t>(0xFFFFFFFFFFFFFFFF) == 0xFFFFFFFFFFFFFFFF );
    }

}


TEST_CASE( "bswap swaps signed integers" ) {

    SECTION( "8-bits remain the same" ) {
        CHECK( bswap<int8_t>(0) == 0 );
        CHECK( bswap<int8_t>(-32) == -32 );
        CHECK( bswap<int8_t>(127) == 127 );
    }

    SECTION( "16-bits swap correctly" ) {
        CHECK( bswap<int16_t>(0) == 0 );
        CHECK( bswap<int16_t>(-1) == -1 );
        CHECK( bswap<int16_t>(-12345) == -14385 );
        CHECK( bswap<int16_t>(32767) == -129 );
    }

    SECTION( "32-bits swap correctly" ) {
        CHECK( bswap<int32_t>(0) == 0 );
        CHECK( bswap<int32_t>(-1) == -1 );
        CHECK( bswap<int32_t>(-33532734) == -1034682114 );
        CHECK( bswap<int32_t>(2147483647) == -129 );
    }

    SECTION( "64-bits swap correctly" ) {
        CHECK( bswap<int64_t>(0) == 0 );
        CHECK( bswap<int64_t>(-1) == -1 );
        CHECK( bswap<uint64_t>(72168265475350669) == -8223372036854775807 );
        CHECK( bswap<uint64_t>(9223372036854775807) == -129 );
    }

}
