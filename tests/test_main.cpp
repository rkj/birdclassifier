/**
 * Main test file for Bird Species Classifier
 *
 * This file provides the main entry point for Google Test.
 * Individual test files are compiled separately.
 */

#include <gtest/gtest.h>

// Main is provided by gtest_main, but we can add global setup here if needed
class GlobalTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        // Global test setup
        // e.g., initialize logging, set up test data paths, etc.
    }

    void TearDown() override {
        // Global test cleanup
    }
};

// Register global test environment
// This is optional - uncomment if you need global setup/teardown
// ::testing::Environment* const global_env =
//     ::testing::AddGlobalTestEnvironment(new GlobalTestEnvironment);

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
