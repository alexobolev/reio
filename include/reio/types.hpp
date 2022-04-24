#ifndef REIO_TYPES_HPP
#define REIO_TYPES_HPP


#ifndef REIO_OPTION_NO_INCLUDES

#include <algorithm>
#include <bit>
#include <cstdint>
#include <exception>
#include <iterator>
#include <utility>

#endif


namespace reio
{

    using byte = unsigned char;


    /// @brief Struct/class which can be (de)serialized by dereferencing.
    template<typename T>
    concept structured_type = std::is_class_v<T> && std::is_standard_layout_v<T>;

    /// @brief Integer or floating point type.
    template<typename T>
    concept numeric_type = std::integral<T> || std::floating_point<T>;

    /// @brief Number @c S for which there is a standard integer type of size @c S.
    template<int S>
    concept regular_numeric_size = S == 1 || S == 2 || S == 4 || S == 8;

    /// @brief Integer or floating point type of one of the standard integer sizes.
    template<typename T>
    concept regular_numeric_type = numeric_type<T> && regular_numeric_size<sizeof(T)>;


#if defined(_MSC_VER)
    #define BSWAP_16_FUNC _byteswap_ushort
    #define BSWAP_32_FUNC _byteswap_ulong
    #define BSWAP_64_FUNC _byteswap_uint64
#elif defined(__GNUC__) || defined(__clang__)
    #define BSWAP_16_FUNC __builtin_bswap16
    #define BSWAP_32_FUNC __builtin_bswap32
    #define BSWAP_64_FUNC __builtin_bswap64
#else
    #error Probably unsupported compiler | bswap |
#endif

    ///
    /// @brief Reverse byte order in a regular-sized integer or float value.
    /// Implemented to try to compile down to a single instruction or even a compile-time operation.
    ///
    template<regular_numeric_type T>
    constexpr auto bswap(T v) noexcept
    {
        const auto size = sizeof(v);

        if constexpr (size == 1) return v;
        if constexpr (size == 2) return std::bit_cast<T>(BSWAP_16_FUNC(std::bit_cast<uint16_t>(v)));
        if constexpr (size == 4) return std::bit_cast<T>(BSWAP_32_FUNC(std::bit_cast<uint32_t>(v)));
        if constexpr (size == 8) return std::bit_cast<T>(BSWAP_64_FUNC(std::bit_cast<uint64_t>(v)));
    }

#undef BSWAP_16_FUNC
#undef BSWAP_32_FUNC
#undef BSWAP_64_FUNC


    ///
    /// Common mixin for classes which shouldn't be copied,
    /// and where move logic should thus be implemented individually.
    ///
    class non_copyable
    {
    public:
        non_copyable() = default;
        non_copyable(const non_copyable&) = delete;
        non_copyable& operator=(const non_copyable&) = delete;
    };


#if defined(_MSC_VER)
    #define _REIO_FUNC_     __FUNCSIG__  // NOLINT(bugprone-reserved-identifier)
#elif defined(__GNUC__) || defined(__clang__)
    #define _REIO_FUNC_     __PRETTY_FUNCTION__  // NOLINT(bugprone-reserved-identifier)
#else
    #error Probably unsupported compiler | _REIO_FUNC_ |
#endif


    ///
    /// Minimum information required to uniquely identify
    /// a location of a method/function call within the codebase.
    ///
    struct code_location final
    {
        long            m_line;
        const char*     m_file;
        const char*     m_func;

        constexpr code_location() = default;
        constexpr code_location(long line, const char* file)
            : m_line{ line }, m_file{ file }, m_func{ nullptr } {}
        constexpr code_location(long line, const char* file, const char* func)
            : m_line{ line }, m_file{ file }, m_func{ func } {}
    };


    ///
    /// Exception class which is thrown by the default
    /// assert handler (@c REIO_FAIL).
    ///
    class io_exception : public std::exception
    {
    protected:

        code_location   m_location{};
        const char*     m_description{};

    public:

        io_exception() = default;
        io_exception(code_location loc, const char* desc)
            : m_location{ loc }, m_description{ desc } {}

        [[nodiscard]] inline auto line() const noexcept -> long { return m_location.m_line; }
        [[nodiscard]] inline auto file() const noexcept -> const char* { return m_location.m_file; }
        [[nodiscard]] inline auto func() const noexcept -> const char* { return m_location.m_func; }
        [[nodiscard]] inline auto desc() const noexcept -> const char* { return m_description; }

        [[nodiscard]] inline const char* what() const noexcept override { return m_description; }

    };

}

#endif // REIO_TYPES_HPP
