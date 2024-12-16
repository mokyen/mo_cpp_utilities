# AlphaException: A Multi-Platform Stack Trace System

## Table of Contents
1. Introduction and Background
2. System Architecture
3. Build Configurations
4. Usage Guide
5. Output Examples
6. Platform Support
7. Performance Considerations
8. Best Practices
9. Technical Details
10. Credits and License

## 1. Introduction and Background

The AlphaException system is an evolution of the OmegaException concept presented by Peter Muldoon at CppCon 2023 in his talk "Exceptionally Bad: The Misuse of Exceptions in C++ & How to Do Better". The name "Alpha" symbolizes how this implementation continues from where OmegaException left off, wrapping around the alphabet to suggest an evolutionary step forward.

Note that this requires C++20 or higher. The full stack version requires C++23 or higher.

### Why Extend OmegaException?

While the original OmegaException demonstrated powerful exception handling capabilities through libbacktrace integration, real-world development often presents additional challenges:

1. Platform Diversity: Many development environments, particularly embedded systems and bare metal platforms, cannot support libbacktrace
2. Resource Constraints: Some systems have strict memory or processing limitations
3. Build System Complexity: Different build systems and environments require different levels of debugging support

The AlphaException system addresses these challenges by providing a flexible, platform-independent implementation that automatically adapts to various development environments and constraints.

## 2. System Architecture

### Adaptive Implementation Levels

The system provides three distinct levels of stack trace support:

1. Full Stack Trace (Mode 2)
   - Utilizes libbacktrace for comprehensive stack information
   - Captures all function calls, including system libraries
   - Provides maximum debugging information
   - Available on supported platforms in debug builds

2. Custom Stack Trace (Mode 1)
   - Uses a lightweight custom implementation
   - Tracks only explicitly marked functions
   - Balances performance and debugging capability
   - Available on all platforms with sufficient resources

3. Minimal Stack Trace (Mode 0)
   - Captures only current location information
   - Zero dynamic memory allocation
   - Minimal runtime overhead
   - Suitable for highly constrained environments

### Key Components

The system consists of several key components:

1. CompileTimeFrame
   - Captures source location information at compile time
   - Includes function name, file name, line number, and column
   - Zero runtime overhead for location capture

2. StackTrace
   - Manages collection of stack frames
   - Thread-safe implementation
   - Configurable memory allocation strategy

3. RAII Guards
   - Automatic stack frame management
   - Exception-safe implementation
   - Minimal runtime overhead

## 3. Build Configurations

### GCC Builds

```bash
# Debug build with full stack trace support
g++ -DDEBUG_BUILD -g program.cpp -lstdc++_libbacktrace

# Production build with custom stack trace
g++ -O3 program.cpp

# Minimal resource build
g++ -DMINIMAL_RESOURCES -O3 program.cpp
```

### Clang Builds

```bash
# Debug build with full stack trace support
clang++ -std=c++23 -DDEBUG_BUILD -g program.cpp -lstdc++_libbacktrace

# Production build with custom stack trace
clang++ -std=c++23 -O3 program.cpp

# Minimal resource build
clang++ -std=c++23 -DMINIMAL_RESOURCES -O3 program.cpp

# Development build with all warnings
clang++ -Weverything -std=c++23 -DDEBUG_BUILD program.cpp

# With specific warning suppressions
clang++ -Weverything -Wno-c++98-compat -std=c++23 program.cpp
```

### Yocto Build Integration

#### Recipe Configuration (your-application.bb)
```bitbake
DEPENDS += "libbacktrace"

EXTRA_OEMAKE = "'CXXFLAGS=${CXXFLAGS} -DYOCTO_BUILD \
    ${@bb.utils.contains('DEPENDS', 'libbacktrace', '-DHAVE_LIBBACKTRACE', '', d)}'"

# For debug builds
DEBUG_OPTIMIZATION = "-O0 -g -DDEBUG_BUILD"
```

#### Development Image Configuration
```bitbake
# local.conf or image recipe
IMAGE_INSTALL:append = " libbacktrace libbacktrace-dev"
EXTRA_CXXFLAGS:append = " -DDEBUG_BUILD"
```

### Bare Metal Builds

```bash
# ARM targets
arm-none-eabi-g++ -DMINIMAL_RESOURCES program.cpp

# AVR targets
avr-g++ -DMINIMAL_RESOURCES program.cpp
```

## 4. Usage Guide

### Basic Exception Handling

```cpp
void process_data() {
    TRACE_FUNCTION();  // Add stack trace support to this function
    
    try {
        validate_input();
    } catch(const AlphaException<void*>& e) {
        std::cout << "Error details:\n" << e << std::endl;
    }
}
```

### Creating Custom Exceptions

```cpp
// Define exception with custom data
struct OrderError {
    int order_id;
    std::string error_type;
};

using OrderException = AlphaException<OrderError>;

void process_order(int id) {
    TRACE_FUNCTION();
    
    if (invalid_order(id)) {
        throw OrderException("Invalid order",
                           OrderError{id, "validation_error"});
    }
}
```

## 5. Output Examples

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
[1] processRequest at request.cpp:72  // Only tracked functions appear
[2] main at main.cpp:23
Missing: intermediateFunction         // Untracked functions not shown
```

### Minimal Stack Trace
```
Exception: Invalid parameter
Location: input.cpp(145:23) `validateInput`
```

## 6. Platform Support

### Desktop Platforms
- Full support on Linux, macOS, and Windows
- All three implementation levels available
- Best debugging experience with full stack traces

### Embedded Linux (Yocto)
- Configurable through Yocto recipes
- Can adapt based on image requirements
- Support for all implementation levels

### Bare Metal
- Automatic fallback to minimal implementation
- No external library dependencies
- Suitable for resource-constrained environments

## 7. Performance Considerations

### Memory Usage
- Full Stack Trace: ~1KB per stack frame
- Custom Stack Trace: ~100 bytes per tracked frame
- Minimal Stack Trace: ~40 bytes fixed

### CPU Overhead
- Full Stack Trace: Stack unwinding on exception
- Custom Stack Trace: Frame push/pop operations
- Minimal Stack Trace: Negligible

### Thread Safety Overhead
- Thread-local storage for independent stack traces
- Minimal synchronization requirements
- Lock-free design for maximum performance

## 8. Best Practices

### Development Environment
- Use full stack traces during development
- Enable all relevant compiler warnings
- Thoroughly test exception paths

### Production Environment
- Choose appropriate implementation level
- Monitor resource usage
- Track critical error paths

### Function Tracking Guidelines
- Track error-prone functions
- Track major API boundaries
- Track complex algorithmic code
- Avoid tracking simple utility functions
- Consider performance impact in tight loops

## 9. Technical Details

### Thread Safety
The implementation ensures thread safety through:
- Thread-local storage for stack traces
- RAII-based frame management
- Atomic operations where necessary
- Lock-free design principles

### Memory Management
- Stack frames allocated on demand
- RAII ensures proper cleanup
- No memory leaks on exception paths
- Configurable allocation strategies

### Platform Detection
The system uses preprocessor definitions to detect:
- Operating system type
- Compiler capabilities
- Available libraries
- Resource constraints

## 10. Running Examples:

To run these, try following compiler flags. Note that the full stack trace has only been tested with x86-64 gcc 13.2, as libbacktrace is not built for every version of every compiler.
https://godbolt.org/z/oE1GvKPqK

Note that this currently only works with gcc for the full stack trace.

# Debug build with full stack trace
-std=c++23 -DDEBUG_BUILD -pthread -lstdc++_libbacktrace

# Production build with custom stack trace
-std=c++23 -O3 -pthread

# Minimal build
-std=c++20 -DMINIMAL_RESOURCES -O3

## 10. Credits and License

This implementation is based on the OmegaException concept presented by Peter Muldoon at CppCon 2023. While it builds upon the original concepts, the AlphaException system is an independent implementation focusing on platform independence and adaptability.

Reference: Muldoon, P. (2023). "Exceptionally Bad: The Misuse of Exceptions in C++ & How to Do Better". CppCon 2023. https://www.youtube.com/watch?v=Oy-VTqz1_58

### License
[TBD]

This documentation and the associated implementation provide a complete guide to understanding and using the AlphaException system across different platforms and build environments. The system's flexibility allows it to adapt to various development scenarios while maintaining a consistent interface for exception handling and debugging.
