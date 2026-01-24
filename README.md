# Bird Species Classifier (BSC)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-0.83-green.svg)](https://github.com/yourusername/birdclassifier)

> **Acoustic bird species recognition using pattern matching and nearest neighbor classification**

Bird Species Classifier is a cross-platform C++/Qt application that identifies bird species based on their vocalizations using digital signal processing and machine learning techniques.

![BSC Screenshot Placeholder](img/screenshot.png)
*Note: Add actual screenshot when available*

---

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [About the Project](#about-the-project)
- [System Requirements](#system-requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)
- [Author](#author)
- [Acknowledgments](#acknowledgments)

---

## Features

### GUI Mode
- 🎵 **Audio File Support**: Load and analyze WAV and MP3 files
- 📊 **Multi-Panel Visualization**:
  - Time-domain audio signal display
  - Digitally filtered signal (2-14 kHz bandpass)
  - Frequency spectrogram with FFT analysis
- 🎙️ **Real-time Recording**: Capture and analyze live audio input
- 🔍 **Interactive Analysis**: Click to select and analyze signal fragments
- 🎨 **Multiple Color Schemes**: Black & White, Color Black, Color White
- ⚡ **Energy Detection**: Visual indicators for high-energy signal regions
- 🔊 **Audio Playback**: Listen to recordings with visual markers

### CLI/Batch Mode
- 📁 **Batch Processing**: Process multiple audio files automatically
- 🎯 **Configurable Parameters**:
  - Signal-to-Noise Ratio (SNR) adjustment
  - Cutoff thresholds for classification
  - Power threshold configuration
  - Hop time for signal segmentation
- 🧪 **Cross-Validation**: 10-fold cross-validation testing
- 💾 **Sample Export/Import**: Manage learning sets
- 📝 **Verbose Output**: Detailed analysis reporting

### Signal Processing
- **FFT Analysis**: Fast Fourier Transform using FFTW3 library
- **Window Functions**: Hamming, Blackman, Hanning, Rectangle
- **Bandpass Filtering**: 2-14 kHz range optimization for bird vocalizations
- **Power-based Segmentation**: Intelligent signal fragment extraction
- **Normalization**: Frequency feature normalization for comparison

### Classification
- **Nearest Neighbor**: Pattern matching with configurable thresholds
- **Learning Set Management**: Build and maintain species reference databases
- **Multiple Species Support**: Extensible bird species database
- **Similarity Metrics**: Custom difference calculation for audio patterns

---

## Quick Start

### Installation (Linux)

```bash
# Install dependencies
sudo apt-get install qt4-dev-tools libsndfile1-dev libfftw3-dev libasound2-dev

# Build
qmake
make

# Run GUI mode
./bin/BSC

# Run batch mode
./bin/BSC -learning learning_data/ audio_file.wav
```

See [INSTALL.md](INSTALL.md) for detailed platform-specific instructions.

### Basic Usage

**GUI Mode** (no parameters):
```bash
./bin/BSC
```

**CLI Mode** (with parameters):
```bash
# Analyze a single file
./bin/BSC -learning data/samples/ bird_recording.wav

# Batch process with custom parameters
./bin/BSC -learning data/samples/ -snr 3.5 -cutoff 0.25 -verbose *.wav

# Cross-validation testing
./bin/BSC -learning data/samples/ -crosstest
```

Run `./bin/BSC --help` for full parameter documentation.

---

## About the Project

Bird Species Classifier (BSC) was developed in 2006 by Roman 'MrStone' Kamyk as part of the **BirdWatch project** for a CSIDC (Computing Science Institute Development Competition) at Poznan University of Technology, Poland.

The application combines digital signal processing, pattern recognition, and interactive visualization to enable automated bird species identification from audio recordings.

**Academic Background**:
- **Project**: BirdWatch - Automated Bird Species Recognition
- **Institution**: Poznan University of Technology, Computing Science Institute
- **Year**: 2006
- **Competition**: CSIDC

**External Resources**:
- [BirdWatch Final Report (PDF)](https://www.dropbox.com/scl/fi/0i752ctsvard7md0nwwr5/Poznan-University-of-Technology-BirdWatch.pdf?rlkey=h0c8oc3167u41vg1rp2p6a4zi&st=5xe6fkn3&dl=0)

---

## System Requirements

### Minimum Requirements
- **OS**: Linux, macOS 10.4+, Windows XP/Vista/7+
- **CPU**: 1 GHz processor
- **RAM**: 512 MB
- **Display**: 1024x768 resolution

### Recommended Requirements
- **OS**: Modern Linux distribution, macOS 10.9+, Windows 7+
- **CPU**: 2 GHz dual-core processor
- **RAM**: 2 GB
- **Display**: 1280x1024 resolution or higher
- **Audio**: Sound card with microphone input (for real-time recording)

### Software Dependencies

#### Required Libraries
- **Qt4** - GUI framework
- **RtAudio (>= 6.0)** - Real-time audio I/O
- **libsndfile** - WAV file I/O
- **FFTW3** - Fast Fourier Transform computations
- **pthread** - Multi-threading support

#### Platform-Specific
- **Linux**: ALSA (`libasound2`)
- **macOS**: CoreAudio (included in OS)
- **Windows**: DirectSound (included in OS)

#### Build Tools
- **gcc/g++** - C++ compiler with C++98 support minimum
- **Bazel/Bazelisk** - Preferred build and test runner
- **qmake** - Qt build tool
- **make** - Build automation

---

### Bazel Build (Preferred)

```bash
./tools/bazelisk build //:bsc_cli
./tools/bazelisk test //tests:audio_tests
```

## Installation

For detailed installation instructions for your platform, see [INSTALL.md](INSTALL.md).

### Quick Platform Guide

<details>
<summary><b>Linux (Ubuntu/Debian)</b></summary>

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential qt4-dev-tools qt4-qmake \
                     libsndfile1-dev libfftw3-dev libasound2-dev librtaudio-dev

# Clone repository
git clone https://github.com/yourusername/birdclassifier.git
cd birdclassifier

# Build
qmake BirdSpeciesClassifier.pro
make

# Run
./bin/BSC
```
</details>

<details>
<summary><b>macOS</b></summary>

```bash
# Install dependencies (using Homebrew)
brew install qt@4 libsndfile fftw

# Build
qmake BirdSpeciesClassifier.pro
make

# Run
./bin/BSC
```
</details>

<details>
<summary><b>Windows (MinGW)</b></summary>

1. Install MinGW and Qt4
2. Install libsndfile and FFTW3 development libraries
3. Update paths in `BirdSpeciesClassifier.pro` (lines 26-28)
4. Run:
```cmd
qmake
mingw32-make
bin\BSC.exe
```
</details>

---

## Usage

### GUI Mode

Launch without parameters:
```bash
./bin/BSC
```

**Interface Overview**:
1. **Audio Signal Panel** (top): Time-domain waveform display
2. **Filtered Signal Panel** (middle): Bandpass-filtered signal (2-14 kHz)
3. **Spectrogram Panel** (bottom): Frequency-domain FFT visualization

**Mouse Controls**:
- **Left Mouse Button (LMB)**: Select fragments in audio signal panel
- **Middle Mouse Button (MMB)**: Analyze selected fragment
- **Right Mouse Button (RMB)**: Zoom in/out

**Visual Indicators**:
- **Green fragments**: High-energy regions above threshold
- **Red markers**: Playback position

**Workflow**:
1. Load audio file (File → Open or drag & drop)
2. Adjust signal energy threshold if needed
3. Click on green fragments to analyze
4. View FFT results in spectrogram panel
5. Compare with learning set for species identification

### CLI/Batch Mode

Process files with parameters:
```bash
./bin/BSC [OPTIONS] audio_files...
```

**Common Options**:
- `-h` - Display help
- `-learning <dir>` - Load learning set from directory
- `-learnFile <file>` - Load learning set from file
- `-verbose` - Enable verbose output
- `-snr <value>` - Set Signal-to-Noise Ratio (default: 3.0)
- `-cutoff <value>` - Set difference cutoff threshold (default: 0.255)
- `-powerCutoff <value>` - Set signal power threshold (default: 1e-04)
- `-crosstest` - Perform 10-fold cross-validation

**Examples**:

```bash
# Basic recognition
./bin/BSC -learning samples/ recording.wav

# Multiple files with custom SNR
./bin/BSC -learning samples/ -snr 4.0 -verbose bird1.wav bird2.wav

# Cross-validation testing
./bin/BSC -learning samples/ -crosstest

# Save analyzed samples
./bin/BSC -learning samples/ -save analyzed_ recording.wav

# No filtering (use raw signal)
./bin/BSC -learning samples/ -nofilter recording.wav
```

For complete parameter documentation, run `./bin/BSC --help`.

---

## Project Structure

```
birdclassifier/
├── README.md                    # This file
├── INSTALL.md                   # Installation guide
├── CODE_REVIEW.md              # Code review and analysis
├── ARCHITECTURE.md             # System architecture documentation
├── CONTRIBUTING.md             # Contribution guidelines
├── LICENSE                      # MIT license
├── BirdSpeciesClassifier.pro   # Qt project configuration
├── Images.qrc                  # Qt resource file
├── .gitignore                  # Git ignore rules
│
├── bin/                        # Build output directory
│   ├── BSC                     # Release binary
│   └── BSC_Debug              # Debug binary
│
├── detect/                     # Core detection engine
│   ├── Audio.hxx/cpp          # Audio processing & FFT
│   ├── detect.hxx/cpp         # Recognition algorithms
│   ├── Files.hxx/cpp          # File I/O abstraction
│   ├── Filter.hxx/cpp         # Signal filtering
│   └── Manager.hxx/cpp        # Sample management
│
├── Drawers/                    # Visualization components
│   ├── AudioDraw.*            # Waveform display
│   ├── SignalDraw.*           # Filtered signal display
│   ├── SpectroDraw.*          # Spectrogram display
│   ├── Spectrogram.*          # Spectrogram rendering
│   └── EnergyDraw.*           # Energy profile display
│
├── mpglib/                     # MP3 decoding library
│   ├── interface.c            # Library interface
│   ├── layer3.c               # MP3 layer 3 decoder
│   └── ...                    # Supporting files
│
├── img/                        # GUI icons and images
│
└── *.ui                        # Qt Designer UI files
    ├── MainWindow.ui
    ├── SettingsDialog.ui
    ├── LearningDialog.ui
    └── ComparisonWindow.ui
```

---

## Documentation

- **[INSTALL.md](INSTALL.md)** - Detailed installation instructions for all platforms
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - System design and architecture documentation
- **[CODE_REVIEW.md](CODE_REVIEW.md)** - Code quality analysis and recommendations
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Guidelines for contributors
- **`--help`** - Run `./bin/BSC --help` for command-line parameter reference

---

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on:
- Code style and standards
- Submitting issues and bug reports
- Creating pull requests
- Development workflow

---

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

```
MIT License
Copyright (c) 2006 Roman Kamyk
```

---

## Author

**Roman 'MrStone' Kamyk**
- Email: rkj@go2.pl
- Affiliation: Poznan University of Technology, Computing Science Institute
- Role: Original Developer (2006)

---

## Acknowledgments

- **Poznan University of Technology** - Academic support and project sponsorship
- **BirdWatch Project Team** - Collaboration and research
- **FFTW3 Developers** - Efficient FFT library
- **RtAudio Project** - Cross-platform audio I/O
- **Qt Project** - GUI framework

---

## Citation

If you use this software in academic research, please cite:

```bibtex
@software{bsc2006,
  author = {Kamyk, Roman},
  title = {Bird Species Classifier},
  year = {2006},
  publisher = {Poznan University of Technology},
  url = {http://www.birdwatch.pl}
}
```

---

## Support

For issues, questions, or suggestions:
1. Check existing [Issues](https://github.com/yourusername/birdclassifier/issues)
2. Run `./bin/BSC --help` for usage documentation
3. Create a new issue with detailed information

---

## Changelog

### Version 0.83 (2006-07-10)
- Initial public release
- GUI and CLI modes
- WAV and MP3 support
- Cross-platform audio I/O
- FFT-based analysis
- Nearest neighbor classification

---

**Last Updated**: 2026-01-18
