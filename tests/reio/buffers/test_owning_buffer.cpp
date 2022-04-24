#include <algorithm>
#include <array>

#include "reio/buffers/owning_buffer.hpp"
#include "reio/buffers/weak_buffer.hpp"
using namespace reio;


TEST_CASE( "owning buffer can be initialized", "[buffer][owning_buffer]" ) {

    std::uint32_t junk[4] = { 1u, 2u, 3u, 4u };
    const auto junk_ptr = reinterpret_cast<byte*>(&junk[0]);
    const auto junk_len = 4 * sizeof *junk;

    default_allocator alloc{};

    SECTION( "it is empty after default initialization" ) {
        owning_buffer buffer{ &alloc };
        CHECK( buffer.length() == 0 );
        CHECK( buffer.capacity() == 0 );
        CHECK( buffer.data() == nullptr );
        CHECK( buffer.growth() == k_default_growth_factor );
        CHECK( buffer.allocator() == &alloc );
    }

    SECTION( "it is empty but has capacity after preallocating initialization" ) {
        owning_buffer buffer{ 0x20, &alloc };
        CHECK( buffer.length() == 0 );
        CHECK( buffer.capacity() == 0x20 );
        CHECK( buffer.data() != nullptr );
        CHECK( buffer.growth() == k_default_growth_factor );
        CHECK( buffer.allocator() == &alloc );
    }

    SECTION( "it has the correct contents after fill initialization" ) {
        owning_buffer buffer{ 0x20, 2u, &alloc };
        CHECK( buffer.length() == 0x20 );
        CHECK( buffer.capacity() >= 0x20 );
        CHECK( buffer.data() != nullptr );
        CHECK( buffer.growth() == k_default_growth_factor );
        CHECK( std::ranges::all_of(buffer, [](byte b) { return b == 2u; }) );
        CHECK( buffer.allocator() == &alloc );
    }

    SECTION( "it has the correct contents after memcopy initialization" ) {
        owning_buffer buffer{ weak_buffer{ junk_ptr, junk_len }, &alloc };
        CHECK( buffer.length() == junk_len );
        CHECK( buffer.capacity() >= junk_len );
        CHECK( buffer.data() != nullptr );
        CHECK( buffer.data() != junk_ptr );
        CHECK( buffer.growth() == k_default_growth_factor );
        CHECK( buffer.allocator() == &alloc );

        const auto intarrptr = reinterpret_cast<std::uint32_t*>(buffer.data());
        CHECK( intarrptr[0] == 1u );
        CHECK( intarrptr[1] == 2u );
        CHECK( intarrptr[2] == 3u );
        CHECK( intarrptr[3] == 4u );
    }

}


TEST_CASE( "owning buffer supports move semantics", "[buffer][owning_buffer]" ) {

    std::array<std::uint32_t, 4> junk = { 1u, 2u, 3u, 4u };
    const auto junk_ptr = reinterpret_cast<byte*>(&junk[0]);
    const auto junk_len = 4 * sizeof junk.front();

    default_allocator alloc{};

    SECTION( "it can be move-constructed correctly" ) {
        owning_buffer source{ weak_buffer{ junk_ptr, junk_len }, &alloc };
        owning_buffer target{ std::move(source) };

        CHECK( source.data() == nullptr ); // NOLINT(bugprone-use-after-move)
        CHECK( source.length() == 0u );
        CHECK( source.capacity() == 0u );
        CHECK( source.growth() == k_default_growth_factor );
        CHECK( source.allocator() == nullptr );


        CHECK( target.length() == junk_len );
        CHECK( target.capacity() >= junk_len );
        CHECK( target.data() != nullptr );
        CHECK( target.data() != junk_ptr );
        CHECK( target.growth() == k_default_growth_factor );
        CHECK( target.allocator() == &alloc );

        const auto intarrptr = reinterpret_cast<std::uint32_t*>(target.data());
        CHECK( intarrptr[0] == 1u );
        CHECK( intarrptr[1] == 2u );
        CHECK( intarrptr[2] == 3u );
        CHECK( intarrptr[3] == 4u );
    }

    SECTION( "it can be move-assigned correctly" ) {
        owning_buffer source{ weak_buffer{ junk_ptr, junk_len }, &alloc };
        owning_buffer target = std::move(source);

        CHECK( source.data() == nullptr ); // NOLINT(bugprone-use-after-move)
        CHECK( source.length() == 0u );
        CHECK( source.capacity() == 0u );
        CHECK( source.growth() == k_default_growth_factor );
        CHECK( source.allocator() == nullptr );


        CHECK( target.length() == junk_len );
        CHECK( target.capacity() >= junk_len );
        CHECK( target.data() != nullptr );
        CHECK( target.data() != junk_ptr );
        CHECK( target.growth() == k_default_growth_factor );
        CHECK( target.allocator() == &alloc );

        const auto intarrptr = reinterpret_cast<std::uint32_t*>(target.data());
        CHECK( intarrptr[0] == 1u );
        CHECK( intarrptr[1] == 2u );
        CHECK( intarrptr[2] == 3u );
        CHECK( intarrptr[3] == 4u );
    }

}


TEST_CASE( "owning buffer has iterator methods", "[buffer][owning_buffer]" ) {

    std::array<std::uint8_t, 20> junk{ 0u };
    std::ranges::generate(junk, [n = 0]() mutable { return ++n; });

    owning_buffer buffer{ weak_buffer{ junk.data(), junk.size() } };

    SECTION( "has begin iterator method" ) {
        CHECK( *buffer.begin() + 00 == 01u );
        CHECK( *buffer.begin() + 01 == 02u );
        CHECK( *buffer.begin() + 19 == 20u );
    }

    SECTION( "has end iterator method" ) {
        CHECK( buffer.end() == buffer.begin() + buffer.length() );
        CHECK( buffer.cend() == buffer.cbegin() + buffer.length() );

        CHECK( *(buffer.end() - 01) == 20u );
        CHECK( *(buffer.end() - 02) == 19u );
        CHECK( *(buffer.end() - 20) == 01u );
    }

    SECTION( "has capacity end iterator method" ) {
        CHECK( buffer.alloc_end() == buffer.begin() + buffer.capacity() );
        CHECK( buffer.alloc_cend() == buffer.cbegin() + buffer.capacity() );
    }

    SECTION( "can be used in range-for" ) {
        std::size_t i = 0u;

        for (auto b : buffer) {
            CHECK( b == ++i );
        }
    }

}


TEST_CASE( "owning buffer provides byte access", "[buffer][owning_buffer]" ) {

    std::array<std::uint8_t, 32> junk{ 0u };
    std::ranges::generate(junk, [n = 0]() mutable { return ++n; });
    owning_buffer buffer{ weak_buffer{ junk.data(), junk.size() } };

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


TEST_CASE( "owning buffer provides partial access", "[buffer][owning_buffer]" ) {

    std::array<std::uint8_t, 10> junk{ 0u };
    std::ranges::generate(junk, [n = 0]() mutable { return ++n; });
    owning_buffer buffer{ weak_buffer{ junk.data(), junk.size() } };

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


TEST_CASE( "owning buffer supports unsafe resizing", "[buffer][owning_buffer]" ) {

    std::array<std::uint8_t, 32> junk{ 0u };
    std::ranges::generate(junk, [n = 0]() mutable { return ++n; });
    owning_buffer buffer{ weak_buffer{ junk.data(), junk.size() } };

    const auto old_data = buffer.data();
    const auto old_length = buffer.length();
    const auto old_capacity = buffer.capacity();

    SECTION( "downwards (to zero)" ) {
        buffer.resize_to_zero();

        CHECK( buffer.data() == old_data );
        CHECK( buffer.capacity() == old_capacity );

        CHECK( buffer.length() == 0u );
        CHECK( buffer.end() == buffer.begin() );
    }

    /// @todo implement test case for owning_buffer::resize_to_capacity()

//    SECTION( "upwards (to capacity)" ) {
//        // ...
//    }

}


TEST_CASE( "owning buffer can be overwritten", "[buffer][owning_buffer]" ) {

    std::array<std::uint8_t, 10> junk{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };
    std::array<std::uint8_t, 12> repl{ 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u, 32u };

    owning_buffer buffer{ weak_buffer{ junk.data(), junk.size() } };

    const auto old_data = buffer.data();
    const auto old_length = buffer.length();
    const auto old_capacity = buffer.capacity();

    SECTION( "but destination iterator must be within the buffer" ) {

        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.begin() + 3, buffer.cend() + 1), io_exception );
        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.begin() + 3, buffer.cbegin() - 1), io_exception );
        CHECK_THROWS_AS( buffer.overwrite(repl.begin(), repl.begin() + 3, nullptr), io_exception );

    }

    SECTION( "but source iterators must be ordered" ) {

        CHECK_THROWS_AS( buffer.overwrite(repl.end(), repl.begin(), buffer.cbegin()), io_exception );
        CHECK_THROWS_AS( buffer.overwrite(repl.begin() + 5, repl.begin() + 2, buffer.cbegin()), io_exception );

    }

    SECTION( "from the start" ) {

        // empty write:

        auto end_0 = buffer.overwrite(repl.begin(), repl.begin(), buffer.begin());
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.begin() );

        // partial write:

        auto end_1 = buffer.overwrite(repl.begin(), repl.begin() + 3u, buffer.begin());
        auto result_1 = std::array{ 21u, 22u, 23u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_1) );
        CHECK( end_1 == buffer.begin() + 3u );

        // full write:

        auto end_2 = buffer.overwrite(repl.begin(), repl.begin() + 10u, buffer.begin());
        auto result_2 = std::array{ 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 30u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_2) );
        CHECK( end_2 == buffer.end() );

        // overflowing write:

        auto end_3 = buffer.overwrite(repl.begin(), repl.end(), buffer.begin());
        auto result_3 = std::array{ 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u, 32u };

        CHECK( buffer.data() != old_data );
        CHECK( buffer.length() == repl.size() );
        CHECK( buffer.capacity() >= old_capacity );
        CHECK( buffer.capacity() >= repl.size() );
        CHECK( std::ranges::equal(buffer, result_3) );
        CHECK( end_3 == buffer.end() );

    }

    SECTION( "from the middle" ) {

        // empty write:

        auto end_0 = buffer.overwrite(repl.begin(), repl.begin(), buffer.begin() + 4u);
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.begin() + 4u );

        // partial write:

        auto end_1 = buffer.overwrite(repl.begin(), repl.begin() + 3u, buffer.begin() + 4u);
        auto result_1 = std::array{ 1u, 2u, 3u, 4u, 21u, 22u, 23u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_1) );
        CHECK( end_1 == buffer.begin() + 7u );

        // full write:

        auto end_2 = buffer.overwrite(repl.begin(), repl.begin() + 6u, buffer.begin() + 4u);
        auto result_2 = std::array{ 1u, 2u, 3u, 4u, 21u, 22u, 23u, 24u, 25u, 26u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_2) );
        CHECK( end_2 == buffer.end() );

        // overflowing write:

        auto end_3 = buffer.overwrite(repl.begin(), repl.begin() + 7u, buffer.begin() + 4u);
        auto result_3 = std::array{ 1u, 2u, 3u, 4u, 21u, 22u, 23u, 24u, 25u, 26u, 27u };

        CHECK( buffer.data() != old_data );
        CHECK( buffer.length() == 11u );
        CHECK( buffer.capacity() >= old_capacity );
        CHECK( buffer.capacity() >= repl.size() );
        CHECK( std::ranges::equal(buffer, result_3) );
        CHECK( end_3 == buffer.end() );

    }

    SECTION( "from the end" ) {

        // empty write:

        auto end_0 = buffer.overwrite(repl.begin(), repl.begin(), buffer.end());
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.end() );

        // overflowing write:

        auto end_1 = buffer.overwrite(repl.begin(), repl.begin() + 1u, buffer.end());
        auto result_1 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 21u };

        CHECK( buffer.data() != old_data );
        CHECK( buffer.capacity() >= old_capacity );
        CHECK( buffer.length() == 11u );
        CHECK( std::ranges::equal(buffer, result_1) );
        CHECK( end_1 == buffer.end() );

    }

}


TEST_CASE( "owning buffer can be inserted into", "[buffer][owning_buffer]" ) {

    std::array<std::uint8_t, 10> junk{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };
    std::array<std::uint8_t, 12> repl{ 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u, 32u };

    owning_buffer buffer{ weak_buffer{ junk.data(), junk.size() } };

    const auto old_data = buffer.data();
    const auto old_length = buffer.length();
    const auto old_capacity = buffer.capacity();

    SECTION( "but destination iterator must be within the buffer" ) {

        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.begin() + 3, buffer.cend() + 1), io_exception );
        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.begin() + 3, buffer.cbegin() - 1), io_exception );
        CHECK_THROWS_AS( buffer.insert(repl.begin(), repl.begin() + 3, nullptr), io_exception );

    }

    SECTION( "but source iterators must be ordered" ) {

        CHECK_THROWS_AS( buffer.insert(repl.end(), repl.begin(), buffer.cbegin()), io_exception );
        CHECK_THROWS_AS( buffer.insert(repl.begin() + 5, repl.begin() + 2, buffer.cbegin()), io_exception );

    }

    SECTION( "at the start" ) {

        // empty write:

        auto end_0 = buffer.insert(repl.begin(), repl.begin(), buffer.begin());
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.begin() );

        // partial write:

        auto end_1 = buffer.insert(repl.begin(), repl.begin() + 3u, buffer.begin());
        auto result_1 = std::array{ 21u, 22u, 23u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.capacity() >= old_length + 3u );
        CHECK( buffer.length() == old_length + 3u );
        CHECK( std::ranges::equal(buffer, result_1) );
        CHECK( end_1 == buffer.begin() + 3u );

        // full write:

        auto end_2 = buffer.insert(repl.begin(), repl.end(), buffer.begin());
        auto result_2 = std::array{ 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u, 32u, 21u, 22u, 23u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.capacity() >= old_length + 3u + 12u );
        CHECK( buffer.length() == old_length + 3u + 12u );
        CHECK( std::ranges::equal(buffer, result_2) );
        CHECK( end_2 == buffer.begin() + 12u );

    }

    SECTION( "at the middle" ) {

        // empty write:

        auto end_0 = buffer.insert(repl.begin(), repl.begin(), buffer.begin() + 4u);
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.begin() + 4u );

        // partial write:

        auto end_1 = buffer.insert(repl.begin(), repl.begin() + 3u, buffer.begin() + 4u);
        auto result_1 = std::array{ 1u, 2u, 3u, 4u, 21u, 22u, 23u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.capacity() >= old_length + 3u );
        CHECK( buffer.length() == old_length + 3u );
        CHECK( std::ranges::equal(buffer, result_1) );
        CHECK( end_1 == buffer.begin() + 7u );

        // full write:

        auto end_2 = buffer.insert(repl.begin(), repl.begin() + 6u, buffer.begin() + 4u);
        auto result_2 = std::array{ 1u, 2u, 3u, 4u, 21u, 22u, 23u, 24u, 25u, 26u, 21u, 22u, 23u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.capacity() >= old_length + 3u + 6u );
        CHECK( buffer.length() == old_length + 3u + 6u );
        CHECK( std::ranges::equal(buffer, result_2) );
        CHECK( end_2 == buffer.begin() + 4u + 6u );

    }

    SECTION( "at the end" ) {

        // empty write:

        auto end_0 = buffer.insert(repl.begin(), repl.begin(), buffer.end());
        auto result_0 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };

        CHECK( buffer.data() == old_data );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( buffer.length() == old_length );
        CHECK( std::ranges::equal(buffer, result_0) );
        CHECK( end_0 == buffer.end() );

        // overflowing write:

        auto end_1 = buffer.insert(repl.begin(), repl.end(), buffer.end());
        auto result_1 = std::array{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 21u, 22u, 23u, 24u, 25u, 26u, 27u, 28u, 29u, 30u, 31u, 32u };

        CHECK( buffer.capacity() >= old_capacity + 12u );
        CHECK( buffer.length() == old_length + 12u );
        CHECK( std::ranges::equal(buffer, result_1) );
        CHECK( end_1 == buffer.end() );

    }

}


TEST_CASE( "owning buffer can be removed from", "[buffer][owning_buffer]" ) {

    std::array<std::uint8_t, 12> junk{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u };
    owning_buffer buffer{ weak_buffer{ junk.data(), junk.size() } };

    const auto old_data = buffer.data();
    const auto old_length = buffer.length();
    const auto old_capacity = buffer.capacity();

    SECTION( "but the iterators must be ordered" ) {
        CHECK_THROWS_AS( buffer.erase(buffer.end(), buffer.begin()), io_exception );
        CHECK_THROWS_AS( buffer.erase(buffer.begin() + 2u, buffer.begin() + 1u), io_exception );
    }

    SECTION( "but the iterators must be within the buffer" ) {
        CHECK_THROWS_AS( buffer.erase(nullptr, buffer.end()), io_exception );
        CHECK_THROWS_AS( buffer.erase(buffer.begin(), nullptr), io_exception );
        CHECK_THROWS_AS( buffer.erase(buffer.begin() - 1u, buffer.end()), io_exception );
        CHECK_THROWS_AS( buffer.erase(buffer.begin(), buffer.end() + 1u), io_exception );
        CHECK_THROWS_AS( buffer.erase(buffer.begin() - 1000u, buffer.end() + 3u), io_exception );
    }

    SECTION( "at the beginning" ) {
        const auto end_0 = buffer.erase(buffer.begin(), buffer.begin());
        const auto result_0 = { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u };

        CHECK( end_0 == buffer.begin() );
        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_0) );


        const auto end_1 = buffer.erase(buffer.begin(), buffer.begin() + 4u);
        const auto result_1 = { 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u };

        CHECK( end_1 == buffer.begin() );
        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == 8u );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_1) );


        const auto end_2 = buffer.erase(buffer.begin(), buffer.end());

        CHECK( end_2 == buffer.begin() );
        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == 0u );
        CHECK( buffer.capacity() == old_capacity );
    }

    SECTION( "at the middle" ) {
        const auto end_0 = buffer.erase(buffer.begin() + 2u, buffer.begin() + 2u);
        const auto result_0 = { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u };

        CHECK( end_0 == buffer.begin() + 2u );
        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_0) );


        const auto end_1 = buffer.erase(buffer.begin() + 2u, buffer.begin() + 5u);
        const auto result_1 = { 1u, 2u, 6u, 7u, 8u, 9u, 10u, 11u, 12u };

        CHECK( end_1 == buffer.begin() + 2u );
        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == 9u );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_1) );
    }

    SECTION( "at the end" ) {
        const auto end_0 = buffer.erase(buffer.end(), buffer.end());
        const auto result_0 = { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u };

        CHECK( end_0 == buffer.end() );
        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == old_length );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_0) );


        const auto end_1 = buffer.erase(buffer.begin() + 8u, buffer.end());
        const auto result_1 = { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u };

        CHECK( end_1 == buffer.begin() + 8u );
        CHECK( buffer.data() == old_data );
        CHECK( buffer.length() == 8u );
        CHECK( buffer.capacity() == old_capacity );
        CHECK( std::ranges::equal(buffer, result_1) );
    }

}
