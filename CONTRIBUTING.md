# Contributing to Bird Species Classifier

Thank you for your interest in contributing to Bird Species Classifier (BSC)! This document provides guidelines and best practices for contributing to the project.

---

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Setup](#development-setup)
4. [Coding Standards](#coding-standards)
5. [Submitting Changes](#submitting-changes)
6. [Reporting Bugs](#reporting-bugs)
7. [Suggesting Features](#suggesting-features)
8. [Code Review Process](#code-review-process)
9. [Development Priorities](#development-priorities)

---

## Code of Conduct

### Our Pledge

This project welcomes contributors from all backgrounds. We pledge to make participation in our project a harassment-free experience for everyone.

### Expected Behavior

- Be respectful and inclusive
- Accept constructive criticism gracefully
- Focus on what is best for the community
- Show empathy towards other contributors

### Unacceptable Behavior

- Harassment, trolling, or derogatory comments
- Publishing others' private information
- Inappropriate use of project resources
- Any conduct which could reasonably be considered inappropriate

---

## Getting Started

### Prerequisites

Before contributing, ensure you have:

1. Read the [README.md](README.md)
2. Installed the project following [INSTALL.md](INSTALL.md)
3. Reviewed [ARCHITECTURE.md](ARCHITECTURE.md) to understand the codebase
4. Checked [CODE_REVIEW.md](CODE_REVIEW.md) for known issues

### First-Time Contributors

Good first issues for new contributors:

- Documentation improvements
- Fixing typos
- Adding code comments
- Writing unit tests
- Improving error messages
- Updating build configuration

Look for issues tagged with `good-first-issue` in the issue tracker.

---

## Development Setup

### Fork and Clone

```bash
# Fork the repository on GitHub, then:
git clone https://github.com/YOUR_USERNAME/birdclassifier.git
cd birdclassifier

# Add upstream remote
git remote add upstream https://github.com/ORIGINAL_OWNER/birdclassifier.git
```

### Create a Branch

```bash
# Update your fork
git checkout main
git pull upstream main

# Create a feature branch
git checkout -b feature/your-feature-name

# Or for bug fixes:
git checkout -b fix/issue-number-description
```

### Build for Development

```bash
# Build debug version
qmake CONFIG+=debug BirdSpeciesClassifier.pro
make

# Run tests (once implemented)
# make test
```

---

## Coding Standards

### C++ Style Guide

#### General Principles

1. **Clarity over cleverness** - Code should be readable
2. **Consistent style** - Follow existing patterns
3. **Modern C++** - Use C++11/14/17 features where appropriate
4. **Safety first** - Avoid undefined behavior

#### Naming Conventions

```cpp
// Classes: PascalCase with C prefix (existing convention)
class CSignal { };
class CSample { };

// Functions and methods: camelCase
void processSignal();
double computePower();

// Variables: camelCase
int sampleRate;
double *frames;

// Constants: UPPER_CASE
const uint FFT_SIZE = 256;
const double PI = 3.14159;

// Private members: no prefix (or _suffix if preferred)
private:
    int framesCount;
    string name;
```

#### Code Formatting

Use consistent indentation and spacing:

```cpp
// Indentation: Tabs or 4 spaces (be consistent with file)
class Example {
public:
    Example();
    ~Example();

    void method(int param);

private:
    int value;
};

// Braces: K&R style (existing convention)
if (condition) {
    doSomething();
} else {
    doSomethingElse();
}

// Spaces around operators
int result = a + b * c;
if (value > 0 && value < 100) {
    // ...
}
```

#### Header Files

```cpp
// Use include guards (existing convention)
#ifndef _HXX_FILENAME
#define _HXX_FILENAME

// Avoid 'using namespace' in headers
// Use std:: prefix explicitly

// Forward declarations when possible
class CSample;

// Include order:
// 1. System headers
// 2. Library headers
// 3. Project headers

#include <string>
#include <vector>
#include <fftw3.h>
#include "detect/Audio.hxx"

#endif
```

#### Modern C++ Practices

```cpp
// Use smart pointers instead of raw pointers
std::unique_ptr<double[]> frames;
std::shared_ptr<CSample> sample;

// Use nullptr instead of NULL
if (ptr != nullptr) { }

// Use auto for complex types
auto it = container.begin();

// Range-based for loops
for (const auto& sample : samples) {
    processSample(sample);
}

// Use const where possible
const string& getName() const { return name; }

// Use override keyword
virtual void process() override;

// Use RAII for resource management
class AudioFile {
    SNDFILE* file;
public:
    AudioFile(const string& path) {
        file = sf_open(path.c_str(), SFM_READ, &info);
    }
    ~AudioFile() {
        if (file) sf_close(file);
    }
};
```

#### Error Handling

```cpp
// Create custom exception classes
class AudioException : public std::runtime_error {
public:
    AudioException(const std::string& msg)
        : std::runtime_error(msg) {}
};

// Throw meaningful exceptions
if (file == nullptr) {
    throw AudioException("Failed to open file: " + filename);
}

// Document exceptions in comments
/**
 * Load audio file
 * @throws AudioException if file cannot be opened
 */
void loadAudio(const string& filename);
```

#### Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief Compute FFT on audio signal
 *
 * @param in Input signal buffer
 * @param out Output frequency data
 * @param n Number of samples
 * @return Number of frequency bins computed
 * @throws std::invalid_argument if n > FFT_SIZE
 */
int computeFFT(double* in, double* out, int n);
```

---

## Submitting Changes

### Commit Messages

Write clear, descriptive commit messages:

```
Short summary (50 chars or less)

More detailed explanation if necessary. Wrap at 72 characters.
Explain the problem this commit solves and why you chose this
approach.

- Bullet points are okay
- Use present tense: "Add feature" not "Added feature"
- Reference issues: Fixes #123, Relates to #456

Co-authored-by: Name <email@example.com>
```

**Examples**:

```
Fix buffer overflow in HammingWindow function

Replace static array with dynamic allocation to handle
arbitrary window sizes. Adds bounds checking.

Fixes #42
```

```
Add unit tests for FFT computation

Implements tests for:
- Basic FFT accuracy
- Edge cases (zero signal, single sample)
- Performance benchmarks

Relates to #15
```

### Pull Request Process

1. **Update your branch**:
```bash
git fetch upstream
git rebase upstream/main
```

2. **Run tests** (once implemented):
```bash
make test
```

3. **Push to your fork**:
```bash
git push origin feature/your-feature-name
```

4. **Create Pull Request**:
   - Use a clear title
   - Describe what changes you made and why
   - Reference related issues
   - Include screenshots for UI changes
   - List any breaking changes

5. **PR Template**:
```markdown
## Description
Brief description of changes

## Motivation
Why is this change needed?

## Changes Made
- Change 1
- Change 2

## Testing
How did you test this?

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No new warnings introduced
- [ ] Tests added/updated (if applicable)

## Related Issues
Fixes #123
```

---

## Reporting Bugs

### Before Submitting

1. Check if the bug has already been reported
2. Try to reproduce with latest version
3. Gather relevant information

### Bug Report Template

```markdown
## Bug Description
Clear description of the bug

## Steps to Reproduce
1. Step one
2. Step two
3. ...

## Expected Behavior
What should happen

## Actual Behavior
What actually happens

## Environment
- OS: [e.g., Ubuntu 20.04]
- Compiler: [e.g., GCC 9.3.0]
- Qt Version: [e.g., Qt 4.8.7]
- BSC Version: [e.g., 0.83]

## Additional Context
- Error messages
- Stack traces
- Sample audio files (if relevant)
- Screenshots

## Possible Solution
(Optional) Suggest a fix
```

---

## Suggesting Features

### Feature Request Template

```markdown
## Feature Description
Clear description of the proposed feature

## Motivation
Why is this feature needed?
What problem does it solve?

## Proposed Solution
How should this feature work?

## Alternatives Considered
What other approaches did you consider?

## Additional Context
- Mockups
- Example use cases
- Similar features in other software

## Implementation Notes
(Optional) Technical suggestions
```

---

## Code Review Process

### What Reviewers Look For

1. **Correctness**: Does the code work as intended?
2. **Quality**: Is it readable and maintainable?
3. **Testing**: Are there appropriate tests?
4. **Performance**: Are there performance implications?
5. **Security**: Are there security concerns?
6. **Documentation**: Is it well documented?

### Responding to Reviews

- Be receptive to feedback
- Ask for clarification if needed
- Make requested changes or explain why not
- Mark resolved comments
- Re-request review after updates

### Approval Process

- At least one maintainer approval required
- All CI checks must pass (when implemented)
- No unresolved comments
- Up-to-date with main branch

---

## Development Priorities

Based on [CODE_REVIEW.md](CODE_REVIEW.md), current priorities:

### Critical (High Priority)

1. **Memory Safety**
   - Replace raw pointers with smart pointers
   - Fix buffer overflow risks
   - Improve RAII patterns

2. **Error Handling**
   - Create exception hierarchy
   - Add proper cleanup paths
   - Improve error messages

3. **Code Safety**
   - Fix unsafe string operations
   - Remove global mutable state
   - Add bounds checking

### Important (Medium Priority)

4. **Code Modernization**
   - Remove `using namespace std` from headers
   - Add const correctness
   - Fix include issues

5. **Build System**
   - Remove hardcoded paths
   - Add CMake support
   - Improve cross-platform builds

6. **Documentation**
   - Add Doxygen comments
   - API documentation
   - Usage examples

### Nice to Have (Low Priority)

7. **Testing**
   - Unit test framework
   - Integration tests
   - Performance benchmarks

8. **Code Quality**
   - Consistent naming
   - Remove code duplication
   - Improve code organization

---

## Testing Guidelines

### Unit Tests

```cpp
// Example using Google Test (future addition)
TEST(AudioTest, ComputePowerBasic) {
    double frames[] = {1.0, 2.0, 3.0};
    double power = computePower(frames, 3);
    EXPECT_NEAR(power, 4.666, 0.001);
}
```

### Test Coverage Goals

- Core algorithms: 80%+ coverage
- File I/O: 70%+ coverage
- GUI code: 40%+ coverage (harder to test)

---

## Release Process

(For maintainers)

1. Update version in `.pro` file
2. Update CHANGELOG
3. Tag release: `git tag -a v0.84 -m "Release 0.84"`
4. Build binaries for all platforms
5. Create GitHub release
6. Update documentation

---

## Communication

### Where to Ask Questions

- **GitHub Issues**: Bug reports, feature requests
- **GitHub Discussions**: General questions, ideas
- **Email**: rkj@go2.pl (original author)

### Response Times

- Issues: Within 7 days
- Pull requests: Within 14 days
- Security issues: Within 24 hours

---

## License

By contributing, you agree that your contributions will be licensed under the GNU General Public License v2.0.

All new files should include the license header:

```cpp
/*
    Bird Species Classifier (BSC)
    Copyright (C) 2006 Roman Kamyk
    Copyright (C) YEAR YOUR_NAME

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
```

---

## Recognition

Contributors will be:
- Listed in CONTRIBUTORS file (to be created)
- Mentioned in release notes
- Credited in git history

---

## Questions?

If you have questions about contributing, please:
1. Check this document first
2. Search existing issues
3. Create a new issue with the `question` label

---

**Thank you for contributing to Bird Species Classifier!**

---

**Last Updated**: 2026-01-18
