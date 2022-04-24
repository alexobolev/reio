#ifndef REIO_ASSERTS_HPP
#define REIO_ASSERTS_HPP


#ifndef REIO_OPTION_CUSTOM_FAIL

    #define REIO_FAIL(MSG, FILE, LINE, FUNC) \
        throw ::reio::io_exception{ { LINE, FILE, FUNC }, MSG }

#endif // REIO_OPTION_CUSTOM_FAIL


#ifndef REIO_OPTION_NO_ASSERTS

    #define REIO_ASSERT(COND, MSG)                                              \
        do {                                                                    \
            if (!(COND)) [[unlikely]] {                                         \
                REIO_FAIL("" MSG " (! " #COND ")",                              \
                    __FILE__, __LINE__, _REIO_FUNC_);                           \
            }                                                                   \
        } while (false)

#else

    #define REIO_ASSERT(COND, MSG)                                              \
        do { } while (false)

#endif // REIO_OPTION_NO_ASSERTS


#endif //REIO_ASSERTS_HPP
