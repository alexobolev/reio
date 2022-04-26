/// @file       _docs.hpp
/// @brief      File contains definitions for Doxygen groups.

#ifndef REIO__DOCS_HPP
#define REIO__DOCS_HPP


///
/// @defgroup   buffers    Buffer types
/// @brief      Abstractions used for handling contiguous blocks of bytes.
///



///
/// @defgroup   streams    Stream I/O types
/// @brief      Sequential I/O primitives providing buffer and number (de)serialization. @n
///
/// Stream classes provide either input or output capabilities,
/// input being deserialization, and output being serialization. @n
///
/// All streams implement @c base_stream and, depending on whether they
/// are input- or output- oriented, @c input_stream or @c output_stream.
///



///
/// @defgroup   macros     Configuration macros
/// @brief      Several macros that can be used to modify REIO behaviour.
///


#ifdef DOXYGEN_ONLY

///
/// @def        REIO_OPTION_CUSTOM_FAIL
/// @brief      Define together with a custom @c REIO_FAIL to use
///             an error handling mechanism different from custom exceptions.
/// @ingroup    macros
///

#define REIO_OPTION_CUSTOM_FAIL

///
/// @def        REIO_OPTION_NO_ASSERTS
/// @brief      Define to skip all assertion code (might be useful in release builds).
/// @ingroup    macros
///

#define REIO_OPTION_NO_ASSERTS

#endif


#endif //REIO__DOCS_HPP
