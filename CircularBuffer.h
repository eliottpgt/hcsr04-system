#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <cmath>

template <size_t BUFFER_SIZE>
class CircularBuffer {
private:
    std::array<uint16_t, BUFFER_SIZE> buffer = {0};
    size_t writeIndex = 0;
    size_t currentSize = 0;
    uint32_t runningSum = 0;
    double runningSumSq = 0;

public:
    CircularBuffer() = default;

    void add(uint16_t value) {
        uint16_t oldValue = buffer[writeIndex];
        runningSum -= oldValue;
        runningSumSq -= (static_cast<double>(oldValue) * oldValue);

        buffer[writeIndex] = value;
        runningSum += value;
        runningSumSq += (static_cast<double>(value) * value);

        writeIndex = (writeIndex + 1) % BUFFER_SIZE;
        if (currentSize < BUFFER_SIZE) currentSize++;
    }

    float getMean() const {
        if (currentSize == 0) return 0.0f;
        return static_cast<float>(runningSum) / currentSize;
    }

    float getStandardDeviation() const {
        if (currentSize < 2) return 0.0f;
        double n = static_cast<double>(currentSize);
        double mean = static_cast<double>(runningSum) / n;
        double variance = (runningSumSq / n) - (mean * mean);
        if (variance < 0) variance = 0;
        return static_cast<float>(std::sqrt(variance));
    }

    void reset() {
        buffer.fill(0);
        writeIndex = 0;
        currentSize = 0;
        runningSum = 0;
        runningSumSq = 0;
    }
};