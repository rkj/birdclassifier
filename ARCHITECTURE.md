# Architecture Documentation

Technical architecture and design documentation for Bird Species Classifier (BSC).

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Architecture Diagram](#architecture-diagram)
3. [Component Design](#component-design)
4. [Data Flow](#data-flow)
5. [Signal Processing Pipeline](#signal-processing-pipeline)
6. [Classification Algorithm](#classification-algorithm)
7. [Module Documentation](#module-documentation)
8. [Design Patterns](#design-patterns)
9. [Dependencies](#dependencies)
10. [Future Architecture](#future-architecture)

---

## System Overview

Bird Species Classifier is a hybrid GUI/CLI application built with a layered architecture:

```
┌─────────────────────────────────────────┐
│         User Interface Layer            │
│  (Qt GUI / Command Line Interface)      │
└────────────────┬────────────────────────┘
                 │
┌────────────────┴────────────────────────┐
│       Application Logic Layer           │
│  (Detection, Classification, Manager)   │
└────────────────┬────────────────────────┘
                 │
┌────────────────┴────────────────────────┐
│      Signal Processing Layer            │
│  (FFT, Filtering, Feature Extraction)   │
└────────────────┬────────────────────────┘
                 │
┌────────────────┴────────────────────────┐
│         I/O and Platform Layer          │
│  (Audio I/O, File I/O, Cross-platform)  │
└─────────────────────────────────────────┘
```

### Core Technologies

- **Language**: C++ (C++98/03 with some modern patterns)
- **GUI Framework**: Qt4
- **Audio I/O**: RtAudio (cross-platform)
- **Signal Processing**: FFTW3 (Fast Fourier Transform)
- **File I/O**: libsndfile (WAV), mpglib (MP3)

---

## Architecture Diagram

### High-Level Component Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                         BSC Application                       │
│                                                               │
│  ┌─────────────┐                    ┌──────────────────┐    │
│  │   Main      │ ◄──────creates──── │  SettingsDialog  │    │
│  │   Window    │                    └──────────────────┘    │
│  │   (GUI)     │                    ┌──────────────────┐    │
│  │             │ ◄──────creates──── │ LearningDialog   │    │
│  └─────┬───────┘                    └──────────────────┘    │
│        │uses                        ┌──────────────────┐    │
│        │                            │ ComparisonWindow │    │
│        │                            └──────────────────┘    │
│        ▼                                                      │
│  ┌─────────────────────────────────────────────────┐        │
│  │              Detection Engine                   │        │
│  │                                                  │        │
│  │  ┌──────────┐  ┌─────────┐  ┌──────────────┐  │        │
│  │  │ Manager  │─►│ CSample │◄─│ Audio/Files  │  │        │
│  │  └──────────┘  └─────────┘  └──────────────┘  │        │
│  │       │            │              │             │        │
│  │       ▼            ▼              ▼             │        │
│  │  ┌────────────────────────────────────────┐   │        │
│  │  │        Classification Logic            │   │        │
│  │  │  (Nearest Neighbor, Pattern Matching)  │   │        │
│  │  └────────────────────────────────────────┘   │        │
│  └─────────────────────────────────────────────────┘        │
│        │                                                      │
│        ▼                                                      │
│  ┌─────────────────────────────────────────────────┐        │
│  │          Signal Processing                      │        │
│  │                                                  │        │
│  │  ┌──────┐  ┌────────┐  ┌────────┐  ┌───────┐  │        │
│  │  │ CFFT │  │ Filter │  │ Window │  │ Power │  │        │
│  │  └──────┘  └────────┘  └────────┘  └───────┘  │        │
│  └─────────────────────────────────────────────────┘        │
│        │                                                      │
│        ▼                                                      │
│  ┌─────────────────────────────────────────────────┐        │
│  │           Visualization Drawers                 │        │
│  │                                                  │        │
│  │  ┌──────────┐  ┌──────────┐  ┌─────────────┐  │        │
│  │  │ AudioDraw│  │SignalDraw│  │ Spectrogram │  │        │
│  │  └──────────┘  └──────────┘  └─────────────┘  │        │
│  └─────────────────────────────────────────────────┘        │
│        │                                                      │
│        ▼                                                      │
│  ┌─────────────────────────────────────────────────┐        │
│  │               I/O Layer                         │        │
│  │                                                  │        │
│  │  ┌─────────┐  ┌────────┐  ┌─────────────────┐ │        │
│  │  │ RtAudio │  │ mpglib │  │ libsndfile      │ │        │
│  │  │(Real-time│  │ (MP3)  │  │ (WAV)           │ │        │
│  │  └─────────┘  └────────┘  └─────────────────┘ │        │
│  └─────────────────────────────────────────────────┘        │
└──────────────────────────────────────────────────────────────┘
```

---

## Component Design

### Core Components

#### 1. Detection Engine (`detect/`)

**Purpose**: Core audio processing and bird species recognition

**Components**:

| File | Responsibility |
|------|---------------|
| `Audio.cpp/hxx` | Audio signal processing, FFT computation |
| `detect.cpp/hxx` | Classification algorithms, testing |
| `Manager.cpp/hxx` | Batch processing, sample management |
| `Files.cpp/hxx` | File I/O abstraction (WAV/MP3) |
| `Filter.cpp/hxx` | Digital signal filtering |

**Key Classes**:

```cpp
CSignal      // Base class for audio signals
  └─ CSample // Individual audio sample with frequency data
  └─ CAudio  // Complete audio file

CFFT         // FFT processor (injected)
SFrequencies // Normalized frequency data
Manager      // Batch processing coordinator
```

#### 2. Visualization Layer (`Drawers/`)

**Purpose**: Qt-based visual representation of audio data

**Components**:

| Widget | Displays |
|--------|----------|
| `AudioDraw` | Time-domain waveform |
| `SignalDraw` | Filtered signal (2-14 kHz) |
| `SpectroDraw` | Spectrogram (frequency-time) |
| `Spectrogram` | Static spectrogram rendering |
| `EnergyDraw` | Signal energy profile |

**Design Pattern**: Qt Widget inheritance

```cpp
QWidget
  └─ AudioDraw
  └─ SignalDraw
  └─ SpectroDraw
     └─ EnergyDrawWidget
```

#### 3. User Interface Layer

**Purpose**: User interaction and application control

**Components**:

| File | Purpose |
|------|---------|
| `MainWindow.*` | Primary application window, orchestrates all functionality |
| `SettingsDialog.*` | Configuration panel for processing parameters |
| `LearningDialog.*` | Learning set management interface |
| `ComparisonWindow.*` | Side-by-side sample comparison |

#### 4. Audio I/O (RtAudio)

**Purpose**: Cross-platform real-time audio input/output

**Implementation**: External RtAudio library (>= 6.0) via system package or submodule

**Platforms Supported**:
- Linux: ALSA
- macOS: CoreAudio
- Windows: DirectSound

**Interface**: Unified `RtAudio` class abstracts platform differences

#### 5. File Format Support

**WAV Support** (`libsndfile`):
- Multi-channel (converted to mono)
- Various bit depths
- Standard WAV format

**MP3 Support** (`mpglib/`):
- Layer 3 MPEG decoding
- Custom integration
- Memory-based streaming

---

## Data Flow

### GUI Mode Workflow

```
User Action (Open File)
    │
    ▼
MainWindow::loadFile()
    │
    ├──► Files::loadAudio()
    │       │
    │       ├──► libsndfile (WAV)
    │       └──► mpglib (MP3)
    │
    ▼
CSignal created with audio data
    │
    ├──► Filter::apply() [optional]
    │       │
    │       └──► Bandpass 2-14 kHz
    │
    ├──► Segmentation by power threshold
    │
    ▼
CSample objects created
    │
    ├──► computeFrequencies()
    │       │
    │       ├──► Window function (Hamming/Blackman)
    │       ├──► CFFT::compute()
    │       └──► Normalize frequencies
    │
    ▼
Update visualization
    │
    ├──► AudioDraw::paint()
    ├──► SignalDraw::paint()
    └──► SpectroDraw::paint()

User Action (Analyze Fragment)
    │
    ▼
Classification
    │
    ├──► test(sample, learningSet)
    │       │
    │       ├──► For each learning sample:
    │       │      └──► differ(sample1, sample2)
    │       │
    │       └──► Select nearest neighbor
    │
    ▼
Display result
```

### CLI Mode Workflow

```
Command Line Arguments
    │
    ▼
Parse parameters
    │
    ├──► Load learning set
    │       │
    │       └──► readLearning(directory)
    │
    ├──► For each input file:
    │       │
    │       ├──► loadAudio()
    │       ├──► Apply filter [optional]
    │       ├──► Segment signal
    │       ├──► Create CSample objects
    │       ├──► Compute frequencies (FFT)
    │       ├──► test() against learning set
    │       └──► Print result
    │
    └──► [OR] Cross-validation mode
            │
            └──► 10-fold cross validation
```

---

## Signal Processing Pipeline

### Step-by-Step Processing

#### 1. Audio Loading

```
Input: WAV or MP3 file
    │
    ├──► Read file header (sample rate, channels, bit depth)
    ├──► Read audio data
    ├──► Convert to mono (if stereo)
    └──► Convert to double precision

Output: double array of samples, sample rate
```

#### 2. Filtering (Optional)

```
Input: Raw audio signal
    │
    └──► Bandpass Filter (2000 Hz - 14000 Hz)
         │
         ├── Optimized for bird vocalizations
         └── Removes low-frequency noise (wind, handling)

Output: Filtered signal
```

#### 3. Segmentation

```
Input: Filtered signal
    │
    └──► Power-based segmentation
         │
         ├── Compute power in sliding window
         ├── Compare to threshold (default: 1e-04)
         ├── Merge fragments within hopTime
         └── Extract high-energy segments

Output: List of signal fragments (potential vocalizations)
```

#### 4. Windowing

```
Input: Signal fragment
    │
    └──► Apply window function
         │
         ├── Hamming Window (default)
         ├── Blackman Window (alternative)
         └── Hanning Window (alternative)

Output: Windowed signal (reduces spectral leakage)
```

#### 5. FFT Computation

```
Input: Windowed signal (256 or 512 samples)
    │
    └──► FFTW3 Fast Fourier Transform
         │
         ├── Real-to-complex FFT
         ├── Extract magnitude spectrum
         └── Select frequency range (FIRST_FREQ to LAST_FREQ)

Output: Frequency spectrum (52 bins for 256-point FFT)
```

#### 6. Normalization

```
Input: Raw frequency magnitudes
    │
    └──► Normalization
         │
         ├── Find maximum magnitude
         ├── Divide all bins by maximum
         └── Apply SNR threshold check

Output: SFrequencies (normalized 0-1 range)
```

### Mathematical Details

#### FFT Configuration

```
FFT_SIZE = 256 samples
FIRST_FREQ = 12 (bin index)
LAST_FREQ = 64 (bin index)
COUNT_FREQ = 52 frequency bins

Frequency resolution = sampleRate / FFT_SIZE
Bin frequency = (bin_index * sampleRate) / FFT_SIZE

For 44100 Hz sample rate:
  Resolution ≈ 172 Hz
  Bin 12 ≈ 2064 Hz
  Bin 64 ≈ 11008 Hz
```

#### Window Functions

**Hamming Window**:
```
w(n) = 0.54 - 0.46 * cos(2π * n / N)
```

**Blackman Window**:
```
w(n) = 0.42 - 0.5 * cos(2π * n / (N-1)) + 0.08 * cos(4π * n / (N-1))
```

#### Power Computation

```
power = Σ(sample[i]²) / N
```

---

## Classification Algorithm

### Nearest Neighbor Implementation

#### Algorithm Overview

```
function test(sample, learningSet):
    minDifference = INFINITY
    bestMatch = NULL

    for each learningSample in learningSet:
        diff = differ(sample, learningSample)

        if diff < minDifference:
            minDifference = diff
            bestMatch = learningSample

    if minDifference < CUTOFF_THRESHOLD:
        return bestMatch.birdId
    else:
        return UNKNOWN
```

#### Difference Metric

```cpp
double CSample::differ(CSample& other) {
    double totalDiff = 0.0;
    int compareCount = min(freqCount, other.freqCount);

    for (int i = 0; i < compareCount; i++) {
        for (int freq = 0; freq < COUNT_FREQ; freq++) {
            double diff = abs(frequencies[i].freq[freq] -
                            other.frequencies[i].freq[freq]);
            totalDiff += diff;
        }
    }

    return totalDiff / (compareCount * COUNT_FREQ);
}
```

This computes the **mean absolute difference** across all frequency bins and time segments.

#### Parameters

```
SNR_MIN       = 3.0    // Signal-to-noise ratio threshold
CUTOFF        = 0.255  // Maximum difference for positive match (25.5%)
POWER_CUTOFF  = 1e-04  // Minimum signal power threshold
```

### Cross-Validation

**10-Fold Cross Validation**:

```
1. Partition learning set into 10 equal subsets
2. For fold = 1 to 10:
   a. Use subset[fold] as test set
   b. Use remaining 9 subsets as training set
   c. Test each sample in test set
   d. Record accuracy
3. Report average accuracy across all folds
```

---

## Module Documentation

### detect/Audio Module

#### Class Hierarchy

```
CSignal (base class)
├── Attributes:
│   ├── name: string
│   ├── framesCount: int
│   ├── sampleRate: int
│   └── frames: double*
├── Methods:
│   ├── loadAudio(filename)
│   ├── saveAudio(filename)
│   └── computePower(start, count)
│
├─► CSample (derived)
│   ├── Additional Attributes:
│   │   ├── frequencies: SFrequencies*
│   │   ├── origFrequencies: OrigFrequencies*
│   │   ├── birdId: uint
│   │   ├── id: uint
│   │   ├── startSample, endSample: uint
│   │   └── freqCount: uint
│   └── Methods:
│       ├── differ(other)
│       ├── consume(other)
│       ├── saveFrequencies(file)
│       └── normalize()
│
└─► CAudio (derived)
    └── Attributes:
        └── samples: list<CSample*>
```

#### CFFT Instance

```cpp
class CFFT {
public:
    CFFT();  // Public constructor
    void compute(double* in, SFrequencies& out);
};

// Usage:
CFFT fft;
fft.compute(audioData, freqOutput);
```

**Design**: FFT context is constructed and passed where needed to avoid global state.

---

### Drawers Module

#### Painter Pattern

```cpp
class AudioDraw : public QWidget {
protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);

        // Draw waveform
        for (int i = 0; i < framesCount; i++) {
            int x = mapToWidget(i);
            int y = mapAmplitude(frames[i]);
            painter.drawPoint(x, y);
        }

        // Draw energy markers (green highlights)
        // Draw selection
    }
};
```

#### Spectrogram Rendering

**Color Schemes**:
- Black & White: Intensity mapped to grayscale
- Color Black: Hot colormap on black background
- Color White: Cool colormap on white background

**Logarithmic Scale**:
```
displayValue = log10(magnitude + epsilon) * scaleFactor
```

---

## Design Patterns

### Patterns Used

1. **FFT Context** (`CFFT` passed via dependency injection)
   - Ensures single FFTW plan instance
   - Global access point

2. **Template Method** (Window functions)
   - Common structure with varying implementations
   - Hamming, Blackman, Hanning

3. **Strategy Pattern** (Implicit in filters)
   - Different filtering strategies
   - Bandpass, MP3 filter

4. **Factory Method** (File loading)
   - Abstract file loading (WAV vs MP3)
   - Files class provides unified interface

5. **Observer** (Qt signals/slots)
   - GUI updates on data changes
   - Event-driven architecture

### Anti-Patterns Present

⚠️ See [CODE_REVIEW.md](CODE_REVIEW.md) for issues:

- **God Object**: `MainWindow` handles too many responsibilities
- **Global State**: `SNR_MIN` global variable
- **Manual Memory Management**: Raw pointers instead of smart pointers
- **Magic Numbers**: Hardcoded constants without explanation

---

## Dependencies

### External Libraries

#### FFTW3 (Fast Fourier Transform)

**Purpose**: Efficient FFT computation
**License**: GPL
**Usage**: Core frequency analysis

```cpp
#include <fftw3.h>

fftw_plan plan = fftw_plan_r2r_1d(n, in, out,
                                  FFTW_R2HC, FFTW_ESTIMATE);
fftw_execute(plan);
```

#### libsndfile (Audio File I/O)

**Purpose**: WAV file reading/writing
**License**: LGPL
**Usage**: Primary audio format

```cpp
#include <sndfile.h>

SNDFILE* file = sf_open(filename, SFM_READ, &sfinfo);
sf_read_double(file, buffer, frames);
sf_close(file);
```

#### Qt4 (GUI Framework)

**Purpose**: Cross-platform GUI and widgets
**License**: LGPL/GPL
**Usage**: Entire UI layer

**Key Qt Classes Used**:
- `QMainWindow`, `QDialog`
- `QWidget`, `QPainter`
- `QListView`, `QTableWidget`
- `QFileDialog`, `QMessageBox`

#### RtAudio (Real-time Audio)

**Purpose**: Cross-platform audio I/O
**License**: MIT-like
**Usage**: Recording functionality (system library >= 6.0)

#### mpglib (MP3 Decoding)

**Purpose**: MP3 file support
**License**: LGPL
**Usage**: Alternative audio format

---

### Dependency Graph

```
BSC Application
    │
    ├──► Qt4 (GUI, Widgets, File Dialogs)
    │
    ├──► FFTW3 (FFT Computation)
    │
    ├──► libsndfile (WAV I/O)
    │
    ├──► mpglib (MP3 Decoding)
    │
    ├──► RtAudio
    │      │
    │      ├──► ALSA (Linux)
    │      ├──► CoreAudio (macOS)
    │      └──► DirectSound (Windows)
    │
    └──► pthread (Threading)
```

---

## Future Architecture

### Planned Improvements

See [TODO](TODO) and [CODE_REVIEW.md](CODE_REVIEW.md) for details.

#### Short-Term Refactoring

1. **Separate Concerns**:
```
MainWindow (current: ~2000 lines)
    │
    ├──► AudioController (audio operations)
    ├──► VisualizationController (drawing coordination)
    ├──► LearningController (learning set management)
    └──► SettingsController (configuration)
```

2. **Modern C++ Migration**:
```cpp
// Current:
double* frames;
frames = new double[count];
delete[] frames;

// Future:
std::vector<double> frames;
frames.resize(count);
// Automatic cleanup
```

3. **CMake Build System**:
```cmake
cmake_minimum_required(VERSION 3.10)
project(BirdSpeciesClassifier)

find_package(Qt4 REQUIRED)
find_package(FFTW3 REQUIRED)
find_package(SndFile REQUIRED)

add_executable(BSC ${SOURCES})
target_link_libraries(BSC Qt4::QtGui FFTW3::fftw3 SndFile::sndfile)
```

#### Medium-Term Architecture

4. **Plugin System** for classification algorithms:
```cpp
class IClassifier {
public:
    virtual ~IClassifier() = default;
    virtual BirdID classify(const CSample& sample) = 0;
};

class NearestNeighborClassifier : public IClassifier { };
class NeuralNetworkClassifier : public IClassifier { };
```

5. **Observer Pattern** for processing pipeline:
```cpp
class IProcessingObserver {
public:
    virtual void onProgress(int percent) = 0;
    virtual void onComplete(const Result& result) = 0;
    virtual void onError(const Error& error) = 0;
};
```

#### Long-Term Vision

6. **Machine Learning Integration**:
   - Deep learning models (TensorFlow/PyTorch integration)
   - Neural network classifiers
   - Transfer learning from pre-trained models

7. **Distributed Processing**:
   - Client-server architecture
   - Cloud-based classification
   - Large-scale batch processing

8. **Modern GUI**:
   - Qt5/Qt6 migration
   - Responsive design
   - Web-based interface option

---

## Performance Considerations

### Bottlenecks

1. **FFT Computation**: Most CPU-intensive operation
   - Mitigated by FFTW3 optimization
   - Consider GPU acceleration for real-time

2. **File I/O**: Large audio files
   - Consider streaming instead of loading entirely
   - Lazy loading for batch processing

3. **GUI Rendering**: Redrawing large spectrograms
   - Implement caching
   - Use painter optimization

### Optimization Opportunities

```cpp
// Current: Creates new frequencies each time
SFrequencies* frequencies = new SFrequencies[count];

// Future: Reuse allocated memory
class FrequencyPool {
    std::vector<SFrequencies> pool;
public:
    SFrequencies* acquire();
    void release(SFrequencies* freq);
};
```

---

## Security Considerations

### Input Validation

- File path sanitization
- Audio file format validation
- Buffer overflow prevention (see CODE_REVIEW.md)

### Resource Limits

- Maximum file size checks
- Memory allocation limits
- Timeout for processing

---

## Testing Architecture

### Planned Test Structure

```
tests/
├── unit/
│   ├── test_fft.cpp
│   ├── test_filtering.cpp
│   ├── test_classification.cpp
│   └── test_signal.cpp
├── integration/
│   ├── test_file_loading.cpp
│   ├── test_pipeline.cpp
│   └── test_learning.cpp
└── performance/
    ├── benchmark_fft.cpp
    └── benchmark_classification.cpp
```

---

## Conclusion

Bird Species Classifier follows a layered architecture with clear separation between signal processing, classification logic, and user interface. The current design is functional but would benefit from modernization to current C++ standards and design patterns.

For implementation details, see source code.
For improvements needed, see [CODE_REVIEW.md](CODE_REVIEW.md).
For contribution guidelines, see [CONTRIBUTING.md](CONTRIBUTING.md).

---

**Document Version**: 1.0
**Last Updated**: 2026-01-18
**Corresponds to**: BSC v0.83
