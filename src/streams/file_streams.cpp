#include "reio/streams/file_streams.hpp"
#include <limits>


namespace reio
{

    static constexpr int seek_origin_to_std(seek_origin origin) noexcept
    {
        switch (origin)
        {
            case seek_origin::begin:     return SEEK_SET;
            case seek_origin::current:   return SEEK_CUR;
            case seek_origin::end:       return SEEK_END;
        }
    }

    template<seek_origin Origin>
    inline void DoSeek(std::FILE* file, int64_t offset)
    {
        using limits = std::numeric_limits<long>;
        REIO_ASSERT(offset >= limits::min(), "seeked offset is too little for fseek");
        REIO_ASSERT(offset <= limits::max(), "seeked offset is too big for fseek");

        const auto offset_ = static_cast<long>(offset);
        constexpr auto origin_ = seek_origin_to_std(Origin);

        [[maybe_unused]] const auto rc = std::fseek(file, offset_, origin_);
        REIO_ASSERT(rc == 0, "failed to seek the file");
    }

    inline long DoTell(std::FILE* file)
    {
        const auto pos = std::ftell(file);
        REIO_ASSERT(pos != -1L, "failed to get current stream position");
        return pos;
    }

    inline long DoGetLength(std::FILE* file)
    {
        int rc;
        std::fpos_t pos{};

        rc = std::fgetpos(file, &pos);
        REIO_ASSERT(!rc, "failed to store file position");

        DoSeek<seek_origin::end>(file, 0);
        const auto len = DoTell(file);

        rc = std::fsetpos(file, &pos);
        REIO_ASSERT(!rc, "failed to restore file position");

        return len;
    }



    ///
    /// @brief      Initialize stream by acquiring ownership of a file handle.
    ///             The handle is closed by the stream.
    /// @param      handle   Externally-opened file handle.
    ///
    file_input_stream::file_input_stream(std::FILE *handle)
        : m_handle{ handle }
    {
        REIO_ASSERT(handle != nullptr, "can't initialize file stream with a null handle");
    }

    ///
    /// @brief      Initialize stream by opening a physical file.
    /// @param      path    Path to the file which should be used for input.
    ///
    file_input_stream::file_input_stream(std::string_view path)
    {
        m_handle = std::fopen(path.data(), "rb");
        REIO_ASSERT(m_handle != nullptr, "failed to open a file for file stream");
    }

    file_input_stream::~file_input_stream()
    {
        [[maybe_unused]] const auto rc = std::fclose(m_handle);

#ifdef _DEBUG
        if (rc != 0)
        {
            std::fputs("warning: std::fclose reported failure in ~file_input_stream", stderr);
        }
#endif
    }

    int64_t
    file_input_stream::position()
    {
        return DoTell(m_handle);
    }

    int64_t
    file_input_stream::length()
    {
        return DoGetLength(m_handle);
    }

    void
    file_input_stream::seek_begin(int64_t offset)
    {
        DoSeek<seek_origin::begin>(m_handle, offset);
    }

    void
    file_input_stream::seek_current(int64_t offset)
    {
        DoSeek<seek_origin::current>(m_handle, offset);
    }

    void
    file_input_stream::seek_end(int64_t offset)
    {
        DoSeek<seek_origin::end>(m_handle, offset);
    }

    int64_t
    file_input_stream::read_bytes(weak_buffer output)
    {
        static_assert(sizeof(byte) == 1u);
        const auto read = std::fread(
                output.data(),
                sizeof(byte),
                output.length(),
                m_handle);

        return static_cast<int64_t>(read);
    }



    ///
    /// @brief      Initialize stream by acquiring ownership of a file handle.
    ///             The handle is closed by the stream.
    /// @param      handle   Externally-opened file handle.
    ///
    file_output_stream::file_output_stream(std::FILE *handle)
        : m_handle{ handle }
    {
        REIO_ASSERT(handle != nullptr, "can't initialize file stream with a null handle");
    }

    ///
    /// @brief      Initialize stream by opening a physical file.
    /// @param      path    Path to the file which should be used for output.
    ///
    file_output_stream::file_output_stream(std::string_view path)
    {
        m_handle = std::fopen(path.data(), "wb");
        REIO_ASSERT(m_handle != nullptr, "failed to open a file for file output stream");
    }

    file_output_stream::~file_output_stream()
    {
        [[maybe_unused]] const auto rc = std::fclose(m_handle);

#ifdef _DEBUG
        if (rc != 0)
        {
            std::fputs("warning: std::fclose reported failure in ~file_output_stream", stderr);
        }
#endif
    }

    int64_t
    file_output_stream::position()
    {
        return DoTell(m_handle);
    }

    int64_t
    file_output_stream::length()
    {
        return DoGetLength(m_handle);
    }

    void
    file_output_stream::seek_begin(int64_t offset)
    {
        DoSeek<seek_origin::begin>(m_handle, offset);
    }

    void
    file_output_stream::seek_current(int64_t offset)
    {
        DoSeek<seek_origin::current>(m_handle, offset);
    }

    void
    file_output_stream::seek_end(int64_t offset)
    {
        DoSeek<seek_origin::end>(m_handle, offset);
    }

    int64_t
    file_output_stream::write_bytes(weak_buffer input)
    {
        static_assert(sizeof(byte) == 1u);
        const auto written = std::fwrite(
                input.data(),
                sizeof(byte),
                input.length(),
                m_handle);

        return static_cast<int64_t>(written);
    }
}
