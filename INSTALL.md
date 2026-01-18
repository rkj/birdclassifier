# Installation Guide

Complete installation instructions for Bird Species Classifier (BSC) on Linux, macOS, and Windows.

---

## Table of Contents

1. [Quick Install](#quick-install)
2. [Linux Installation](#linux-installation)
3. [macOS Installation](#macos-installation)
4. [Windows Installation](#windows-installation)
5. [Build Configuration](#build-configuration)
6. [Troubleshooting](#troubleshooting)
7. [Verification](#verification)

---

## Quick Install

Choose your platform:

| Platform | Jump to Instructions |
|----------|---------------------|
| Ubuntu/Debian | [Linux (apt)](#ubuntu--debian) |
| Fedora/RHEL | [Linux (dnf/yum)](#fedora--rhel--centos) |
| Arch Linux | [Linux (pacman)](#arch-linux) |
| macOS | [macOS (Homebrew)](#macos-installation) |
| Windows | [Windows (MinGW)](#windows-installation) |

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
    qt4-dev-tools \
    qt4-qmake \
    libqt4-dev \
    libsndfile1-dev \
    libfftw3-dev \
    libasound2-dev \
    libpthread-stubs0-dev
```

#### Build and Install

```bash
# Clone the repository (or use your existing directory)
git clone https://github.com/yourusername/birdclassifier.git
cd birdclassifier

# Generate Makefile
qmake BirdSpeciesClassifier.pro

# Compile (use -j for parallel compilation)
make -j$(nproc)

# Optional: Install to system (requires sudo)
# sudo make install  # Not configured in current .pro file
```

#### Run

```bash
# Run from build directory
./bin/BSC

# Or for CLI mode
./bin/BSC -h
```

---

### Fedora / RHEL / CentOS

#### Prerequisites

```bash
# Install build tools
sudo dnf groupinstall "Development Tools"

# For RHEL/CentOS 7, use yum:
# sudo yum groupinstall "Development Tools"

# Install required development libraries
sudo dnf install -y \
    qt-devel \
    qt-config \
    libsndfile-devel \
    fftw-devel \
    alsa-lib-devel

# For RHEL/CentOS 7:
# sudo yum install qt-devel libsndfile-devel fftw-devel alsa-lib-devel
```

#### Build

```bash
cd birdclassifier
qmake-qt4 BirdSpeciesClassifier.pro  # or just 'qmake'
make -j$(nproc)
```

---

### Arch Linux

#### Prerequisites

```bash
# Install required packages
sudo pacman -Syu
sudo pacman -S --needed base-devel git qt4 libsndfile fftw alsa-lib
```

#### Build

```bash
cd birdclassifier
qmake-qt4 BirdSpeciesClassifier.pro
make -j$(nproc)
```

---

### Other Linux Distributions

**General requirements**:
- GCC/G++ compiler (4.x or later)
- Qt4 development files
- libsndfile development files
- FFTW3 development files
- ALSA development files
- pthread support

Consult your distribution's package manager documentation for package names.

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
# Install Qt4 (may require tapping deprecated formulas)
brew tap cartr/qt4
brew install qt@4

# Install audio and math libraries
brew install libsndfile fftw

# Add Qt4 to PATH
export PATH="/usr/local/opt/qt@4/bin:$PATH"
```

#### Build

```bash
cd birdclassifier

# Use Qt4's qmake
/usr/local/opt/qt@4/bin/qmake BirdSpeciesClassifier.pro

# Compile
make -j$(sysctl -n hw.ncpu)
```

#### Run

```bash
./bin/BSC
```

### Using MacPorts

```bash
# Install dependencies
sudo port install qt4-mac libsndfile fftw-3

# Build
qmake-qt4 BirdSpeciesClassifier.pro
make -j$(sysctl -n hw.ncpu)
```

---

## Windows Installation

### Using MinGW

#### Prerequisites

1. **Install MinGW**:
   - Download MinGW installer from [mingw.org](http://www.mingw.org/)
   - Install to `C:\MinGW` (recommended)
   - Select packages: `mingw32-base`, `mingw32-gcc-g++`, `msys-base`

2. **Install Qt4**:
   - Download Qt4 for Windows from Qt archives
   - Install Qt4 SDK with MinGW support
   - Default: `C:\Qt\4.8.x`

3. **Install libsndfile**:
   - Download from [mega-nerd.com](http://www.mega-nerd.com/libsndfile/)
   - Extract to `C:\msys\1.0\local\`

4. **Install FFTW3**:
   - Download precompiled Windows binaries from [fftw.org](http://www.fftw.org/install/windows.html)
   - Extract to `C:\msys\1.0\local\`

#### Configure Build Paths

Edit `BirdSpeciesClassifier.pro` lines 26-28 to match your installation:

```qmake
win32 {
    LIBS += -ldsound
    LIBS += -LC:\msys\1.0\lib -LC:\msys\1.0\local\lib
    INCLUDEPATH += C:\msys\1.0\local\include
    INCLUDEPATH += C:\msys\1.0\include
    DEFINES += __WINDOWS_DS__
    OBJECTS_DIR = bin\
}
```

#### Build

Open MinGW Shell or Command Prompt:

```cmd
cd birdclassifier

REM Add Qt to PATH
set PATH=C:\Qt\4.8.x\bin;%PATH%

REM Generate Makefile
qmake BirdSpeciesClassifier.pro

REM Compile
mingw32-make

REM Run
bin\BSC.exe
```

### Using Visual Studio

BSC was primarily developed with MinGW. Visual Studio support is not tested. If you attempt VS compilation:

1. Install Qt4 Visual Studio edition
2. Use Visual Studio Qt add-in
3. Open .pro file and convert to .sln
4. Ensure library paths are correct

---

## Build Configuration

### Build Types

#### Debug Build

```bash
qmake CONFIG+=debug BirdSpeciesClassifier.pro
make
# Output: bin/BSC_Debug
```

**Features**:
- Debugging symbols included
- RtAudio debug output enabled
- Warnings treated as errors (`-Werror`)
- No optimization

#### Release Build (Default)

```bash
qmake CONFIG+=release BirdSpeciesClassifier.pro
make
# Output: bin/BSC
```

**Features**:
- Optimized binary
- Stripped symbols (`-s`)
- Smaller executable size

### Custom Configuration

#### Change Build Directory

Edit `.pro` file:
```qmake
DESTDIR = /path/to/custom/output
```

#### Disable Warnings as Errors

Comment out in `.pro` file:
```qmake
# QMAKE_CXXFLAGS_DEBUG += -Werror
```

#### Add Custom Compiler Flags

```qmake
QMAKE_CXXFLAGS += -O3 -march=native  # Example: aggressive optimization
```

---

## Troubleshooting

### Common Issues

#### Qt4 Not Found

**Error**: `qmake: command not found`

**Solution**:
```bash
# Linux: Install qt4-qmake
sudo apt-get install qt4-qmake

# macOS: Add Qt4 to PATH
export PATH="/usr/local/opt/qt@4/bin:$PATH"

# Windows: Add Qt bin directory to PATH
set PATH=C:\Qt\4.8.x\bin;%PATH%
```

---

#### libsndfile Not Found

**Error**: `cannot find -lsndfile`

**Solution**:
```bash
# Linux
sudo apt-get install libsndfile1-dev

# macOS
brew install libsndfile

# Windows: Ensure libsndfile.a is in library path specified in .pro file
```

---

#### FFTW3 Not Found

**Error**: `cannot find -lfftw3`

**Solution**:
```bash
# Linux
sudo apt-get install libfftw3-dev

# macOS
brew install fftw

# Windows: Check FFTW3 installation in C:\msys\1.0\local\lib
```

---

#### ALSA Not Found (Linux Only)

**Error**: `cannot find -lasound`

**Solution**:
```bash
sudo apt-get install libasound2-dev
```

---

#### Compilation Errors with `-Werror`

**Error**: Warnings treated as errors in debug mode

**Solution**:
```bash
# Option 1: Build release version
qmake CONFIG+=release BirdSpeciesClassifier.pro
make

# Option 2: Disable -Werror (edit .pro file and remove the flag)
```

---

#### Wrong Qt Version

**Error**: Qt5 detected instead of Qt4

**Solution**:
```bash
# Use qt4-specific qmake
qmake-qt4 BirdSpeciesClassifier.pro

# Or specify Qt4 path explicitly
/usr/lib/x86_64-linux-gnu/qt4/bin/qmake BirdSpeciesClassifier.pro
```

---

#### Windows Path Issues

**Error**: Cannot find libraries despite correct installation

**Solution**:
- Use forward slashes in .pro file paths: `/` instead of `\`
- Or use double backslashes: `C:\\msys\\1.0\\lib`
- Ensure no spaces in paths (use quotes if unavoidable)

---

### Compiler Warnings

#### Warning: `using namespace std` in header

This is a known code quality issue. See [CODE_REVIEW.md](CODE_REVIEW.md) for details.

**Workaround**: Ignore warning or build release mode (no `-Werror`)

---

### Platform-Specific Issues

#### macOS: CoreAudio Framework Not Found

**Solution**: Verify Xcode Command Line Tools are installed
```bash
xcode-select --install
```

#### Linux: RtAudio initialization fails

**Solution**: Ensure ALSA is properly configured
```bash
# Test ALSA
aplay -l

# Install pulseaudio-alsa bridge if using PulseAudio
sudo apt-get install pulseaudio-alsa
```

---

## Verification

### Test Installation

#### 1. Check Binary Exists

```bash
ls -lh bin/BSC
# Should show executable file
```

#### 2. Display Help

```bash
./bin/BSC -h
# Should display usage information
```

#### 3. Test GUI Launch

```bash
./bin/BSC
# Should open GUI window
```

#### 4. Test with Sample File

```bash
# If you have a WAV file:
./bin/BSC sample.wav
# Should process the file (may show "unknown" if no learning set)
```

### Verify Dependencies

#### Linux

```bash
ldd bin/BSC
# Should show all required libraries are found
```

#### macOS

```bash
otool -L bin/BSC
# Should show all required frameworks/libraries
```

#### Windows

Use Dependency Walker or:
```cmd
dumpbin /dependents bin\BSC.exe
```

---

## Advanced Installation

### System-Wide Installation (Linux)

Create installation rules in `.pro` file:

```qmake
unix {
    target.path = /usr/local/bin
    INSTALLS += target
}
```

Then:
```bash
sudo make install
```

### Creating Desktop Launcher (Linux)

Create `bsc.desktop`:

```ini
[Desktop Entry]
Version=1.0
Type=Application
Name=Bird Species Classifier
Comment=Acoustic bird identification
Exec=/path/to/bin/BSC
Icon=/path/to/img/icon.png
Terminal=false
Categories=Audio;Science;Education;
```

Install:
```bash
cp bsc.desktop ~/.local/share/applications/
update-desktop-database ~/.local/share/applications/
```

---

## Uninstallation

### Remove Built Binaries

```bash
make clean
rm -rf bin/
```

### Remove Dependencies (Use with Caution)

Only remove if not needed by other applications:

```bash
# Ubuntu/Debian
sudo apt-get remove qt4-dev-tools libsndfile1-dev libfftw3-dev

# macOS
brew uninstall qt@4 libsndfile fftw
```

---

## Next Steps

After successful installation:

1. Read [USAGE](USAGE) for usage instructions
2. Review [README.md](README.md) for features overview
3. See [ARCHITECTURE.md](ARCHITECTURE.md) for system design
4. Check [CONTRIBUTING.md](CONTRIBUTING.md) if you want to contribute

---

## Getting Help

If you encounter issues not covered here:

1. Check [Troubleshooting](#troubleshooting) section above
2. Review [CODE_REVIEW.md](CODE_REVIEW.md) for known issues
3. Search existing [Issues](https://github.com/yourusername/birdclassifier/issues)
4. Create a new issue with:
   - Your OS and version
   - Compiler version (`gcc --version`)
   - Qt version (`qmake --version`)
   - Complete error message
   - Steps to reproduce

---

**Last Updated**: 2026-01-18
