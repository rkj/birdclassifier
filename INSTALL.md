# Installation Guide

Complete installation instructions for Bird Species Classifier (BSC) on Linux, macOS, and Windows.

---

## Table of Contents

1. [Quick Install](#quick-install)
2. [Bazel Build (Required)](#bazel-build-required)
3. [Linux Installation](#linux-installation)
4. [macOS Installation](#macos-installation)
5. [Windows Installation](#windows-installation)
6. [Build Configuration](#build-configuration)
7. [Running Tests](#running-tests)
8. [Troubleshooting](#troubleshooting)
9. [Verification](#verification)

---

## Quick Install

Choose your platform:

| Platform | Jump to Instructions |
|----------|---------------------|
| Ubuntu/Debian | [Linux (apt)](#ubuntu--debian) |
| Fedora/RHEL | [Linux (dnf/yum)](#fedora--rhel--centos) |
| Arch Linux | [Linux (pacman)](#arch-linux) |
| macOS | [macOS (Homebrew)](#macos-installation) |
| Windows | [Windows](#windows-installation) |

---

## Bazel Build (Required)

Bazel is the single supported build system. The CLI and tests are built with Bazel.
The Qt GUI is not available in the Bazel-only workflow yet.

### Install Bazel

Install Bazel using your system package manager (example for Ubuntu/Debian):

```bash
sudo apt-get update
sudo apt-get install -y bazel
```

### Build CLI and Tests

```bash
bazel build //:bsc_cli
bazel test //tests:audio_tests
```

---

## Linux Installation

### Ubuntu / Debian

#### Prerequisites

```bash
# Update package list
sudo apt-get update

# Install build tools
sudo apt-get install -y build-essential git bazel

# Install required development libraries
sudo apt-get install -y \
    libsndfile1-dev \
    libfftw3-dev \
    librtaudio-dev \
    libasound2-dev \
    libpthread-stubs0-dev
```

#### Build and Run

```bash
# Clone the repository
git clone https://github.com/yourusername/birdclassifier.git
cd birdclassifier

# Build CLI and tests
bazel build //:bsc_cli
bazel test //tests:audio_tests

# Run CLI
./bazel-bin/bsc_cli -h
```

---

### Fedora / RHEL / CentOS

#### Prerequisites

```bash
# Install build tools
sudo dnf groupinstall "Development Tools"

# Install required development libraries
sudo dnf install -y \
    bazel \
    libsndfile-devel \
    fftw-devel \
    rtaudio-devel \
    alsa-lib-devel
```

#### Build and Run

```bash
cd birdclassifier
bazel build //:bsc_cli
bazel test //tests:audio_tests
./bazel-bin/bsc_cli -h
```

---

### Arch Linux

#### Prerequisites

```bash
# Install required packages
sudo pacman -Syu
sudo pacman -S --needed base-devel git bazel libsndfile fftw alsa-lib rtaudio
```

#### Build and Run

```bash
cd birdclassifier
bazel build //:bsc_cli
bazel test //tests:audio_tests
./bazel-bin/bsc_cli -h
```

---

### Other Linux Distributions

**General requirements**:
- GCC/G++ compiler with C++17 support (GCC 7+)
- Bazel
- RtAudio development files (>= 6.0)
- libsndfile development files
- FFTW3 development files
- ALSA development files
- pthread support

---

## macOS Installation

### Using Homebrew (Recommended)

#### Prerequisites

1. **Install Xcode Command Line Tools**:
```bash
xcode-select --install
```

2. **Install Homebrew** (if not already installed):
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

3. **Install dependencies**:
```bash
brew install bazel libsndfile fftw rtaudio
```

#### Build and Run

```bash
cd birdclassifier
bazel build //:bsc_cli
bazel test //tests:audio_tests
./bazel-bin/bsc_cli -h
```

---

## Windows Installation

### Prerequisites

1. **Install Bazel**:
   - Install Bazel from https://bazel.build/install
2. **Install Dependencies**:
   - Install libsndfile, FFTW3, and RtAudio development libraries for your toolchain.

### Build

```cmd
cd birdclassifier
bazel build //:bsc_cli
bazel test //tests:audio_tests
bazel-bin\bsc_cli.exe -h
```

---

## Build Configuration

Bazel targets:
- `//:bsc_cli` - CLI binary
- `//tests:audio_tests` - unit tests

---

## Running Tests

```bash
bazel test //tests:audio_tests
```

---

## Troubleshooting

### Missing system libraries

If Bazel fails to link, ensure the development packages for libsndfile, FFTW3,
RtAudio, and ALSA are installed for your platform.

---

## Verification

```bash
./bazel-bin/bsc_cli ~/test.mp3
```
