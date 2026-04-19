#include "CircularBuffer.h"

template <size_t BUFFER_SIZE>
void CircularBuffer<BUFFER_SIZE>::add(uint16_t value) {
    runningSum -= buffer[writeIndex];
    buffer[writeIndex] = value;
    
    runningSum += value;
    writeIndex = (writeIndex + 1) % BUFFER_SIZE;

    if (currentSize < BUFFER_SIZE) {
        currentSize++;
    }
}

template <size_t BUFFER_SIZE>
void CircularBuffer<BUFFER_SIZE>::reset() {
    buffer.fill(0);
    writeIndex = 0;
    currentSize = 0;
    }

template <size_t BUFFER_SIZE>
float CircularBuffer<BUFFER_SIZE>::getMean(){
    if (currentSize == 0) return 0.0f; 
    return (runningSum * 1.0f) / currentSize;
}