# AlphaException: A Multi-Platform Stack Trace System

## Introduction

The AlphaException system represents an evolution in C++ exception handling, building upon the concepts presented by Peter Muldoon in his OmegaException work at CppCon 2023. While OmegaException demonstrated the power of enhanced exception handling with stack traces, AlphaException takes these ideas further by providing a platform-independent implementation that works across different compilers and environments.

The name "Alpha" symbolizes how this implementation continues from where OmegaException left off, wrapping around the alphabet to suggest an evolutionary step forward. This implementation focuses particularly on compatibility with modern compilers and build environments, with special attention paid to Clang and libc++ integration.

## Core Features and Design Philosophy

AlphaException addresses several real-world challenges that developers face when implementing exception handling:

1. Platform Independence: Not all development environments support external libraries like libbacktrace
2. Compiler Compatibility: Different compilers handle stack traces and memory management differently
3. Resource Management: Some systems have strict memory or processing limitations
4. Build System Integration: Development environments require different levels of debugging support
5. Production Readiness: The system must balance debugging capabilities with performance

## Implementation Details

### Stack Trace Modes

The system provides three levels of stack trace support:

1. Full Stack Trace (Mode 2):
   - Uses comprehensive stack trace information
   - Available in debug builds on supported platforms
   - Provides maximum debugging information
   - Requires additional library support

2. Custom Stack Trace (Mode 1):
   - Uses a lightweight custom implementation
   - Tracks explicitly marked functions
   - Balances performance and debugging capability
   - Works across all platforms with sufficient resources

3. Minimal Stack Trace (Mode 0):
   - Captures only current location information
   - Zero dynamic memory allocation
   - Suitable for highly constrained environments
   - Minimal runtime overhead

### Compiler-Specific Considerations

When using Clang (the primary supported compiler), several important implementation details come into play:

1. Stack Trace Header:
```cpp
#if defined(__clang__)
    #if defined(_LIBCPP_VERSION) && defined(DEBUG_BUILD)
        #define FULL_STACK_TRACE_CAPABLE 1
        #define USE_LIBCXX_STACKTRACE 1
        #include <__stacktrace/stacktrace.h>
```

This implementation detail reflects libc++'s internal organization of its stack trace support.

## Building and Compilation

### Primary Build Configuration (Clang)

```bash
# Debug build with full stack trace support
clang++ -std=c++23 -DDEBUG_BUILD -pthread -lstdc++_libbacktrace -fsized-deallocation program.cpp

# Production build
clang++ -std=c++23 -O3 -pthread program.cpp

# Resource-constrained build
clang++ -std=c++23 -DMINIMAL_RESOURCES -O3 -pthread program.cpp
```

The flags serve specific purposes:
- `-std=c++23`: Enables C++23 features, including stack trace support
- `-DDEBUG_BUILD`: Activates full stack trace functionality
- `-pthread`: Enables thread support for thread-local storage
- `-lstdc++_libbacktrace`: Links against libbacktrace for stack unwinding
- `-fsized-deallocation`: Resolves memory management compatibility issues

### Alternative Compiler Support (GCC)

While Clang is the primary supported compiler, GCC is also supported:

```bash
# Debug build
g++ -DDEBUG_BUILD -std=c++23 program.cpp -lstdc++_libbacktrace -pthread

# Production build
g++ -O3 -std=c++23 program.cpp -pthread
```

## Usage Examples

### Basic Exception Handling
```cpp
void process_data() {
    TRACE_FUNCTION();  // Add stack trace support
    try {
        validate_input();
    } catch (const AlphaException<std::string>& e) {
        std::cout << "Error details:\n" << e << std::endl;
    }
}
```

### Custom Data in Exceptions
```cpp
struct OrderError {
    int order_id;
    std::string error_type;
};

void process_order(int id) {
    TRACE_FUNCTION();
    if (invalid_order(id)) {
        throw AlphaException<OrderError>(
            "Order processing failed",
            OrderError{id, "validation_error"}
        );
    }
}
```

## Stack Trace Output Examples

### Full Stack Trace (Debug Build)
```
Exception: Invalid parameter
Location: input.cpp(145:23) `validateInput`
Stack trace:
[0] myapp::validateInput(std::string) at input.cpp:145
[1] std::__cxx11::basic_string::_M_create at string.cpp:89
[2] myapp::processRequest at request.cpp:72
[3] main at main.cpp:23
```

### Custom Stack Trace (Production)
```
Exception: Invalid parameter
Location: input.cpp(145:23) `validateInput`
Stack trace:
[0] validateInput at input.cpp:145
[1] processRequest at request.cpp:72
[2] main at main.cpp:23
```

### Minimal Stack Trace
```
Exception: Invalid parameter
Location: input.cpp(145:23) `validateInput`
```

## Performance Characteristics

### Memory Usage
- Full Stack Trace: ~1KB per stack frame
- Custom Stack Trace: ~100 bytes per tracked frame
- Minimal Stack Trace: ~40 bytes fixed

### Runtime Overhead
- Full Stack Trace: Stack unwinding cost on exception
- Custom Stack Trace: Minor overhead for frame tracking
- Minimal Stack Trace: Negligible overhead

## Best Practices

### Development Environment
1. Use full stack traces with Clang's debug build configuration
2. Enable all relevant compiler warnings
3. Build with debug information enabled
4. Test exception paths thoroughly

### Production Environment
1. Choose appropriate stack trace mode based on requirements
2. Enable compiler optimizations
3. Consider resource constraints
4. Monitor performance impact

### Function Tracking Guidelines
1. Mark error-prone functions with TRACE_FUNCTION()
2. Track major API boundaries
3. Consider performance impact in tight loops
4. Avoid tracking simple utility functions

## Known Limitations and Considerations

1. Compiler Support:
   - Primary support for Clang with specific compilation flags
   - Secondary support for GCC
   - Platform-specific considerations for other compilers

2. Memory Management:
   - Requires sized deallocation support in Clang builds
   - Different behavior across debug and release modes
   - Resource usage varies by stack trace mode

3. Build System Integration:
   - Requires specific compiler flags for full functionality
   - May need environment-specific configuration
   - Build system must handle different compilation modes

## Credits and References

This implementation builds upon the OmegaException concept presented by Peter Muldoon at CppCon 2023 in his talk "Exceptionally Bad: The Misuse of Exceptions in C++ & How to Do Better" (https://www.youtube.com/watch?v=Oy-VTqz1_58).

## Version Information

- Current Version: 1.0.0
- Last Updated: December 2024
- Primary Compiler Support: Clang 18.1.0 and later
- Secondary Compiler Support: GCC 13.2.0 and later


## 10. Running Examples:

To run these, try following compiler flags. Note that the full stack trace has only been tested with x86-64 gcc 13.2 and x86-64 clang 18.1.0, as libbacktrace is not built for every version of every compiler on godbolt.
[https://godbolt.org/z/oE1GvKPqK](https://godbolt.org/z/ha4TcE499)


### Debug build with full stack trace
-std=c++23 -DDEBUG_BUILD -pthread -lstdc++_libbacktrace

NOTE: If using clang, the `-fsized-deallocation` flag must also be added

### Production build with custom stack trace
-std=c++23 -O3 -pthread

### Minimal build
-std=c++20 -DMINIMAL_RESOURCES -O3

### License
[TBD]

This documentation and the associated implementation provide a complete guide to understanding and using the AlphaException system across different platforms and build environments. The system's flexibility allows it to adapt to various development scenarios while maintaining a consistent interface for exception handling and debugging.
