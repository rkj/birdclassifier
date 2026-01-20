# Modernization Plan

**Project**: Bird Species Classifier (BSC)
**Goal**: Incrementally modernize 2006 C++ codebase to modern C++17 standards
**Strategy**: PR-by-PR approach with tests for each change
**Timeline**: Estimated 20-25 PRs over multiple weeks

---

## Table of Contents

1. [Overview](#overview)
2. [Modernization Principles](#modernization-principles)
3. [Phase 1: Foundation (PRs 1-5)](#phase-1-foundation-prs-1-5)
4. [Phase 2: Core Modernization (PRs 6-12)](#phase-2-core-modernization-prs-6-12)
5. [Phase 3: Advanced Features (PRs 13-18)](#phase-3-advanced-features-prs-13-18)
6. [Phase 4: Polish & Optimization (PRs 19-25)](#phase-4-polish--optimization-prs-19-25)
7. [Testing Strategy](#testing-strategy)
8. [Success Criteria](#success-criteria)

---

## Overview

This plan transforms the BSC codebase while maintaining functionality at every step. Each PR is:
- **Focused**: Single concern or file group
- **Tested**: Includes tests before and after changes
- **Reviewable**: Small enough for thorough review
- **Functional**: Compiles and runs after each PR

### Current State
- C++98/03 code from 2006
- Raw pointers everywhere
- Manual memory management
- No tests
- qmake build system
- Some critical safety issues

### Target State
- Modern C++17 codebase
- Smart pointers and RAII
- Comprehensive test coverage (70%+)
- CMake build system
- Memory-safe and type-safe
- CI/CD pipeline

---

## Modernization Principles

1. **Safety First**: Fix critical issues (buffer overflows, memory leaks) immediately
2. **Test Coverage**: Add tests before refactoring when possible
3. **Incremental Changes**: Small, focused PRs
4. **Backward Compatibility**: Maintain existing functionality
5. **Documentation**: Update docs with each change
6. **No Regressions**: Every PR must compile and pass tests

---

## Phase 1: Foundation (PRs 1-5)

### PR #1: Setup Testing Framework ✅ Next

**Branch**: `modernize/01-setup-testing`
**Estimated Effort**: 4-6 hours
**Files Changed**: New files only

**Tasks**:
- [ ] Add Google Test as submodule or vendored dependency
- [ ] Create `tests/` directory structure
- [ ] Add CMakeLists.txt for building tests
- [ ] Update .pro file to support tests (optional)
- [ ] Create initial test file: `tests/test_main.cpp`
- [ ] Write 3-5 basic tests for existing functions
- [ ] Update .gitignore for test artifacts
- [ ] Document how to run tests in INSTALL.md

**New Files**:
```
tests/
├── CMakeLists.txt
├── test_main.cpp
├── test_audio.cpp (basic tests)
└── README.md
CMakeLists.txt (root)
```

**Acceptance Criteria**:
- [ ] Tests compile and run successfully
- [ ] At least 5 passing tests
- [ ] Documentation updated
- [ ] CI-friendly (can run headless)

---

### PR #2: Fix Critical Buffer Overflows

**Branch**: `modernize/02-fix-buffer-overflows`
**Estimated Effort**: 3-4 hours
**Files Changed**: `detect/Audio.cpp` (window functions)

**Critical Issues** (from CODE_REVIEW.md #2):
- HammingWindow, HanningWindow, BlackmanWindow use `static T tab[4096]`
- No bounds checking when `n > 4096`

**Changes**:
```cpp
// Before:
template <class T>
void HammingWindow(T* in, T* out, int n) {
    static T tab[4096] = {-1};
    // ...
}

// After:
template <class T>
void HammingWindow(T* in, T* out, int n) {
    assert(n <= 4096 && "Window size exceeds maximum");
    static std::array<T, 4096> tab;
    static bool initialized = false;
    if (!initialized) {
        // initialize...
        initialized = true;
    }
    // ...
}
```

**Tests**:
- [ ] Test with n = 256 (normal case)
- [ ] Test with n = 4096 (boundary)
- [ ] Test with n = 512 (another FFT_SIZE)
- [ ] Verify window function correctness

**Acceptance Criteria**:
- [ ] All buffer overflow risks eliminated
- [ ] Tests pass
- [ ] No performance regression
- [ ] Assertions in debug mode

---

### PR #3: Fix Unsafe String Operations

**Branch**: `modernize/03-fix-string-safety`
**Estimated Effort**: 2-3 hours
**Files Changed**: `detect/Audio.hxx`, `detect/Audio.cpp`

**Critical Issue** (CODE_REVIEW.md #3):
- `sprintf` with fixed buffer in `getBirdAndId()`

**Changes**:
```cpp
// Before:
string getBirdAndId() {
    char buf[60];
    sprintf(buf, "%s (%d-%d)", birdPolishNameFromId(birdId), birdId, id);
    return buf;
}

// After:
string getBirdAndId() const {
    std::ostringstream oss;
    oss << birdPolishNameFromId(birdId) << " (" << birdId << "-" << id << ")";
    return oss.str();
}
```

**Additional Changes**:
- Fix all other `sprintf` calls in codebase
- Add `const` correctness to getter methods

**Tests**:
- [ ] Test getBirdAndId() with normal names
- [ ] Test with very long bird names
- [ ] Test with special characters
- [ ] Verify no buffer overflows

**Acceptance Criteria**:
- [ ] No sprintf/strcpy/strcat in codebase
- [ ] All string operations safe
- [ ] Tests pass
- [ ] Const correctness improved

---

### PR #4: Remove Global Mutable State

**Branch**: `modernize/04-remove-globals`
**Estimated Effort**: 3-4 hours
**Files Changed**: `detect/Audio.cpp`, `detect/Audio.hxx`, `detect/detect.cpp`, `MainWindow.cpp`

**Critical Issue** (CODE_REVIEW.md #4):
- Global `SNR_MIN` variable

**Changes**:
```cpp
// Before:
double SNR_MIN = 3.0;  // Global

// After:
// In Audio.hxx
struct AudioConfig {
    double snrMin = 3.0;
    double powerCutoff = 1e-04;
    double cutoffThreshold = 0.255;

    static AudioConfig& getInstance() {
        static AudioConfig instance;
        return instance;
    }
};

// Or pass as parameter:
void normalize(const AudioConfig& config);
```

**Tests**:
- [ ] Test with default config
- [ ] Test with custom config
- [ ] Verify thread-safety (if applicable)
- [ ] Test configuration changes don't affect other instances

**Acceptance Criteria**:
- [ ] No global mutable state
- [ ] Configuration encapsulated
- [ ] Tests pass
- [ ] Backward compatible (CLI parameters still work)

---

### PR #5: Fix Header Pollution

**Branch**: `modernize/05-fix-headers`
**Estimated Effort**: 2-3 hours
**Files Changed**: `detect/Audio.hxx`, `detect/detect.hxx`, `detect/Manager.hxx`, `detect/Filter.hxx`, `detect/Files.hxx`

**Medium Issue** (CODE_REVIEW.md #6, #7):
- `using namespace std;` in headers
- Duplicate includes

**Changes**:
```cpp
// Before (Audio.hxx):
using namespace std;
#include <map>
// ...
#include <map>  // duplicate!
#include <string>
// ...
#include <string>  // duplicate!

// After:
// No using namespace
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>
```

**Tests**:
- [ ] Ensure all files still compile
- [ ] No namespace pollution
- [ ] Existing tests still pass

**Acceptance Criteria**:
- [ ] No `using namespace std` in any header
- [ ] No duplicate includes
- [ ] All files compile
- [ ] Tests pass

---

## Phase 2: Core Modernization (PRs 6-12)

### PR #6: Modernize CSignal Class - Part 1 (Memory)

**Branch**: `modernize/06-csignal-memory`
**Estimated Effort**: 6-8 hours
**Files Changed**: `detect/Audio.hxx`, `detect/Audio.cpp`

**Focus**: Replace raw pointers with modern containers

**Changes**:
```cpp
// Before:
class CSignal {
protected:
    double *frames;
    int framesCount;

public:
    ~CSignal() {
        if (frames != NULL) {
            delete [] frames;
        }
    }
};

// After:
class CSignal {
protected:
    std::vector<double> frames;

public:
    // Destructor now automatic (Rule of Zero)
    ~CSignal() = default;

    // Add move semantics
    CSignal(CSignal&& other) noexcept = default;
    CSignal& operator=(CSignal&& other) noexcept = default;
};
```

**Tests**:
- [ ] Test signal creation
- [ ] Test copy operations
- [ ] Test move operations
- [ ] Test memory management (valgrind)
- [ ] Test with large audio files
- [ ] Benchmark performance vs old version

**Acceptance Criteria**:
- [ ] No memory leaks (valgrind clean)
- [ ] RAII compliance
- [ ] Tests pass
- [ ] Performance within 5% of original

---

### PR #7: Modernize CSignal Class - Part 2 (Const Correctness)

**Branch**: `modernize/07-csignal-const`
**Estimated Effort**: 3-4 hours
**Files Changed**: `detect/Audio.hxx`, `detect/Audio.cpp`

**Focus**: Add const correctness

**Changes**:
```cpp
// Before:
string& getName() { return name; }
double* getFrames() { return frames; }
int getFramesCount() { return framesCount; }

// After:
const std::string& getName() const { return name; }
const std::vector<double>& getFrames() const { return frames; }
size_t getFramesCount() const { return frames.size(); }

// For mutable access when needed:
std::vector<double>& getFramesMutable() { return frames; }
```

**Tests**:
- [ ] Test const object access
- [ ] Test const method calls
- [ ] Verify compilation with const references
- [ ] Test that non-const still works

**Acceptance Criteria**:
- [ ] All getters marked const
- [ ] Const references used appropriately
- [ ] Tests pass
- [ ] No const_cast abuse

---

### PR #8: Modernize CSample Class

**Branch**: `modernize/08-csample`
**Estimated Effort**: 6-8 hours
**Files Changed**: `detect/Audio.hxx`, `detect/Audio.cpp`

**Focus**: Smart pointers and proper resource management

**Changes**:
```cpp
// Before:
class CSample : public CSignal {
private:
    SFrequencies *frequencies;
    OrigFrequencies *origFrequencies;

public:
    ~CSample() {
        // Manual cleanup
    }
};

// After:
class CSample : public CSignal {
private:
    std::vector<SFrequencies> frequencies;
    std::vector<OrigFrequencies> origFrequencies;

public:
    // Automatic cleanup via Rule of Zero
    ~CSample() = default;
};
```

**Tests**:
- [ ] Test sample creation from audio
- [ ] Test frequency computation
- [ ] Test differ() method
- [ ] Test consume() method
- [ ] Memory leak tests
- [ ] Performance tests

**Acceptance Criteria**:
- [ ] No raw pointers in CSample
- [ ] RAII compliant
- [ ] Tests pass
- [ ] Memory safe

---

### PR #9: Modernize CFFT Singleton

**Branch**: `modernize/09-cfft`
**Estimated Effort**: 4-5 hours
**Files Changed**: `detect/Audio.hxx`, `detect/Audio.cpp`

**Focus**: Thread-safe singleton and modern patterns

**Changes**:
```cpp
// Before:
class CFFT {
private:
    CFFT();
    double * in;
    double * out;
    fftw_plan rplan;

public:
    static CFFT& getInstance() {
        static CFFT singleton;
        return singleton;
    }
};

// After:
class CFFT {
private:
    CFFT();
    std::unique_ptr<double[]> in;
    std::unique_ptr<double[]> out;
    fftw_plan rplan;

public:
    ~CFFT();

    // Delete copy and move
    CFFT(const CFFT&) = delete;
    CFFT& operator=(const CFFT&) = delete;
    CFFT(CFFT&&) = delete;
    CFFT& operator=(CFFT&&) = delete;

    static CFFT& getInstance() noexcept {
        static CFFT singleton;
        return singleton;
    }
};
```

**Tests**:
- [ ] Test FFT computation accuracy
- [ ] Test singleton instance uniqueness
- [ ] Test thread safety (if applicable)
- [ ] Benchmark performance
- [ ] Memory leak tests

**Acceptance Criteria**:
- [ ] Thread-safe singleton
- [ ] No memory leaks
- [ ] Tests pass
- [ ] FFT results identical to original

---

### PR #10: Fix Exception Safety

**Branch**: `modernize/10-exceptions`
**Estimated Effort**: 5-6 hours
**Files Changed**: `detect/Audio.cpp`, new `detect/Exceptions.hxx`

**Focus**: Proper exception hierarchy and RAII

**Changes**:
```cpp
// New file: detect/Exceptions.hxx
class AudioException : public std::runtime_error {
public:
    explicit AudioException(const std::string& msg)
        : std::runtime_error(msg) {}
};

class FileNotFoundException : public AudioException {
public:
    explicit FileNotFoundException(const std::string& filename)
        : AudioException("File not found: " + filename) {}
};

class InvalidFormatException : public AudioException {
    // ...
};

// Before:
void loadAudio(const string& filename) {
    SNDFILE* file = sf_open(fn, SFM_READ, &sf_info);
    if (file == NULL) {
        throw string("File not found");
    }
    double *tmp = new double[framesCount*sf_info.channels];
    // ...
}

// After:
void loadAudio(const std::string& filename) {
    SF_INFO sf_info{};
    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sf_info);
    if (!file) {
        throw FileNotFoundException(filename);
    }

    std::vector<double> tmp(framesCount * sf_info.channels);
    // ... automatic cleanup on exception
}
```

**Tests**:
- [ ] Test exception catching
- [ ] Test RAII cleanup on exception
- [ ] Test exception messages
- [ ] Memory leak tests with exceptions
- [ ] Test invalid file formats

**Acceptance Criteria**:
- [ ] Custom exception hierarchy
- [ ] No resource leaks on exception
- [ ] Tests pass
- [ ] Proper error messages

---

### PR #11: Modernize Files Module

**Branch**: `modernize/11-files`
**Estimated Effort**: 4-5 hours
**Files Changed**: `detect/Files.hxx`, `detect/Files.cpp`

**Focus**: RAII file handling

**Changes**:
```cpp
// Create RAII wrapper for SNDFILE
class AudioFileHandle {
    SNDFILE* file;

public:
    explicit AudioFileHandle(const std::string& path, int mode, SF_INFO* info)
        : file(sf_open(path.c_str(), mode, info)) {
        if (!file) {
            throw FileNotFoundException(path);
        }
    }

    ~AudioFileHandle() {
        if (file) {
            sf_close(file);
        }
    }

    // Delete copy, allow move
    AudioFileHandle(const AudioFileHandle&) = delete;
    AudioFileHandle& operator=(const AudioFileHandle&) = delete;
    AudioFileHandle(AudioFileHandle&& other) noexcept;
    AudioFileHandle& operator=(AudioFileHandle&& other) noexcept;

    SNDFILE* get() const { return file; }
};
```

**Tests**:
- [ ] Test WAV file loading
- [ ] Test MP3 file loading
- [ ] Test file writing
- [ ] Test invalid files
- [ ] Test resource cleanup
- [ ] Memory leak tests

**Acceptance Criteria**:
- [ ] RAII file handling
- [ ] No file descriptor leaks
- [ ] Tests pass
- [ ] Exception safe

---

### PR #12: Modernize Filter Module

**Branch**: `modernize/12-filter`
**Estimated Effort**: 3-4 hours
**Files Changed**: `detect/Filter.hxx`, `detect/Filter.cpp`

**Focus**: Modern C++ patterns

**Changes**:
- Replace raw arrays with vectors
- Add const correctness
- Improve interface

**Tests**:
- [ ] Test bandpass filter
- [ ] Test filter accuracy
- [ ] Test edge cases
- [ ] Performance tests

**Acceptance Criteria**:
- [ ] Modern C++ code
- [ ] Tests pass
- [ ] No performance regression

---

## Phase 3: Advanced Features (PRs 13-18)

### PR #13: Add CMake Build System

**Branch**: `modernize/13-cmake`
**Estimated Effort**: 6-8 hours
**Files Changed**: New `CMakeLists.txt` files

**Tasks**:
- [ ] Create root CMakeLists.txt
- [ ] Configure Qt4 detection
- [ ] Configure library dependencies
- [ ] Set up test builds
- [ ] Add install targets
- [ ] Create FindFFTW3.cmake if needed
- [ ] Update INSTALL.md

**New Files**:
```
CMakeLists.txt
cmake/
├── FindFFTW3.cmake
└── FindLibSndFile.cmake
detect/CMakeLists.txt
Drawers/CMakeLists.txt
tests/CMakeLists.txt
```

**Acceptance Criteria**:
- [ ] Builds on Linux, macOS, Windows
- [ ] Tests build and run
- [ ] Install works
- [ ] Both qmake and CMake supported
- [ ] Documentation updated

---

### PR #14: Modernize Manager Class

**Branch**: `modernize/14-manager`
**Estimated Effort**: 5-6 hours
**Files Changed**: `detect/Manager.hxx`, `detect/Manager.cpp`

**Focus**: Modern C++ patterns, smart pointers

**Tests**:
- [ ] Test batch processing
- [ ] Test sample extraction
- [ ] Memory tests
- [ ] Performance tests

**Acceptance Criteria**:
- [ ] Modern C++ code
- [ ] Tests pass
- [ ] Memory safe

---

### PR #15: Modernize Detection Algorithms

**Branch**: `modernize/15-detect`
**Estimated Effort**: 6-8 hours
**Files Changed**: `detect/detect.hxx`, `detect/detect.cpp`

**Focus**: Modern C++ patterns, algorithms

**Changes**:
- Replace raw loops with STL algorithms
- Use lambdas where appropriate
- Add const correctness

**Tests**:
- [ ] Test classification accuracy
- [ ] Test cross-validation
- [ ] Test nearest neighbor
- [ ] Performance tests

**Acceptance Criteria**:
- [ ] Modern C++ code
- [ ] Classification accuracy unchanged
- [ ] Tests pass

---

### PR #16: Modernize Drawer Classes - Part 1

**Branch**: `modernize/16-drawers-1`
**Estimated Effort**: 5-6 hours
**Files Changed**: `Drawers/AudioDraw.*`, `Drawers/SignalDraw.*`

**Focus**: Modern C++ patterns in Qt code

**Tests**:
- [ ] GUI tests (if possible)
- [ ] Rendering correctness
- [ ] Memory tests

**Acceptance Criteria**:
- [ ] Modern C++ code
- [ ] GUI works correctly
- [ ] Tests pass

---

### PR #17: Modernize Drawer Classes - Part 2

**Branch**: `modernize/17-drawers-2`
**Estimated Effort**: 5-6 hours
**Files Changed**: `Drawers/Spectrogram.*`, `Drawers/SpectroDraw.*`, `Drawers/EnergyDraw.*`

**Focus**: Modern C++ patterns in visualization

**Tests**:
- [ ] Spectrogram rendering tests
- [ ] Color scheme tests
- [ ] Memory tests

**Acceptance Criteria**:
- [ ] Modern C++ code
- [ ] Visualization works correctly
- [ ] Tests pass

---

### PR #18: Modernize MainWindow - Part 1 (Refactor)

**Branch**: `modernize/18-mainwindow-refactor`
**Estimated Effort**: 8-10 hours
**Files Changed**: `MainWindow.hxx`, `MainWindow.cpp`, new controller files

**Focus**: Break down God Object into controllers

**Changes**:
```cpp
// Extract responsibilities:
class AudioController {
    // Audio operations
};

class VisualizationController {
    // Drawing coordination
};

class LearningController {
    // Learning set management
};

class SettingsController {
    // Configuration
};

class MainWindow : public QMainWindow {
    std::unique_ptr<AudioController> audioController;
    std::unique_ptr<VisualizationController> vizController;
    std::unique_ptr<LearningController> learningController;
    std::unique_ptr<SettingsController> settingsController;
};
```

**Tests**:
- [ ] Integration tests
- [ ] Controller unit tests
- [ ] GUI functionality tests

**Acceptance Criteria**:
- [ ] MainWindow < 500 lines
- [ ] Clear separation of concerns
- [ ] Tests pass
- [ ] GUI works correctly

---

## Phase 4: Polish & Optimization (PRs 19-25)

### PR #19: Modernize MainWindow - Part 2 (Modern C++)

**Branch**: `modernize/19-mainwindow-modern`
**Estimated Effort**: 4-5 hours
**Files Changed**: `MainWindow.hxx`, `MainWindow.cpp`

**Focus**: Modern C++ in remaining code

**Tests**:
- [ ] GUI tests
- [ ] Integration tests

**Acceptance Criteria**:
- [ ] Modern C++ code
- [ ] Tests pass

---

### PR #20: Add Comprehensive Unit Tests

**Branch**: `modernize/20-comprehensive-tests`
**Estimated Effort**: 10-12 hours
**Files Changed**: All test files

**Focus**: Achieve 70%+ code coverage

**Tasks**:
- [ ] Add tests for all core algorithms
- [ ] Add edge case tests
- [ ] Add integration tests
- [ ] Add performance benchmarks
- [ ] Generate coverage report

**Acceptance Criteria**:
- [ ] 70%+ code coverage
- [ ] All critical paths tested
- [ ] Performance benchmarks established

---

### PR #21: Add Static Analysis and CI

**Branch**: `modernize/21-ci-pipeline`
**Estimated Effort**: 6-8 hours
**Files Changed**: New `.github/workflows/` or similar

**Tasks**:
- [ ] Add clang-tidy configuration
- [ ] Add cppcheck configuration
- [ ] Add CI workflow (GitHub Actions)
- [ ] Add coverage reporting
- [ ] Add automated builds
- [ ] Add format checking (clang-format)

**New Files**:
```
.clang-tidy
.clang-format
.github/
└── workflows/
    ├── build.yml
    ├── test.yml
    └── coverage.yml
```

**Acceptance Criteria**:
- [ ] CI runs on all PRs
- [ ] Static analysis integrated
- [ ] Coverage tracked
- [ ] Multi-platform builds

---

### PR #22: Performance Optimization

**Branch**: `modernize/22-optimization`
**Estimated Effort**: 6-8 hours
**Files Changed**: Various (hot paths)

**Focus**: Profile and optimize

**Tasks**:
- [ ] Profile with gprof/perf
- [ ] Optimize hot paths
- [ ] Add move semantics where beneficial
- [ ] Consider parallelization (OpenMP)
- [ ] Benchmark improvements

**Tests**:
- [ ] Performance benchmarks
- [ ] Correctness tests
- [ ] Memory usage tests

**Acceptance Criteria**:
- [ ] 10%+ performance improvement
- [ ] No correctness regressions
- [ ] Benchmarks document improvements

---

### PR #23: Documentation and Comments

**Branch**: `modernize/23-documentation`
**Estimated Effort**: 8-10 hours
**Files Changed**: All source files

**Focus**: Add comprehensive documentation

**Tasks**:
- [ ] Add Doxygen comments to all public APIs
- [ ] Document algorithms
- [ ] Add usage examples
- [ ] Generate API documentation
- [ ] Update README with modernization notes

**Acceptance Criteria**:
- [ ] All public APIs documented
- [ ] Doxygen generates without warnings
- [ ] Examples provided

---

### PR #24: Code Style and Consistency

**Branch**: `modernize/24-style`
**Estimated Effort**: 4-6 hours
**Files Changed**: All source files

**Focus**: Apply consistent formatting

**Tasks**:
- [ ] Create .clang-format config
- [ ] Format all code
- [ ] Fix naming inconsistencies
- [ ] Remove dead code
- [ ] Clean up commented code

**Acceptance Criteria**:
- [ ] All code formatted consistently
- [ ] No commented-out code
- [ ] Naming conventions followed

---

### PR #25: Final Polish and Release Prep

**Branch**: `modernize/25-final-polish`
**Estimated Effort**: 6-8 hours
**Files Changed**: Various

**Focus**: Final cleanup and release

**Tasks**:
- [ ] Update all documentation
- [ ] Create migration guide
- [ ] Update CHANGELOG
- [ ] Bump version to 1.0
- [ ] Create release notes
- [ ] Tag release
- [ ] Create binary releases

**Acceptance Criteria**:
- [ ] All tests pass
- [ ] Documentation complete
- [ ] Ready for v1.0 release

---

## Testing Strategy

### Test Pyramid

```
           /\
          /  \  Integration Tests (10-20%)
         /____\
        /      \  Unit Tests (70-80%)
       /________\
      /          \
     /__________\ E2E/GUI Tests (5-10%)
```

### Test Categories

1. **Unit Tests** (70-80% coverage target)
   - Individual functions
   - Class methods
   - Edge cases
   - Error conditions

2. **Integration Tests** (10-20%)
   - Module interactions
   - File I/O workflows
   - Classification pipeline
   - Learning set operations

3. **Performance Tests**
   - Benchmarks for critical paths
   - Memory usage tests
   - Regression tests

4. **GUI Tests** (5-10%)
   - Qt Test framework
   - Basic functionality
   - Regression tests

### Testing Tools

- **Google Test**: Unit testing framework
- **Google Benchmark**: Performance testing
- **Valgrind**: Memory leak detection
- **AddressSanitizer**: Memory error detection
- **Coverage**: gcov/lcov or similar

---

## Success Criteria

### Per-PR Criteria

Each PR must:
- ✅ Compile without warnings (with `-Wall -Wextra`)
- ✅ Pass all existing tests
- ✅ Add new tests for changed code
- ✅ Maintain or improve code coverage
- ✅ No memory leaks (valgrind clean)
- ✅ No performance regression (within 5%)
- ✅ Update relevant documentation
- ✅ Code review approved

### Overall Success Criteria

Project modernization is complete when:
- ✅ All 25 PRs merged
- ✅ 70%+ test coverage
- ✅ No valgrind errors
- ✅ CI pipeline green
- ✅ CMake build working
- ✅ All critical issues from CODE_REVIEW.md addressed
- ✅ Documentation complete
- ✅ Ready for v1.0 release

---

## Risk Management

### Potential Risks

1. **Breaking Changes**: Careful testing mitigates
2. **Performance Regression**: Benchmarks catch issues
3. **Build System Issues**: Keep qmake working initially
4. **Qt4 Compatibility**: May need Qt5 migration (future)
5. **Time Estimates**: Buffer 20-30% for unexpected issues

### Mitigation Strategies

- Small, incremental PRs
- Comprehensive testing
- Performance benchmarking
- Feature flags for risky changes
- Rollback plan (git revert)

---

## Timeline Estimate

**Conservative Estimate**:
- Phase 1 (5 PRs): 2-3 weeks
- Phase 2 (7 PRs): 4-5 weeks
- Phase 3 (6 PRs): 4-5 weeks
- Phase 4 (7 PRs): 4-5 weeks

**Total**: 14-18 weeks (3.5-4.5 months)

**Aggressive Estimate** (with dedicated focus):
- Phase 1: 1 week
- Phase 2: 2-3 weeks
- Phase 3: 2-3 weeks
- Phase 4: 2-3 weeks

**Total**: 7-10 weeks (2-2.5 months)

---

## PR Naming Convention

```
modernize/[number]-[short-description]

Examples:
- modernize/01-setup-testing
- modernize/02-fix-buffer-overflows
- modernize/06-csignal-memory
```

---

## Tracking Progress

Create GitHub project board with columns:
- 📋 Planned
- 🏗️ In Progress
- 👀 In Review
- ✅ Merged
- 🚫 Blocked

Track each PR as an issue/card.

---

## Notes

- Each PR should be merged to main after review
- Keep main branch always buildable
- Tag after major milestones
- Consider feature branches for experimental work
- Communicate breaking changes clearly

---

**Plan Version**: 1.0
**Created**: 2026-01-18
**Last Updated**: 2026-01-18
**Status**: Ready to begin with PR #1
