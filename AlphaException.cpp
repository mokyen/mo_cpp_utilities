#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <source_location>
#include <sstream>

// Platform detection and capability definitions
#if defined(YOCTO_BUILD)
    #if defined(HAVE_LIBBACKTRACE)
        #define FULL_STACK_TRACE_CAPABLE 1
    #else
        #define FULL_STACK_TRACE_CAPABLE 0
    #endif
#elif defined(__linux__) || defined(__APPLE__) || defined(_WIN32)
    #define FULL_STACK_TRACE_CAPABLE 1
#else
    #define FULL_STACK_TRACE_CAPABLE 0
#endif

// Memory constraints detection
#if defined(__AVR__) || defined(__MSP430__) || defined(MINIMAL_RESOURCES)
    #define MINIMAL_STACK_TRACE 1
#else
    #define MINIMAL_STACK_TRACE 0
#endif

// Stack trace configuration based on build type and platform capabilities
#if defined(DEBUG_BUILD) && FULL_STACK_TRACE_CAPABLE
    #include <stacktrace>
    #define STACK_TRACE_MODE 2  // Full stack trace with libbacktrace
#elif !MINIMAL_STACK_TRACE
    #define STACK_TRACE_MODE 1  // Custom stack trace implementation
#else
    #define STACK_TRACE_MODE 0  // Minimal stack trace (just current location)
#endif

struct CompileTimeFrame {
    const char* function_name;
    const char* file_name;
    unsigned int line;
    unsigned int column;
    
    constexpr CompileTimeFrame(const std::source_location& loc = 
                              std::source_location::current()) noexcept
        : function_name(loc.function_name())
        , file_name(loc.file_name())
        , line(loc.line())
        , column(loc.column())
    {}

    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os, 
        const CompileTimeFrame& frame) {
        os << frame.file_name << "(" << frame.line << ":"
           << frame.column << ") `" << frame.function_name << "`";
        return os;
    }
};

#if STACK_TRACE_MODE == 1
    class StackTrace {
        std::vector<CompileTimeFrame> frames_;

    public:
        void push_frame(const CompileTimeFrame& frame) {
            frames_.push_back(frame);
        }

        void pop_frame() {
            if (!frames_.empty()) {
                frames_.pop_back();
            }
        }

        const std::vector<CompileTimeFrame>& frames() const { return frames_; }

        template<typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits>& operator<<(
            std::basic_ostream<CharT, Traits>& os,
            const StackTrace& trace) {
            for (const auto& frame : trace.frames()) {
                os << frame << "\n";
            }
            return os;
        }
    };

    thread_local StackTrace current_trace;

    class StackGuard {
        StackTrace& trace_;

    public:
        explicit StackGuard(StackTrace& trace, 
                        const std::source_location& loc = std::source_location::current())
            : trace_(trace) {
            trace_.push_frame(CompileTimeFrame{loc});
        }
        
        ~StackGuard() {
            trace_.pop_frame();
        }
    };
#endif

template<typename DATA_T>
class OmegaException {
    std::string err_str;
    DATA_T data_;
    CompileTimeFrame location_;
    
    #if STACK_TRACE_MODE == 2
        std::stacktrace debug_backtrace_;
    #elif STACK_TRACE_MODE == 1
        StackTrace production_backtrace_;
    #endif

public:
    OmegaException(const char* str, const DATA_T& data,
                   const std::source_location& loc = std::source_location::current())
        : err_str(str)
        , data_(data)
        , location_(loc)
        #if STACK_TRACE_MODE == 2
            , debug_backtrace_(std::stacktrace::current())
        #elif STACK_TRACE_MODE == 1
            , production_backtrace_(current_trace)
        #endif
    {}

    const DATA_T& data() const noexcept { return data_; }
    const std::string& what() const noexcept { return err_str; }
    const CompileTimeFrame& where() const { return location_; }

    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const OmegaException& e) {
        os << "Exception: " << e.what() << "\nLocation: " << e.where();
        #if STACK_TRACE_MODE == 2
            os << "\nStack trace:\n" << e.debug_backtrace_;
        #elif STACK_TRACE_MODE == 1
            os << "\nStack trace:\n" << e.production_backtrace_;
        #endif
        return os;
    }

    #if STACK_TRACE_MODE == 2
        const std::stacktrace& stack() const { return debug_backtrace_; }
    #elif STACK_TRACE_MODE == 1
        const StackTrace& stack() const { return production_backtrace_; }
    #else
        const CompileTimeFrame& stack() const { return location_; }
    #endif
};

#if STACK_TRACE_MODE == 1
    #define TRACE_FUNCTION() StackGuard _stack_guard(current_trace)
#else
    #define TRACE_FUNCTION() ((void)0)
#endif
