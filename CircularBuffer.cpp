#include "CircularBuffer.h"

template <size_t BUFFER_SIZE>
void CircularBuffer<BUFFER_SIZE>::add(uint16_t value) {
    uint16_t oldValue = buffer[writeIndex];
    runningSum -= oldValue;
    runningSumSq -= (static_cast<double>(oldValue) * oldValue);
    
    buffer[writeIndex] = value;
    runningSum += value;
    runningSumSq += (static_cast<double>(value) * value);

    writeIndex = (writeIndex + 1) % BUFFER_SIZE;
    if (currentSize < BUFFER_SIZE) currentSize++;
}

template <size_t BUFFER_SIZE>
void CircularBuffer<BUFFER_SIZE>::reset() {
    buffer.fill(0);
    writeIndex = 0;
    currentSize = 0;
    runningSum = 0;
    runningSumSq = 0;
}

template <size_t BUFFER_SIZE>
float CircularBuffer<BUFFER_SIZE>::getMean() const {
    if (currentSize < 2) return 0.0f; 
    return (runningSum * 1.0f) / currentSize;
}

template <size_t BUFFER_SIZE>
float CircularBuffer<BUFFER_SIZE>::getStandardDeviation() const {
    if (currentSize < 2) return 0.0f;

    double n = static_cast<double>(currentSize);
    double mean = static_cast<double>(runningSum) / n;
    
    double variance = (runningSumSq / n) - (mean * mean);
    if (variance < 0) variance = 0;

    return static_cast<float>(std::sqrt(variance));
}