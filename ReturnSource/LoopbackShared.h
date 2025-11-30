#pragma once
#include <JuceHeader.h>

#if JUCE_WINDOWS
#include <windows.h>
#endif

// Configuration that can be changed by the Return plugin
struct LoopbackConfig
{
    std::atomic<float> bufferLengthMs{ 500.0f }; // Adjustable delay 50ms-3000ms
    std::atomic<float> feedbackAmount{ 0.0f };   // 0.0 to 0.95
    std::atomic<bool> feedbackReduction{ false }; // Enable anti-feedback
    std::atomic<float> smoothingAmount{ 0.8f };   // 0.0-1.0, higher = smoother
};

// Ring buffer structure that lives in shared memory
struct SharedRingBuffer
{
    static constexpr int bufferSizeSamples = 524288; // 4x larger for safety

    std::atomic<int> writePos{ 0 };
    std::atomic<int> readPos{ 0 };
    float leftChannel[bufferSizeSamples];
    float rightChannel[bufferSizeSamples];
    std::atomic<int64_t> totalWritten{ 0 };
    std::atomic<int64_t> totalRead{ 0 };

    LoopbackConfig config;

    // Anti-feedback: track recent peak levels
    std::atomic<float> recentPeakL{ 0.0f };
    std::atomic<float> recentPeakR{ 0.0f };
};

class LoopbackShared
{
public:
    static LoopbackShared& getInstance()
    {
        static LoopbackShared instance;
        return instance;
    }

    bool isInitialized() const { return sharedBuffer != nullptr; }

    void write(const float* left, const float* right, int numSamples, double /*sampleRate*/) noexcept
    {
        if (!sharedBuffer) return;

        const int mask = SharedRingBuffer::bufferSizeSamples - 1;
        int writeIndex = sharedBuffer->writePos.load(std::memory_order_acquire);

        // Apply feedback if enabled
        float feedback = sharedBuffer->config.feedbackAmount.load(std::memory_order_relaxed);
        bool antiFeedback = sharedBuffer->config.feedbackReduction.load(std::memory_order_relaxed);

        for (int i = 0; i < numSamples; ++i)
        {
            int idx = (writeIndex + i) & mask;

            float leftSample = left[i];
            float rightSample = right ? right[i] : left[i];

            // Mix with feedback from buffer
            if (feedback > 0.001f)
            {
                leftSample += sharedBuffer->leftChannel[idx] * feedback;
                rightSample += sharedBuffer->rightChannel[idx] * feedback;

                // Anti-feedback: compress peaks
                if (antiFeedback)
                {
                    if (std::abs(leftSample) > 0.95f)
                        leftSample *= 0.95f / std::abs(leftSample);
                    if (std::abs(rightSample) > 0.95f)
                        rightSample *= 0.95f / std::abs(rightSample);
                }
            }

            sharedBuffer->leftChannel[idx] = leftSample;
            sharedBuffer->rightChannel[idx] = rightSample;
        }

        sharedBuffer->writePos.store((writeIndex + numSamples) & mask, std::memory_order_release);
        sharedBuffer->totalWritten.fetch_add(numSamples, std::memory_order_relaxed);
    }

    void read(float* left, float* right, int numSamples, double sampleRate, bool isPlaying) noexcept
    {
        if (!sharedBuffer)
        {
            juce::FloatVectorOperations::clear(left, numSamples);
            if (right) juce::FloatVectorOperations::clear(right, numSamples);
            return;
        }

        // Only stop if explicitly requested (manual stop button)
        // Don't stop just because transport isn't playing - some hosts don't report this correctly
        // The isPlaying parameter now only controls the manual stop feature

        const int mask = SharedRingBuffer::bufferSizeSamples - 1;
        int writeIndex = sharedBuffer->writePos.load(std::memory_order_acquire);
        int currentReadPos = sharedBuffer->readPos.load(std::memory_order_acquire);

        // Calculate target read position based on desired delay
        float delayMs = sharedBuffer->config.bufferLengthMs.load(std::memory_order_relaxed);
        int delaySamples = static_cast<int>((delayMs / 1000.0f) * sampleRate);
        delaySamples = juce::jlimit(2048, SharedRingBuffer::bufferSizeSamples / 2, delaySamples);

        // Target read position that maintains the desired delay
        int targetReadPos = (writeIndex - delaySamples) & mask;

        // Check if we have enough samples available
        int available = (writeIndex - currentReadPos) & mask;
        if (available < delaySamples / 2)
        {
            // Not enough data yet, output silence to prevent artifacts
            juce::FloatVectorOperations::clear(left, numSamples);
            if (right) juce::FloatVectorOperations::clear(right, numSamples);
            return;
        }

        for (int i = 0; i < numSamples; ++i)
        {
            // Simple read from target position (no smoothing for now to test)
            int readIdx = (targetReadPos + i) & mask;

            left[i] = sharedBuffer->leftChannel[readIdx];
            if (right) right[i] = sharedBuffer->rightChannel[readIdx];
        }

        // Update read position to follow target
        sharedBuffer->readPos.store((targetReadPos + numSamples) & mask, std::memory_order_release);

        sharedBuffer->totalRead.fetch_add(numSamples, std::memory_order_relaxed);
    }

    // Configuration setters (called from Return plugin)
    void setBufferLength(float ms) noexcept
    {
        if (sharedBuffer)
            sharedBuffer->config.bufferLengthMs.store(juce::jlimit(50.0f, 3000.0f, ms), std::memory_order_relaxed);
    }

    void setFeedback(float amount) noexcept
    {
        if (sharedBuffer)
            sharedBuffer->config.feedbackAmount.store(juce::jlimit(0.0f, 0.95f, amount), std::memory_order_relaxed);
    }

    void setFeedbackReduction(bool enabled) noexcept
    {
        if (sharedBuffer)
            sharedBuffer->config.feedbackReduction.store(enabled, std::memory_order_relaxed);
    }

    void setSmoothing(float amount) noexcept
    {
        if (sharedBuffer)
            sharedBuffer->config.smoothingAmount.store(juce::jlimit(0.0f, 0.99f, amount), std::memory_order_relaxed);
    }

    // Buffer management functions
    void clearBuffer() noexcept
    {
        if (!sharedBuffer) return;

        // Clear all audio data
        for (int i = 0; i < SharedRingBuffer::bufferSizeSamples; ++i)
        {
            sharedBuffer->leftChannel[i] = 0.0f;
            sharedBuffer->rightChannel[i] = 0.0f;
        }

        // Reset read/write positions
        sharedBuffer->writePos.store(0, std::memory_order_release);
        sharedBuffer->readPos.store(0, std::memory_order_release);
    }

    void fadeOutBuffer(float fadeTimeMs, double sampleRate) noexcept
    {
        if (!sharedBuffer) return;

        const int mask = SharedRingBuffer::bufferSizeSamples - 1;
        int writeIndex = sharedBuffer->writePos.load(std::memory_order_acquire);
        int readIndex = sharedBuffer->readPos.load(std::memory_order_acquire);

        int available = (writeIndex - readIndex) & mask;
        int fadeSamples = static_cast<int>((fadeTimeMs / 1000.0f) * sampleRate);
        fadeSamples = juce::jmin(fadeSamples, available);

        // Apply fade to existing content
        for (int i = 0; i < fadeSamples; ++i)
        {
            int idx = (readIndex + i) & mask;
            float gain = 1.0f - (static_cast<float>(i) / fadeSamples);
            sharedBuffer->leftChannel[idx] *= gain;
            sharedBuffer->rightChannel[idx] *= gain;
        }

        // Clear remaining samples
        for (int i = fadeSamples; i < available; ++i)
        {
            int idx = (readIndex + i) & mask;
            sharedBuffer->leftChannel[idx] = 0.0f;
            sharedBuffer->rightChannel[idx] = 0.0f;
        }
    }

    int getNumAvailable() const noexcept
    {
        if (!sharedBuffer) return 0;

        int writeIndex = sharedBuffer->writePos.load(std::memory_order_acquire);
        int readIndex = sharedBuffer->readPos.load(std::memory_order_acquire);
        return (writeIndex - readIndex) & (SharedRingBuffer::bufferSizeSamples - 1);
    }

    int64_t getTotalWritten() const noexcept
    {
        return sharedBuffer ? sharedBuffer->totalWritten.load(std::memory_order_relaxed) : 0;
    }

    int64_t getTotalRead() const noexcept
    {
        return sharedBuffer ? sharedBuffer->totalRead.load(std::memory_order_relaxed) : 0;
    }

    float getBufferLength() const noexcept
    {
        return sharedBuffer ? sharedBuffer->config.bufferLengthMs.load(std::memory_order_relaxed) : 500.0f;
    }

    float getFeedback() const noexcept
    {
        return sharedBuffer ? sharedBuffer->config.feedbackAmount.load(std::memory_order_relaxed) : 0.0f;
    }

    bool getFeedbackReduction() const noexcept
    {
        return sharedBuffer ? sharedBuffer->config.feedbackReduction.load(std::memory_order_relaxed) : false;
    }

private:
    LoopbackShared()
    {
#if JUCE_WINDOWS
        const char* mapName = "Global\\LoopbackAudioBuffer_V2";

        hMapFile = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(SharedRingBuffer),
            mapName
        );

        if (hMapFile == NULL)
        {
            DBG("Failed to create file mapping");
            return;
        }

        bool existed = (GetLastError() == ERROR_ALREADY_EXISTS);

        sharedBuffer = (SharedRingBuffer*)MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            sizeof(SharedRingBuffer)
        );

        if (sharedBuffer == NULL)
        {
            DBG("Failed to map view of file");
            CloseHandle(hMapFile);
            hMapFile = NULL;
            return;
        }

        if (!existed)
        {
            memset(sharedBuffer, 0, sizeof(SharedRingBuffer));
            // Initialize config with defaults
            sharedBuffer->config.bufferLengthMs.store(500.0f);
            sharedBuffer->config.feedbackAmount.store(0.0f);
            sharedBuffer->config.feedbackReduction.store(false);
            sharedBuffer->config.smoothingAmount.store(0.8f);
            DBG("Created new shared memory region V2");
        }
        else
        {
            DBG("Opened existing shared memory region V2");
        }
#endif
    }

    ~LoopbackShared()
    {
#if JUCE_WINDOWS
        if (sharedBuffer)
        {
            UnmapViewOfFile(sharedBuffer);
            sharedBuffer = nullptr;
        }
        if (hMapFile)
        {
            CloseHandle(hMapFile);
            hMapFile = NULL;
        }
#endif
    }

    LoopbackShared(const LoopbackShared&) = delete;
    LoopbackShared& operator=(const LoopbackShared&) = delete;

#if JUCE_WINDOWS
    HANDLE hMapFile = NULL;
#endif
    SharedRingBuffer* sharedBuffer = nullptr;
};