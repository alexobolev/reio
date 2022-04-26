#ifndef REIO_FILE_STREAMS_HPP
#define REIO_FILE_STREAMS_HPP


#ifndef REIO_OPTION_NO_INCLUDES

#include <cstdio>
#include <string>

#endif

#include "./streams.hpp"


namespace reio
{

    ///
    /// @brief      Implementation of @c input_stream using
    ///             a CRT file handle as a data source.
    ///
    /// @ingroup    streams
    ///
    class file_input_stream final
        : public input_stream
        , public non_copyable
    {
    private:

        std::FILE* m_handle = nullptr;

    public:

        explicit file_input_stream(std::FILE* handle);
        explicit file_input_stream(std::string_view path);

        ~file_input_stream() override;

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


    ///
    /// @brief      Implementation of @c input_stream using
    ///             a CRT file handle as a data sink.
    ///
    /// @ingroup    streams
    ///
    class file_output_stream final
        : public output_stream
        , public non_copyable
    {
    private:

        std::FILE* m_handle = nullptr;

    public:

        explicit file_output_stream(std::FILE* handle);
        explicit file_output_stream(std::string_view path);

        ~file_output_stream() override;

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

#endif //REIO_FILE_STREAMS_HPP
