// Copyright 2008-present Contributors to the OpenImageIO project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/OpenImageIO/oiio


#pragma once

#include <OpenImageIO/export.h>
#include <OpenImageIO/oiioversion.h>
#include <OpenImageIO/strutil.h>


// If OIIO_ERRORHANDLER_HIDE_PRINTF is defined, mark the old-style printf-like
// format functions as deprecated. (This is a debugging aid for downstream
// projects who want to root out any places where they might be using the old
// one).
#if defined(OIIO_ERRORHANDLER_HIDE_PRINTF) || defined(OIIO_INTERNAL)
#    define OIIO_ERRORHANDLER_PRINTF_DEPRECATED \
        OIIO_DEPRECATED(                        \
            "old style (printf-like) formatting version of this function is deprecated")
#else
#    define OIIO_ERRORHANDLER_PRINTF_DEPRECATED
#endif


OIIO_NAMESPACE_BEGIN

/// ErrorHandler is a simple class that accepts error messages
/// (classified as errors, severe errors, warnings, info, messages, or
/// debug output) and handles them somehow.  By default it just prints
/// the messages to stdout and/or stderr (and suppresses some based on a
/// "verbosity" level).
///
/// The basic idea is that your library code has no idea whether some
/// application that will use it someday will want errors or other
/// output to be sent to the console, go to a log file, be intercepted
/// by the calling application, or something else.  So you punt, by
/// having your library take a pointer to an ErrorHandler, passed in
/// from the calling app (and possibly subclassed to have arbitrarily
/// different behavior from the default console output) and make all
/// error-like output via the ErrorHandler*.
///
class OIIO_UTIL_API ErrorHandler {
public:
    /// Error categories.  We use broad categories in the high order bits.
    /// A library may just use these categories, or may create individual
    /// error codes as long as they have the right high bits to designate
    /// their category (file not found = ERROR + 1, etc.).
    enum ErrCode {
        EH_NO_ERROR = 0,  // never sent to handler
        EH_MESSAGE  = 0 << 16,
        EH_INFO     = 1 << 16,
        EH_WARNING  = 2 << 16,
        EH_ERROR    = 3 << 16,
        EH_SEVERE   = 4 << 16,
        EH_DEBUG    = 5 << 16
    };

    /// VerbosityLevel controls how much detail the calling app wants.
    ///
    enum VerbosityLevel {
        QUIET   = 0,  ///< Show MESSAGE, SEVERE, ERROR only
        NORMAL  = 1,  ///< Show MESSAGE, SEVERE, ERROR, WARNING
        VERBOSE = 2   ///< Like NORMAL, but also show INFO
    };

    ErrorHandler() noexcept
        : m_verbosity(NORMAL)
    {
    }
    virtual ~ErrorHandler() {}

    /// Set desired verbosity level.
    void verbosity(int v) noexcept { m_verbosity = v; }

    /// Return the current verbosity level.
    int verbosity() const noexcept { return m_verbosity; }

    /// The main (or "full detail") method -- takes a code (with high
    /// bits being an ErrCode) and writes the message, with a prefix
    /// indicating the error category (no prefix for "MESSAGE") and
    /// error string.
    virtual void operator()(int errcode, const std::string& msg);

    // Base cases -- take a single string
    void info(const std::string& msg) { (*this)(EH_INFO, msg); }
    void warning(const std::string& msg) { (*this)(EH_WARNING, msg); }
    void error(const std::string& msg) { (*this)(EH_ERROR, msg); }
    void severe(const std::string& msg) { (*this)(EH_SEVERE, msg); }
    void message(const std::string& msg) { (*this)(EH_MESSAGE, msg); }
#ifndef NDEBUG
    void debug(const std::string& msg) { (*this)(EH_DEBUG, msg); }
#else
    void debug(const std::string&) {}
#endif

    // Formatted output with the same notation as Strutil::format.
    /// Use with caution! Some day this will change to be fmt-like rather
    /// than printf-like.
    template<typename... Args>
    OIIO_FORMAT_DEPRECATED void info(const char* format, const Args&... args)
    {
        if (verbosity() >= VERBOSE)
            info(Strutil::format(format, args...));
    }

    /// Warning message with printf-like formatted error message.
    /// Will not print unless verbosity >= NORMAL (i.e. will suppress
    /// for QUIET).
    template<typename... Args>
    OIIO_FORMAT_DEPRECATED void warning(const char* format, const Args&... args)
    {
        if (verbosity() >= NORMAL)
            warning(Strutil::format(format, args...));
    }

    /// Error message with printf-like formatted error message.
    /// Will print regardless of verbosity.
    template<typename... Args>
    OIIO_FORMAT_DEPRECATED void error(const char* format, const Args&... args)
    {
        error(Strutil::format(format, args...));
    }

    /// Severe error message with printf-like formatted error message.
    /// Will print regardless of verbosity.
    template<typename... Args>
    OIIO_FORMAT_DEPRECATED void severe(const char* format, const Args&... args)
    {
        severe(Strutil::format(format, args...));
    }

    /// Prefix-less message with printf-like formatted error message.
    /// Will not print if verbosity is QUIET.  Also note that unlike
    /// the other routines, message() will NOT append a newline.
    template<typename... Args>
    OIIO_FORMAT_DEPRECATED void message(const char* format, const Args&... args)
    {
        if (verbosity() > QUIET)
            message(Strutil::format(format, args...));
    }

    /// Debugging message with printf-like formatted error message.
    /// This will not produce any output if not in DEBUG mode, or
    /// if verbosity is QUIET.
    template<typename... Args>
    OIIO_FORMAT_DEPRECATED void debug(const char* format OIIO_MAYBE_UNUSED,
                                      const Args&... args OIIO_MAYBE_UNUSED)
    {
#ifndef NDEBUG
        debug(Strutil::format(format, args...));
#endif
    }

    //
    // Formatted output with printf notation. Use these if you specifically
    // want printf-notation, even after format() changes to python notation
    // in some future OIIO release.
    //
    template<typename... Args>
    OIIO_ERRORHANDLER_PRINTF_DEPRECATED void infof(const char* format,
                                                   const Args&... args)
    {
        if (verbosity() >= VERBOSE)
            info(Strutil::sprintf(format, args...));
    }

    template<typename... Args>
    OIIO_ERRORHANDLER_PRINTF_DEPRECATED void warningf(const char* format,
                                                      const Args&... args)
    {
        if (verbosity() >= NORMAL)
            warning(Strutil::sprintf(format, args...));
    }

    template<typename... Args>
    OIIO_ERRORHANDLER_PRINTF_DEPRECATED void errorf(const char* format,
                                                    const Args&... args)
    {
        error(Strutil::sprintf(format, args...));
    }

    template<typename... Args>
    OIIO_ERRORHANDLER_PRINTF_DEPRECATED void severef(const char* format,
                                                     const Args&... args)
    {
        severe(Strutil::sprintf(format, args...));
    }

    template<typename... Args>
    OIIO_ERRORHANDLER_PRINTF_DEPRECATED void messagef(const char* format,
                                                      const Args&... args)
    {
        if (verbosity() > QUIET)
            message(Strutil::sprintf(format, args...));
    }

    template<typename... Args>
    OIIO_ERRORHANDLER_PRINTF_DEPRECATED void
    debugf(const char* format OIIO_MAYBE_UNUSED,
           const Args&... args OIIO_MAYBE_UNUSED)
    {
#ifndef NDEBUG
        debug(Strutil::sprintf(format, args...));
#endif
    }

    //
    // Formatted output with std::format notation. Use these if you
    // specifically want std::format-notation, even before format() changes
    // to the new notation in some future OIIO release.
    //
    template<typename... Args>
    void infofmt(const char* format, const Args&... args)
    {
        if (verbosity() >= VERBOSE)
            info(Strutil::fmt::format(format, args...));
    }

    template<typename... Args>
    void warningfmt(const char* format, const Args&... args)
    {
        if (verbosity() >= NORMAL)
            warning(Strutil::fmt::format(format, args...));
    }

    template<typename... Args>
    void errorfmt(const char* format, const Args&... args)
    {
        error(Strutil::fmt::format(format, args...));
    }

    template<typename... Args>
    void severefmt(const char* format, const Args&... args)
    {
        severe(Strutil::fmt::format(format, args...));
    }

    template<typename... Args>
    void messagefmt(const char* format, const Args&... args)
    {
        if (verbosity() > QUIET)
            message(Strutil::fmt::format(format, args...));
    }

    template<typename... Args>
    void debugfmt(const char* format, const Args&... args)
    {
#ifndef NDEBUG
        debug(Strutil::fmt::format(format, args...));
#endif
    }

    /// One built-in handler that can always be counted on to be present
    /// and just echoes the error messages to the console (stdout or
    /// stderr, depending on the error category).
    static ErrorHandler& default_handler();

private:
    int m_verbosity;
};

OIIO_NAMESPACE_END
