/*
  This is a feature-rich exception class that gives lots of information for developers.
  This code is derived from the OmegaException presented by Peter Muldoon at CppCon 2023
  See the omega_exception.cpp in this repo for links. It's named Alpha because it continues
  from Omega and wraps around the alphabet.

  Unlike the OmegaException, this class does not depend on external libraries. It attempts
  to gather as much stack information as possible at compile time.

  This implementation could be implemented with a PIMPL idiom or #defines to switch it out
  for release builds to avoid performance hits, but it's expected that this could would be
  relatively performant.

  ** Details **
  This implementation provides several key advantages:
  
  Compile-Time Optimization:
  
  The CompileTimeFrame structure captures source location information at compile time
  The CompileTimeStackTrace provides a fixed-size container that can be populated at compile time
  Static frame information is stored in the static frames array, avoiding runtime allocations
  
  
  Runtime Flexibility:
  
  The HybridStackTrace combines both compile-time and runtime information
  Dynamic frames can be added and removed as needed during program execution
  The RAII guard pattern ensures proper cleanup of dynamic frames
  
  
  Enhanced Debugging:
  
  Both static and dynamic frames are clearly labeled in the output
  The full call stack is preserved, showing both compile-time and runtime information
  Source location information includes file, line, column, and function name
  
  
  Thread Safety:
  
  The thread-local storage ensures that stack traces are thread-safe
  Each thread maintains its own independent stack trace
  
  
  
  The system provides a good balance between performance and functionality:
  
  Compile-time information is captured with no runtime overhead
  Runtime information is added only when needed
  Memory allocation is minimized by using fixed-size arrays where possible
  The RAII pattern ensures proper resource management
*/



#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <source_location>
#include <sstream>

// First, we define our compile-time frame structure that captures source location information
struct CompileTimeFrame {
    const char* function_name;
    const char* file_name;
    unsigned int line;
    unsigned int column;
    
    // Captures location information at compile time using constexpr
    constexpr CompileTimeFrame(const std::source_location& loc = 
                              std::source_location::current()) noexcept
        : function_name(loc.function_name())
        , file_name(loc.file_name())
        , line(loc.line())
        , column(loc.column())
    {}

    // Stream operator for CompileTimeFrame to enable easy printing
    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os, 
        const CompileTimeFrame& frame) {
        os << frame.file_name << "(" << frame.line << ":" 
           << frame.column << ") `" << frame.function_name << "`";
        return os;
    }
};

// A fixed-size container for stack frames that can be populated at compile time
template<size_t N>
class CompileTimeStackTrace {
    CompileTimeFrame frames_[N];
    size_t current_size_ = 0;

public:
    constexpr void push_frame(const CompileTimeFrame& frame) {
        if (current_size_ < N) {
            frames_[current_size_++] = frame;
        }
    }

    constexpr const CompileTimeFrame* begin() const { return frames_; }
    constexpr const CompileTimeFrame* end() const { return frames_ + current_size_; }
    constexpr size_t size() const { return current_size_; }
};

// Our hybrid stack trace that combines compile-time and runtime information
template<size_t N>
class HybridStackTrace {
    CompileTimeStackTrace<N> static_frames_;
    std::vector<CompileTimeFrame> dynamic_frames_;

public:
    // Constructor that captures the initial frame at compile time
    constexpr HybridStackTrace(const CompileTimeFrame& frame = CompileTimeFrame{}) {
        static_frames_.push_frame(frame);
    }

    // Add runtime frame information when needed
    void add_dynamic_frame(const CompileTimeFrame& frame) {
        dynamic_frames_.push_back(frame);
    }

    // Remove the most recent dynamic frame
    void pop_dynamic_frame() {
        if (!dynamic_frames_.empty()) {
            dynamic_frames_.pop_back();
        }
    }

    // Stream operator for the hybrid stack trace
    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const HybridStackTrace& trace) {
        // Print static frames first
        for (const auto& frame : trace.static_frames_) {
            os << "Static: " << frame << "\n";
        }
        // Then print dynamic frames
        for (const auto& frame : trace.dynamic_frames_) {
            os << "Dynamic: " << frame << "\n";
        }
        return os;
    }
};

// RAII guard for managing dynamic frames
template<size_t N>
class StackFrameGuard {
    HybridStackTrace<N>& trace_;

public:
    StackFrameGuard(HybridStackTrace<N>& trace, 
                    const CompileTimeFrame& frame = CompileTimeFrame{})
        : trace_(trace) {
        trace_.add_dynamic_frame(frame);
    }
    
    ~StackFrameGuard() {
        trace_.pop_dynamic_frame();
    }
};

// Our enhanced exception class that uses the hybrid stack trace
template<typename DATA_T, size_t MaxFrames = 32>
class AlphaException {
    std::string err_str;
    DATA_T data_;
    CompileTimeFrame location_;
    HybridStackTrace<MaxFrames> backtrace_;

public:
    AlphaException(const char* str, 
                   const DATA_T& data,
                   const CompileTimeFrame& loc = CompileTimeFrame{},
                   const HybridStackTrace<MaxFrames>& trace = HybridStackTrace<MaxFrames>{})
        : err_str(str)
        , data_(data)
        , location_(loc)
        , backtrace_(trace)
    {}

    const DATA_T& data() const noexcept { return data_; }
    const std::string& what() const noexcept { return err_str; }
    const CompileTimeFrame& where() const { return location_; }
    const HybridStackTrace<MaxFrames>& stack() const { return backtrace_; }
};

// Business logic starts here
using MyExceptionErrsVoid = AlphaException<std::nullptr_t>;

struct Order {
    unsigned int id_;
    double value_;
};

std::ostream& operator<<(std::ostream& os, const Order& order) {
    os << "value : " << order.value_;
    return os;
}

// Global storage for orders
std::map<unsigned int, Order> orders{{1,Order{1,2.0}},{11, Order{11,5.0}}};

// Thread-local storage for the stack trace
thread_local HybridStackTrace<32> current_trace;

Order UpdateOrder(const Order& ord) {
    // Create a guard that will automatically manage the dynamic frame
    StackFrameGuard guard(current_trace);
    
    auto it = orders.find(ord.id_);
    if(it == orders.end())
        throw MyExceptionErrsVoid("update error : ", 
                                 nullptr, 
                                 CompileTimeFrame{},
                                 current_trace);
    return it->second = ord;
}

Order findOrder(unsigned int id) {
    StackFrameGuard guard(current_trace);
    
    auto it = orders.find(id);
    if(it == orders.end())
        throw MyExceptionErrsVoid("Bad Order id", 
                                 nullptr, 
                                 CompileTimeFrame{},
                                 current_trace);
    return it->second;
}

bool processOrder(unsigned int id) {
    StackFrameGuard guard(current_trace);
    
    try {
        Order ord = findOrder(id);
        std::cout << "Found order id : " << id << " : " << ord << std::endl;
        return true;
    }
    catch(MyExceptionErrsVoid& e) {
        std::cout << "[where] Failed to process : " << e.what() 
                 << " : " << e.where() << std::endl;
        std::cout << "[stack] Failed to process : " << e.what() << "\n" 
                 << e.stack() << std::endl;
    }
    return false;
}

int main() {
    try {
        unsigned int id = 10;
        // Create a guard for main's stack frame
        StackFrameGuard main_guard(current_trace);
        
        if(processOrder(id) == 1)
            std::cout << "success" << std::endl;
    }
    catch(...) {
        std::cout << "Unknown exception" << std::endl;
    }
    std::cout << "End" << std::endl;
    return 0;
}
