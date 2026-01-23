cmake_minimum_required(VERSION 3.14)
project(BirdSpeciesClassifier VERSION 0.83 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_GUI "Build GUI application" ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# Find libsndfile
pkg_check_modules(SNDFILE REQUIRED sndfile)

# Find FFTW3
pkg_check_modules(FFTW3 REQUIRED fftw3)

# Find pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Platform-specific audio libraries
if(UNIX AND NOT APPLE)
    # Linux - ALSA
    pkg_check_modules(ALSA REQUIRED alsa)
    set(AUDIO_LIBS ${ALSA_LIBRARIES})
    set(AUDIO_DEFINES __LINUX_ALSA__)
elseif(APPLE)
    # macOS - CoreAudio
    find_library(COREAUDIO_LIBRARY CoreAudio)
    set(AUDIO_LIBS ${COREAUDIO_LIBRARY})
    set(AUDIO_DEFINES __MACOSX_CORE__)
elseif(WIN32)
    # Windows - DirectSound
    set(AUDIO_LIBS dsound)
    set(AUDIO_DEFINES __WINDOWS_DS__)
endif()

# Source files for core library (non-GUI)
set(CORE_SOURCES
    detect/Audio.cpp
    detect/detect.cpp
    detect/Files.cpp
    detect/Filter.cpp
    detect/Manager.cpp
    RtAudio/RtAudio.cpp
)

set(CORE_HEADERS
    detect/Audio.hxx
    detect/detect.hxx
    detect/Files.hxx
    detect/Filter.hxx
    detect/Manager.hxx
    RtAudio/RtAudio.h
    RtAudio/RtError.h
)

# Create core library (shared between GUI and tests)
add_library(bsc_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(bsc_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/detect
    ${CMAKE_CURRENT_SOURCE_DIR}/Drawers
    ${CMAKE_CURRENT_SOURCE_DIR}/RtAudio
    ${SNDFILE_INCLUDE_DIRS}
    ${FFTW3_INCLUDE_DIRS}
)

target_link_libraries(bsc_core PUBLIC
    ${SNDFILE_LIBRARIES}
    ${FFTW3_LIBRARIES}
    ${AUDIO_LIBS}
    Threads::Threads
    m
)

target_compile_definitions(bsc_core PUBLIC
    ${AUDIO_DEFINES}
)

# GUI application (optional)
if(BUILD_GUI)
    find_package(Qt6 COMPONENTS Core Gui Widgets)

    if(Qt6_FOUND)
        # Enable Qt6 automatic MOC, UIC, and RCC processing
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)

        # Add Qt to bsc_core for GUI-aware code (e.g., readLearning with QProgressBar)
        target_link_libraries(bsc_core PUBLIC Qt6::Core Qt6::Widgets)

        set(GUI_SOURCES
            MainWindow.cpp
            SettingsDialog.cpp
            LearningDialog.cpp
            ComparisonWindow.cpp
            Drawers/AudioDraw.cpp
            Drawers/EnergyDraw.cpp
            Drawers/EnergyDrawWidget.cpp
            Drawers/SignalDraw.cpp
            Drawers/SpectroDraw.cpp
            Drawers/Spectrogram.cpp
        )

        set(GUI_HEADERS
            MainWindow.hxx
            SettingsDialog.hxx
            LearningDialog.hxx
            ComparisonWindow.hxx
            MyListView.hxx
            Drawers/AudioDraw.hxx
            Drawers/EnergyDraw.hxx
            Drawers/EnergyDrawWidget.hxx
            Drawers/SignalDraw.hxx
            Drawers/SpectroDraw.hxx
            Drawers/Spectrogram.hxx
        )

        set(GUI_FORMS
            MainWindow.ui
            SettingsDialog.ui
            LearningDialog.ui
            ComparisonWindow.ui
            Drawers/EnergyDraw.ui
            Drawers/SignalDraw.ui
            Drawers/SpectroDraw.ui
        )

        set(GUI_RESOURCES
            Images.qrc
        )

        add_executable(BSC
            ${GUI_SOURCES}
            ${GUI_HEADERS}
            ${GUI_FORMS}
            ${GUI_RESOURCES}
        )

        target_link_libraries(BSC PRIVATE
            bsc_core
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
        )

        target_include_directories(BSC PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}  # For generated UI headers
            ${CMAKE_CURRENT_BINARY_DIR}/BSC_autogen/include  # For AUTOMOC generated headers
        )

        message(STATUS "GUI application will be built with Qt6")
    else()
        message(WARNING "Qt6 not found - GUI application will not be built")
        message(STATUS "To build GUI, install Qt6 development files (qt6-base-dev)")
    endif()
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS bsc_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

if(TARGET BSC)
    install(TARGETS BSC
        RUNTIME DESTINATION bin
    )
endif()

# Print configuration summary
message(STATUS "=== Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Building tests: ${BUILD_TESTS}")
message(STATUS "Building GUI: ${BUILD_GUI}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Audio backend: ${AUDIO_DEFINES}")
message(STATUS "=========================")
