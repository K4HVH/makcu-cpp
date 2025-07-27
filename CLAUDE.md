# CLAUDE.md - makcu-cpp Project

## Project Overview
High-performance C++ library for MAKCU mouse control with async capabilities. This project aims to modernize async patterns and eliminate fake-async implementations.

## Build & Test Commands

```bash
# Build the project
./build.sh

# Run tests (if available)
# Note: No specific test framework found - check with user for testing approach

# Lint/Format (if available)
# Note: No linting tools configured - consider adding clang-format, clang-tidy
```

## Testing Requirements

**CRITICAL**: All implementations must be thoroughly tested before completion, including:

- Unit tests for all async functionality
- Integration tests with real hardware
- Performance benchmarks comparing sync vs async performance
- Memory leak detection for async operations
- **IntelliSense error checking** - All code must compile without warnings
- Cross-platform compatibility testing (Windows/Linux)

## Architecture

### Core Components

- **Device** (`makcu-cpp/include/makcu.h`, `makcu-cpp/src/makcu.cpp`) - Main device interface with mixed sync/async methods
- **SerialPort** (`makcu-cpp/include/serialport.h`, `makcu-cpp/src/serialport.cpp`) - Serial communication with command tracking
- **Main Demo** (`makcu-cpp/main.cpp`) - Usage examples and performance benchmarks

### Current Async Issues Identified

#### ❌ Fake-Async Patterns (High Priority Fixes)

1. **Immediate Promise Resolution** - `makcu.cpp:226-248`
   - `executeCommandAsync()` fires command and immediately returns resolved promise
   - No real concurrency benefit

2. **std::async Overuse** - `makcu.cpp:391-393, 421-423`
   - `connectAsync()` and `disconnectAsync()` use heavyweight threads unnecessarily
   - Inefficient for simple I/O operations

3. **Sync-wrapped-as-Async** - Multiple `*Async()` methods
   - Call synchronous methods then wrap result in promise
   - No actual async execution

#### ✅ Legitimate Async Components

- SerialPort command tracking with future-based correlation
- Background listener thread for serial I/O
- Proper timeout and cleanup mechanisms

## Modernization Roadmap

### Phase 1: C++20/23 Coroutine Integration

```cpp
// Target implementation pattern:
Task<bool> Device::clickAsync(MouseButton button) {
    co_return co_await m_impl->executeCommandCoroutine(getClickCommand(button));
}
```

### Phase 2: Async Executor Framework

- Add configurable async executor
- Create separate `DeviceAsync` class for pure async interface
- Implement proper thread pool management

### Phase 3: Replace Fake-Async

- Convert `executeCommandAsync()` to use proper async I/O
- Implement `clickSequenceAsync()` with coroutine-based sequencing
- Add async socket operations for connection methods

### Phase 4: Async Streams

- Add `AsyncStream<ButtonEvent>` for continuous button monitoring
- Implement `AsyncStream<std::string>` for raw response data
- Zero-copy async data processing

## Performance Targets

- **Current**: Fake-async provides no real performance benefit
- **Target**: 10-100x improvement in throughput for batched operations
- **Latency**: Sub-millisecond response times with proper async I/O
- **Memory**: Reduced thread overhead, better cache locality

## Dependencies

- C++23 compiler support
- Platform-specific serial libraries:
  - Windows: SetupAPI, Windows.h
  - Linux: termios, libudev
- Threading: std::thread, std::future
- **Recommended additions**:
  - C++20 coroutines support
  - io_uring (Linux) / IOCP (Windows) for async I/O

## Key Files to Modernize

1. `makcu-cpp/src/makcu.cpp:226-248` - Replace fake executeCommandAsync
2. `makcu-cpp/src/makcu.cpp:391-393` - Fix connectAsync std::async overuse
3. `makcu-cpp/src/makcu.cpp:543-568` - Fix clickAsync fake pattern
4. `makcu-cpp/include/makcu.h:126-248` - Add proper async method signatures

## Reference Implementation

- Rust makcu-rs: [https://github.com/whatzwastaken/makcu-rs](https://github.com/whatzwastaken/makcu-rs)
- Uses Tokio async runtime with proper async/await patterns
- Clean separation between sync and async APIs

## Notes for Development

- Maintain backwards compatibility during async modernization
- Consider adding async feature flag for gradual migration
- Focus on gaming performance - sub-millisecond latencies required
- Preserve existing command caching and optimization patterns
