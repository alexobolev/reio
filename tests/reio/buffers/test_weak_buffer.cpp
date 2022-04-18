#include <algorithm>
#include <array>

#include "reio/buffers/weak_buffer.hpp"
using namespace reio;


TEST_CASE( "weak buffer can be initialized", "[buffer][weak_buffer]" ) {

    std::uint32_t junk[4] = { 1u, 2u, 3u, 4u };
    const auto junk_ptr = reinterpret_cast<byte*>(&junk[0]);
    const auto junk_len = 4 * sizeof *junk;

    SECTION( "it is empty after default initialization" ) {
        weak_buffer view{};
        CHECK( view.data() == nullptr );
        CHECK( view.length() == 0u );
    }

    SECTION( "it has correct length and pointer after data initialization" ) {
        weak_buffer view{ junk_ptr , junk_len };
        CHECK( view.data() == junk_ptr );
        CHECK( view.length() == junk_len );
    }

}


TEST_CASE( "weak buffer supports copy/move semantics", "[buffer][weak_buffer]" ) {

    std::uint32_t junk[4] = { 1u, 2u, 3u, 4u };
    const auto junk_ptr = reinterpret_cast<byte*>(&junk[0]);
    const auto junk_len = 4 * sizeof *junk;

    SECTION( "it can be copied-constructed correctly" ) {
        weak_buffer source{ junk_ptr, junk_len };
        weak_buffer target{ source };

        CHECK( source.data() == junk_ptr );
        CHECK( source.length() == junk_len );

        CHECK( target.data() == junk_ptr );
        CHECK( target.length() == junk_len );
    }

    SECTION( "it can be copied-assigned correctly" ) {
        weak_buffer source{ junk_ptr, junk_len };
        weak_buffer target = source;

        CHECK( source.data() == junk_ptr );
        CHECK( source.length() == junk_len );

        CHECK( target.data() == junk_ptr );
        CHECK( target.length() == junk_len );
    }

    SECTION( "it can be move-constructed correctly" ) {
        weak_buffer source{ junk_ptr, junk_len };
        weak_buffer target{ std::move(source) };

        CHECK( source.data() == nullptr );  // NOLINT(bugprone-use-after-move)
        CHECK( source.length() == 0u );

        CHECK( target.data() == junk_ptr );
        CHECK( target.length() == junk_len );
    }

    SECTION( "it can be move-assigned correctly" ) {
        weak_buffer source{ junk_ptr, junk_len };
        weak_buffer target = std::move(source);

        CHECK( source.data() == nullptr );  // NOLINT(bugprone-use-after-move)
        CHECK( source.length() == 0u );

        CHECK( target.data() == junk_ptr );
        CHECK( target.length() == junk_len );
    }

}


TEST_CASE( "weak buffer has iterator methods", "[buffer][weak_buffer]" ) {

    std::array<std::uint8_t, 20> junk{ 0u };
    std::ranges::generate(junk, [n = 0]() mutable { return ++n; });
    weak_buffer buffer{ junk.data(), junk.size() };

    SECTION( "has begin iterator method" ) {

        CHECK( *buffer.begin() + 00 == 01u );
        CHECK( *buffer.begin() + 01 == 02u );
        CHECK( *buffer.begin() + 19 == 20u );

    }

    SECTION( "has end iterator method" ) {

        CHECK( buffer.end() == buffer.begin() + buffer.length() );

        CHECK( *(buffer.end() - 01) == 20u );
        CHECK( *(buffer.end() - 02) == 19u );
        CHECK( *(buffer.end() - 20) == 01u );

    }

    SECTION( "can be used in range-for" ) {

        std::size_t i = 0u;

        for (auto b : buffer) {
            CHECK( b == ++i );
        }

    }

}


TEST_CASE( "weak buffer provides byte access", "[buffer][weak_buffer]" ) {

    std::array<std::uint8_t, 32> junk{ 0u };
    std::ranges::generate(junk, [n = 0]() mutable { return ++n; });
    weak_buffer buffer{ junk.data(), junk.size() };

    SECTION( "using subscript operator" ) {

        CHECK( buffer[0] == 1 );
        CHECK( buffer[1] == 2 );
        CHECK( buffer[10] == 11 );
        CHECK( buffer[25] == 26 );
        CHECK( buffer[31] == 32 );

        CHECK_NOTHROW( buffer[32] );
        CHECK_NOTHROW( buffer[100] );

    }

    SECTION( "using subscript method" ) {

        CHECK( buffer.at(0) == 1 );
        CHECK( buffer.at(1) == 2 );
        CHECK( buffer.at(10) == 11 );
        CHECK( buffer.at(25) == 26 );
        CHECK( buffer.at(31) == 32 );

        #ifndef REIO_OPTION_NO_ASSERTS

        CHECK_THROWS_AS( buffer.at(32), io_exception );
        CHECK_THROWS_AS( buffer.at(60), io_exception );

        #endif

    }

}


TEST_CASE( "weak buffer provides partial access", "[buffer][weak_buffer]" ) {

    std::array<std::uint8_t, 10> junk{ 0u };
    std::ranges::generate(junk, [n = 0]() mutable { return ++n; });
    weak_buffer buffer{ junk.data(), junk.size() };

    SECTION( "using full subview" ) {

        weak_buffer view = buffer.view();

        CHECK( view.data() == buffer.data() );
        CHECK( view.length() == buffer.length() );

    }

    SECTION( "using partial subview" ) {

        weak_buffer sv1 = buffer.subview(0, 5);
        CHECK( sv1.data() == buffer.data() );
        CHECK( sv1.length() == 5 );

        weak_buffer sv2 = buffer.subview(2, 2);
        CHECK( sv2.data() == buffer.data() + 2 );
        CHECK( sv2.length() == 2 );

        weak_buffer sv3 = buffer.subview(7, 3);
        CHECK( sv3.data() == buffer.data() + 7 );
        CHECK( sv3.length() == 3 );

        CHECK( sv3[0] == 8u );
        CHECK( sv3[1] == 9u );
        CHECK( sv3[2] == 10u );

        CHECK_THROWS_AS( buffer.subview(0, 11), io_exception );
        CHECK_THROWS_AS( buffer.subview(13, 2), io_exception );

    }

    SECTION( "using starting subview" ) {

        weak_buffer view = buffer.first(3);

        CHECK( view.data() == buffer.data() );
        CHECK( view.length() == 3 );

        CHECK( view[0] == 1 );
        CHECK( view[1] == 2 );
        CHECK( view[2] == 3 );

        CHECK_THROWS_AS( view.first(100), io_exception );

    }

    SECTION( "using trailing subview" ) {

        weak_buffer view = buffer.last(3);

        CHECK( view.data() == buffer.data() + 7 );
        CHECK( view.length() == 3 );

        CHECK( view[0] == 8 );
        CHECK( view[1] == 9 );
        CHECK( view[2] == 10 );

        CHECK_THROWS_AS( view.last(12), io_exception );

    }

    SECTION( "using trailing (from position) subview" ) {

        weak_buffer view = buffer.last_from(6);

        CHECK( view.data() == buffer.data() + 6 );
        CHECK( view.length() == 4 );

        CHECK( view[0] == 7 );
        CHECK( view[1] == 8 );
        CHECK( view[2] == 9 );
        CHECK( view[3] == 10 );

        CHECK_THROWS_AS( view.last_from(22), io_exception );

    }

}


TEST_CASE( "weak buffer can be overwritten", "[buffer][weak_buffer]" ) {

    std::array<std::uint8_t, 10> junk{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };
    std::array<std::uint8_t, 12> repl{ 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u, 32u };

    weak_buffer buffer{ junk.data(), junk.size() };

    const auto old_data = buffer.data();
    const auto old_length = buffer.length();


    SECTION( "but destination iterator must be within the buffer" ) {

        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.begin() + 3, buffer.cend() + 1), io_exception );
        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.begin() + 3, buffer.cbegin() - 1), io_exception );
        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.begin() + 3, nullptr), io_exception );

    }

    SECTION( "but source iterators must be ordered" ) {

        CHECK_THROWS_AS( buffer.overwrite(repl.end(), repl.begin(), buffer.cbegin()), io_exception );
        CHECK_THROWS_AS( buffer.overwrite(repl.begin() + 5, repl.begin() + 2, buffer.cbegin()), io_exception );

    }

    SECTION( "but there must be enough space" ) {

        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.end(), buffer.cbegin()), io_exception );
        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.end(), buffer.cbegin() + 3), io_exception );
        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.end(), buffer.cend()), io_exception );

    }

    SECTION( "from the start" ) {

        // empty write:

        auto end_0 = buffer.overwrite(repl.begin(), repl.begin(), buffer.begin());
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.begin() );

        // partial write:

        auto end_1 = buffer.overwrite(repl.begin(), repl.begin() + 3u, buffer.begin());
        auto result_1 = std::array{ 21u, 22u, 23u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_1) );
        CHECK( end_1 == buffer.begin() + 3u );

        // full write:

        auto end_2 = buffer.overwrite(repl.begin(), repl.begin() + 10u, buffer.begin());
        auto result_2 = std::array{ 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 30u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_2) );
        CHECK( end_2 == buffer.end() );

        // overflowing write:

        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.end(), buffer.begin()), io_exception );

    }

    SECTION( "from the middle" ) {

        // empty write:

        auto end_0 = buffer.overwrite(repl.begin(), repl.begin(), buffer.begin() + 4u);
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.begin() + 4u );

        // partial write:

        auto end_1 = buffer.overwrite(repl.begin(), repl.begin() + 3u, buffer.begin() + 4u);
        auto result_1 = std::array{ 1u, 2u, 3u, 4u, 21u, 22u, 23u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_1) );
        CHECK( end_1 == buffer.begin() + 7u );

        // full write:

        auto end_2 = buffer.overwrite(repl.begin(), repl.begin() + 6u, buffer.begin() + 4u);
        auto result_2 = std::array{ 1u, 2u, 3u, 4u, 21u, 22u, 23u, 24u, 25u, 26u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_2) );
        CHECK( end_2 == buffer.end() );

        // overflowing write:

        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.begin() + 7u, buffer.begin() + 4u), io_exception );

    }

    SECTION( "from the end" ) {

        // empty write:

        auto end_0 = buffer.overwrite(repl.begin(), repl.begin(), buffer.end());
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.end() );

        // overflowing write:

        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.begin() + 1u, buffer.end()), io_exception );

    }

}


TEST_CASE( "weak buffer can be inserted into", "[buffer][weak_buffer]" ) {

    std::array<std::uint8_t, 10> junk{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };
    std::array<std::uint8_t, 12> repl{ 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u, 32u };

    weak_buffer buffer{ junk.data(), junk.size() };

    const auto old_data = buffer.data();
    const auto old_length = buffer.length();


    SECTION( "but destination iterator must be within the buffer" ) {

        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.begin() + 3, buffer.cend() + 1), io_exception );
        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.begin() + 3, buffer.cbegin() - 1), io_exception );
        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.begin() + 3, nullptr), io_exception );

    }

    SECTION( "but source iterators must be ordered" ) {

        CHECK_THROWS_AS( buffer.insert(repl.end(), repl.begin(), buffer.cbegin()), io_exception );
        CHECK_THROWS_AS( buffer.insert(repl.begin() + 5, repl.begin() + 2, buffer.cbegin()), io_exception );

    }

    SECTION( "but there must be enough space" ) {

        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.end(), buffer.cbegin()), io_exception );
        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.end(), buffer.cbegin() + 3), io_exception );
        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.end(), buffer.cend()), io_exception );

    }

    SECTION( "at the start" ) {

        // empty write:

        auto end_0 = buffer.insert(repl.begin(), repl.begin(), buffer.begin());
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.begin() );

        // partial write:

        auto end_1 = buffer.insert(repl.begin(), repl.begin() + 3u, buffer.begin());
        auto result_1 = std::array{ 21u, 22u, 23u, 1u, 2u, 3u, 4u, 5u, 6u, 7u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_1) );
        CHECK( end_1 == buffer.begin() + 3u );

        // full write:

        auto end_2 = buffer.insert(repl.begin(), repl.begin() + 10u, buffer.begin());
        auto result_2 = std::array{ 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 30u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_2) );
        CHECK( end_2 == buffer.end() );

        // overflowing write:

        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.end(), buffer.begin()), io_exception );

    }

    SECTION( "at the middle" ) {

        // empty write:

        auto end_0 = buffer.insert(repl.begin(), repl.begin(), buffer.begin() + 4u);
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.begin() + 4u );

        // partial write:

        auto end_1 = buffer.insert(repl.begin(), repl.begin() + 3u, buffer.begin() + 4u);
        auto result_1 = std::array{ 1u, 2u, 3u, 4u, 21u, 22u, 23u, 5u, 6u, 7u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_1) );
        CHECK( end_1 == buffer.begin() + 7u );

        // full write:

        auto end_2 = buffer.insert(repl.begin(), repl.begin() + 6u, buffer.begin() + 4u);
        auto result_2 = std::array{ 1u, 2u, 3u, 4u, 21u, 22u, 23u, 24u, 25u, 26u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_2) );
        CHECK( end_2 == buffer.end() );

        // overflowing write:

        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.begin() + 7u, buffer.begin() + 4u), io_exception );

    }

    SECTION( "at the end" ) {

        // empty write:

        auto end_0 = buffer.insert(repl.begin(), repl.begin(), buffer.end());
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.end() );

        // overflowing write:

        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.begin() + 1u, buffer.end()), io_exception );

    }

}
