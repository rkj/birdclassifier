# Tests

Unit and integration tests for Bird Species Classifier.

## Running Tests

### Using CMake (Recommended)

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
make

# Run tests
make test

# Or run with verbose output
ctest --output-on-failure

# Or run test executable directly
./bin/test_audio
```

### Using Custom Target

```bash
# Build and run tests in one command
make check
```

## Test Structure

```
tests/
├── CMakeLists.txt       # Test build configuration
├── README.md            # This file
├── test_main.cpp        # Main entry point for all tests
└── test_audio.cpp       # Audio module tests
```

## Test Categories

### Unit Tests (`test_audio.cpp`)

- **Power Computation Tests**: Verify energy calculation
- **Window Function Tests**: Hamming, Blackman window correctness
- **SFrequencies Tests**: Frequency data structure operations
- **CFFT Tests**: FFT singleton and operations
- **Edge Case Tests**: Boundary conditions and error handling

### Test Fixtures

Tests use Google Test fixtures for common setup/teardown:

```cpp
class AudioTest : public ::testing::Test {
protected:
    void SetUp() override { /* setup */ }
    void TearDown() override { /* cleanup */ }
};
```

## Writing New Tests

### Basic Test

```cpp
TEST_F(AudioTest, TestName) {
    // Arrange
    double input[] = {1.0, 2.0, 3.0};

    // Act
    double result = computePower(input, 3);

    // Assert
    EXPECT_DOUBLE_EQ(result, 4.666666);
}
```

### Assertions

```cpp
EXPECT_EQ(a, b);           // a == b
EXPECT_NE(a, b);           // a != b
EXPECT_LT(a, b);           // a < b
EXPECT_LE(a, b);           // a <= b
EXPECT_GT(a, b);           // a > b
EXPECT_GE(a, b);           // a >= b

EXPECT_DOUBLE_EQ(a, b);    // Floating point equality
EXPECT_NEAR(a, b, eps);    // |a - b| < eps

EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

EXPECT_THROW(stmt, exc);   // stmt throws exc
EXPECT_NO_THROW(stmt);     // stmt doesn't throw
```

## Test Coverage

Current coverage:
- ✅ Power computation
- ✅ Window functions
- ✅ SFrequencies operations
- ✅ CFFT singleton
- ⚠️ File I/O (TODO)
- ⚠️ Signal loading (TODO)
- ⚠️ Classification (TODO)
- ⚠️ GUI components (TODO)

Target: 70%+ code coverage

## Test Requirements

### Dependencies

- Google Test (automatically fetched by CMake)
- Core BSC library (`bsc_core`)

### Compiler

- C++17 or later
- GCC 7+, Clang 5+, or MSVC 2017+

## Continuous Integration

Tests run automatically on:
- Every pull request
- Every commit to main branch
- Nightly builds

See `.github/workflows/` for CI configuration (when added).

## Debugging Tests

### Run specific test

```bash
./bin/test_audio --gtest_filter=AudioTest.ComputePowerBasicSignal
```

### List all tests

```bash
./bin/test_audio --gtest_list_tests
```

### Repeat test N times

```bash
./bin/test_audio --gtest_repeat=100
```

### Shuffle test order

```bash
./bin/test_audio --gtest_shuffle
```

### Break on failure

```bash
./bin/test_audio --gtest_break_on_failure
```

## Memory Testing

### Valgrind

```bash
valgrind --leak-check=full --show-leak-kinds=all ./bin/test_audio
```

### AddressSanitizer

```bash
# Rebuild with sanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make
./bin/test_audio
```

## Performance Testing

Basic performance tests are included to catch regressions.

For detailed benchmarking, see `benchmarks/` directory (future).

## Test-Driven Development

When adding new features:

1. Write failing test first
2. Implement minimum code to pass
3. Refactor while keeping tests green
4. Add edge case tests

## Resources

- [Google Test Documentation](https://google.github.io/googletest/)
- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Test FAQ](https://google.github.io/googletest/faq.html)

## Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for testing guidelines.

All new code should include tests.
Target: 70%+ coverage for core functionality.
