# Installation Guide

Complete installation instructions for Bird Species Classifier (BSC) on Linux, macOS, and Windows.

---

## Table of Contents

1. [Quick Install](#quick-install)
2. [Linux Installation](#linux-installation)
3. [macOS Installation](#macos-installation)
4. [Windows Installation](#windows-installation)
5. [Build Configuration](#build-configuration)
6. [CMake Build (Recommended)](#cmake-build-recommended)
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

## Vcpkg (All Platforms)

If you want a single dependency manager across platforms, use the `vcpkg.json` manifest in this repo.

```bash
# From your vcpkg clone
./vcpkg/bootstrap-vcpkg.sh

# From this repo (uses vcpkg.json)
vcpkg install

# Configure with vcpkg toolchain
cmake --preset vcpkg
# Or for GUI build
cmake --preset vcpkg-gui
```

Set `VCPKG_ROOT` to your vcpkg clone so presets can find the toolchain:

```bash
export VCPKG_ROOT=/path/to/vcpkg
```

---

## Linux Installation

### Ubuntu / Debian

#### Prerequisites

```bash
# Update package list
sudo apt-get update

# Install build tools
sudo apt-get install -y build-essential git

# Install required development libraries
sudo apt-get install -y \
    qt6-base-dev \
    qt6-charts-dev \
    libsndfile1-dev \
    libfftw3-dev \
    librtaudio-dev \
    libasound2-dev \
    libpthread-stubs0-dev

# Install CMake
# Note: CMake 3.21+ is required for 'cmake --preset'.
# If your distribution provides an older version, install via snap or pip,
# or use the manual build instructions below.
sudo apt-get install -y cmake
```

#### Build and Install

**Option 1: Using CMake Presets (Recommended, requires CMake 3.21+)**

```bash
# Clone the repository
git clone https://github.com/yourusername/birdclassifier.git
cd birdclassifier

# Configure and build
cmake --preset gui
cmake --build --preset gui
```

**Option 2: Manual CMake Configuration (For CMake < 3.21)**

```bash
mkdir build
cd build
cmake -DBUILD_GUI=ON ..
make -j$(nproc)
```

#### Run

```bash
# Run from build directory
./build/gui/bin/BSC

# Or for CLI mode
./build/gui/bin/BSC -h
```

---

### Fedora / RHEL / CentOS

#### Prerequisites

```bash
# Install build tools
sudo dnf groupinstall "Development Tools"

# Install required development libraries
sudo dnf install -y \
    qt6-qtbase-devel \
    qt6-qtcharts-devel \
    libsndfile-devel \
    fftw-devel \
    rtaudio-devel \
    alsa-lib-devel \
    cmake
```

#### Build

```bash
cd birdclassifier
# If CMake >= 3.21:
cmake --preset gui
cmake --build --preset gui

# If CMake < 3.21:
# mkdir build && cd build && cmake -DBUILD_GUI=ON .. && make -j$(nproc)
```

---

### Arch Linux

#### Prerequisites

```bash
# Install required packages
sudo pacman -Syu
sudo pacman -S --needed base-devel git qt6-base qt6-charts libsndfile fftw alsa-lib cmake rtaudio
```

#### Build

```bash
cd birdclassifier
cmake --preset gui
cmake --build --preset gui
```

---

### Other Linux Distributions

**General requirements**:
- GCC/G++ compiler with C++17 support (GCC 7+)
- CMake 3.21+ (for presets) or 3.16+ (manual build)
- Qt6 development files (qt6-base)
- Qt6 Charts module
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

3. **Install Dependencies**:
```bash
# Install Qt6 and other dependencies
brew install cmake qt6 libsndfile fftw rtaudio

# Link Qt6 (if necessary, or set CMAKE_PREFIX_PATH)
# Usually CMake finds it automatically if installed via brew
```

#### Build

```bash
cd birdclassifier

# Option 1: Presets (if CMake >= 3.21)
cmake --preset gui
cmake --build --preset gui

# Option 2: Manual
mkdir build && cd build
cmake -DBUILD_GUI=ON -DCMAKE_PREFIX_PATH=$(brew --prefix qt6) ..
make -j$(sysctl -n hw.ncpu)
```

#### Run

```bash
./build/gui/bin/BSC.app/Contents/MacOS/BSC
```

---

## Windows Installation

### Prerequisites

1. **Install Qt6**:
   - Download the Qt Online Installer from [qt.io](https://www.qt.io/download).
   - Install **Qt 6.x** for your compiler (e.g., MinGW 11.2.0 64-bit or MSVC 2019/2022).

2. **Install CMake**:
   - Install CMake 3.21 or later from [cmake.org](https://cmake.org/download/).

3. **Install Dependencies**:
   - **libsndfile**, **fftw3**, **rtaudio**, and **Qt6** are required.
   - Recommended: Use [vcpkg](https://github.com/microsoft/vcpkg) to manage dependencies.
     ```powershell
     .\vcpkg install libsndfile fftw3 rtaudio qtbase qtcharts --triplet x64-windows
     ```

### Build using Command Line

1. Open your compiler environment (e.g., "Qt 6.x (MinGW) Terminal" or "x64 Native Tools Command Prompt for VS 2022").

2. Navigate to source:
   ```cmd
   cd birdclassifier
   ```

3. Build:
   ```cmd
   cmake --preset gui
   cmake --build --preset gui
   ```

   *Note: You may need to set `CMAKE_PREFIX_PATH` to your Qt installation if not found.*

---

## Build Configuration

The project uses `CMakePresets.json` to define standard build configurations:

- **default**: No GUI, only library and tests (Debug).
- **gui**: Build with Qt6 GUI (Debug).
- **release**: Build with Qt6 GUI (Release, Optimized).

To list available presets:
```bash
cmake --list-presets
```

## Troubleshooting

### CMake Error: Unrecognized "version" field
This error occurs if your CMake version is older than 3.21. `CMakePresets.json` uses version 3 schema which requires CMake 3.21+.

**Solution:**
- Upgrade CMake to 3.21 or newer.
- OR use the manual build method:
  ```bash
  mkdir build
  cd build
  cmake -DBUILD_GUI=ON ..
  make
  ```
