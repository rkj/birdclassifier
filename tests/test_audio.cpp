/**
 * Unit tests for Audio module
 *
 * Tests for:
 * - Window functions (Hamming, Blackman, Hanning)
 * - Power computation
 * - FFT operations
 * - Signal processing
 */

#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "detect/Audio.hxx"

// Test fixture for audio tests
class AudioTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup common test data
    }

    void TearDown() override {
        // Cleanup
    }
};

// ============================================================================
// Power Computation Tests
// ============================================================================

TEST_F(AudioTest, ComputePowerBasicSignal) {
    double frames[] = {1.0, 2.0, 3.0, 4.0};
    double power = computePower(frames, 4);

    // Expected: (1^2 + 2^2 + 3^2 + 4^2) / 4 = 30/4 = 7.5
    EXPECT_DOUBLE_EQ(power, 7.5);
}

TEST_F(AudioTest, ComputePowerZeroSignal) {
    double frames[] = {0.0, 0.0, 0.0, 0.0};
    double power = computePower(frames, 4);

    EXPECT_DOUBLE_EQ(power, 0.0);
}

TEST_F(AudioTest, ComputePowerSingleSample) {
    double frames[] = {5.0};
    double power = computePower(frames, 1);

    // Expected: 5^2 / 1 = 25
    EXPECT_DOUBLE_EQ(power, 25.0);
}

TEST_F(AudioTest, ComputePowerNegativeValues) {
    double frames[] = {-1.0, -2.0, -3.0};
    double power = computePower(frames, 3);

    // Expected: (1 + 4 + 9) / 3 = 14/3 ≈ 4.6667
    EXPECT_NEAR(power, 14.0/3.0, 1e-10);
}

TEST_F(AudioTest, ComputePowerMixedValues) {
    double frames[] = {1.0, -1.0, 2.0, -2.0};
    double power = computePower(frames, 4);

    // Expected: (1 + 1 + 4 + 4) / 4 = 10/4 = 2.5
    EXPECT_DOUBLE_EQ(power, 2.5);
}

// ============================================================================
// Power to dB Conversion Tests
// ============================================================================

TEST_F(AudioTest, PowerTodBZero) {
    // log10(0) is undefined, but we use max(0, ...) so it should return 0
    double db = powerTodB(0.0);
    EXPECT_DOUBLE_EQ(db, 0.0);
}

TEST_F(AudioTest, PowerTodBSmallValue) {
    // Small power value should give reasonable dB
    double db = powerTodB(1e-12);
    EXPECT_GE(db, 0.0);
}

TEST_F(AudioTest, PowerTodBLargeValue) {
    double db = powerTodB(1.0);
    EXPECT_GT(db, 100.0);  // Should be relatively large
}

// ============================================================================
// Window Function Tests
// ============================================================================

TEST_F(AudioTest, HammingWindowSymmetry) {
    const int N = 256;
    double input[N];
    double output[N];

    // Create impulse in center
    for (int i = 0; i < N; i++) {
        input[i] = (i == N/2) ? 1.0 : 0.0;
    }

    HammingWindow(input, output, N);

    // Hamming window should be symmetric
    for (int i = 0; i < N/2; i++) {
        EXPECT_NEAR(output[i], output[N-1-i], 1e-10)
            << "Window not symmetric at index " << i;
    }
}

TEST_F(AudioTest, HammingWindowRange) {
    const int N = 256;
    double input[N];
    double output[N];

    // All ones input
    for (int i = 0; i < N; i++) {
        input[i] = 1.0;
    }

    HammingWindow(input, output, N);

    // Window values should be in [0, 1] range
    for (int i = 0; i < N; i++) {
        EXPECT_GE(output[i], 0.0) << "Negative window value at " << i;
        EXPECT_LE(output[i], 1.0) << "Window value > 1 at " << i;
    }
}

TEST_F(AudioTest, HammingWindowPeakAtCenter) {
    const int N = 256;
    double input[N];
    double output[N];

    for (int i = 0; i < N; i++) {
        input[i] = 1.0;
    }

    HammingWindow(input, output, N);

    // Peak should be near center for Hamming window
    double maxVal = 0.0;
    int maxIdx = 0;
    for (int i = 0; i < N; i++) {
        if (output[i] > maxVal) {
            maxVal = output[i];
            maxIdx = i;
        }
    }

    // Max should be near center (within 10% of window size)
    EXPECT_NEAR(maxIdx, N/2, N/10)
        << "Hamming window peak not near center";
}

TEST_F(AudioTest, BlackmanWindowSymmetry) {
    const int N = 256;
    double input[N];
    double output[N];

    for (int i = 0; i < N; i++) {
        input[i] = 1.0;
    }

    BlackmanWindow(input, output, N);

    // Blackman window should be symmetric
    for (int i = 0; i < N/2; i++) {
        EXPECT_NEAR(output[i], output[N-1-i], 1e-10)
            << "Blackman window not symmetric at index " << i;
    }
}

TEST_F(AudioTest, BlackmanWindowEdgesNearZero) {
    const int N = 256;
    double input[N];
    double output[N];

    for (int i = 0; i < N; i++) {
        input[i] = 1.0;
    }

    BlackmanWindow(input, output, N);

    // Blackman window should be near zero at edges
    EXPECT_NEAR(output[0], 0.0, 0.01)
        << "Blackman window not near zero at start";
    EXPECT_NEAR(output[N-1], 0.0, 0.01)
        << "Blackman window not near zero at end";
}

// ============================================================================
// SFrequencies Tests
// ============================================================================

TEST_F(AudioTest, SFrequenciesConstruction) {
    SFrequencies freq;

    // Should construct without throwing
    SUCCEED();
}

TEST_F(AudioTest, SFrequenciesCopyConstruction) {
    SFrequencies freq1;

    // Initialize with some data
    for (int i = 0; i < COUNT_FREQ; i++) {
        freq1.freq[i] = i * 0.1;
    }

    SFrequencies freq2(freq1);

    // Should have copied data
    for (int i = 0; i < COUNT_FREQ; i++) {
        EXPECT_DOUBLE_EQ(freq2.freq[i], freq1.freq[i])
            << "Copy failed at index " << i;
    }
}

TEST_F(AudioTest, SFrequenciesDiffer_IdenticalShouldBeZero) {
    SFrequencies freq1, freq2;

    // Initialize with same data
    for (int i = 0; i < COUNT_FREQ; i++) {
        freq1.freq[i] = i * 0.1;
        freq2.freq[i] = i * 0.1;
    }

    double diff = freq1.differ(freq2);

    EXPECT_DOUBLE_EQ(diff, 0.0)
        << "Identical frequencies should have zero difference";
}

TEST_F(AudioTest, SFrequenciesDiffer_DifferentShouldBeNonZero) {
    SFrequencies freq1, freq2;

    // Initialize with different data
    for (int i = 0; i < COUNT_FREQ; i++) {
        freq1.freq[i] = 0.5;
        freq2.freq[i] = 0.6;
    }

    double diff = freq1.differ(freq2);

    EXPECT_GT(diff, 0.0)
        << "Different frequencies should have non-zero difference";
}

TEST_F(AudioTest, SFrequenciesDiffer_ShouldBeSymmetric) {
    SFrequencies freq1, freq2;

    for (int i = 0; i < COUNT_FREQ; i++) {
        freq1.freq[i] = i * 0.05;
        freq2.freq[i] = i * 0.07;
    }

    double diff1 = freq1.differ(freq2);
    double diff2 = freq2.differ(freq1);

    EXPECT_DOUBLE_EQ(diff1, diff2)
        << "Difference metric should be symmetric";
}

// ============================================================================
// CFFT Tests
// ============================================================================

TEST_F(AudioTest, CFftSingletonInstance) {
    CFFT& fft1 = CFFT::getInstance();
    CFFT& fft2 = CFFT::getInstance();

    // Should return same instance
    EXPECT_EQ(&fft1, &fft2)
        << "CFFT should be a singleton";
}

TEST_F(AudioTest, CFftGetSize) {
    int size = CFFT::sGetFFTsize();

    EXPECT_EQ(size, FFT_SIZE)
        << "FFT size should match FFT_SIZE constant";
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(AudioTest, WindowFunctionDoesNotAmplify) {
    const int N = 256;
    double input[N];
    double output[N];

    // Create signal with amplitude 1.0
    for (int i = 0; i < N; i++) {
        input[i] = 1.0;
    }

    HammingWindow(input, output, N);

    // Window should not amplify (all values should be <= 1.0)
    for (int i = 0; i < N; i++) {
        EXPECT_LE(output[i], 1.0)
            << "Window amplified signal at index " << i;
    }
}

TEST_F(AudioTest, PowerComputationIsConsistent) {
    // Test that power computation gives same result for same data
    double frames[] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double power1 = computePower(frames, 5);
    double power2 = computePower(frames, 5);

    EXPECT_DOUBLE_EQ(power1, power2)
        << "Power computation should be deterministic";
}

// ============================================================================
// Edge Cases and Error Conditions
// ============================================================================

TEST_F(AudioTest, WindowFunctionSmallSize) {
    const int N = 10;
    double input[N];
    double output[N];

    for (int i = 0; i < N; i++) {
        input[i] = 1.0;
    }

    // Should handle small window sizes
    EXPECT_NO_THROW({
        HammingWindow(input, output, N);
    });
}

TEST_F(AudioTest, ConstantsAreReasonable) {
    // Verify that constants are in expected ranges
    EXPECT_GT(FFT_SIZE, 0) << "FFT_SIZE should be positive";
    EXPECT_LT(FFT_SIZE, 10000) << "FFT_SIZE should be reasonable";

    EXPECT_GT(COUNT_FREQ, 0) << "COUNT_FREQ should be positive";
    EXPECT_LT(COUNT_FREQ, FFT_SIZE) << "COUNT_FREQ should be less than FFT_SIZE";

    EXPECT_GE(FIRST_FREQ, 0) << "FIRST_FREQ should be non-negative";
    EXPECT_LT(FIRST_FREQ, LAST_FREQ) << "FIRST_FREQ should be less than LAST_FREQ";

    EXPECT_EQ(COUNT_FREQ, LAST_FREQ - FIRST_FREQ)
        << "COUNT_FREQ should equal LAST_FREQ - FIRST_FREQ";
}

// ============================================================================
// Performance/Regression Tests (basic)
// ============================================================================

TEST_F(AudioTest, WindowFunctionPerformance) {
    const int N = 4096;
    double input[N];
    double output[N];

    for (int i = 0; i < N; i++) {
        input[i] = std::sin(2.0 * PI * i / N);
    }

    // Should complete in reasonable time (this is just a smoke test)
    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < 1000; iter++) {
        HammingWindow(input, output, 256);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 1000 iterations should complete in less than 1 second
    EXPECT_LT(duration.count(), 1000)
        << "Window function too slow: " << duration.count() << "ms for 1000 iterations";
}

// ============================================================================
// Test Summary
// ============================================================================

/*
 * Test Coverage Summary:
 *
 * ✅ Power computation (5 tests)
 * ✅ Power to dB conversion (3 tests)
 * ✅ Window functions (6 tests)
 * ✅ SFrequencies operations (4 tests)
 * ✅ CFFT singleton (2 tests)
 * ✅ Integration tests (2 tests)
 * ✅ Edge cases (2 tests)
 * ✅ Performance tests (1 test)
 *
 * Total: 25 tests
 *
 * These tests provide a foundation for modernization.
 * As we refactor, we'll ensure all tests continue to pass.
 */
