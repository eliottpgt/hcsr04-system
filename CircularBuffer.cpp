#include "CircularBuffer.h"

template <size_t BUFFER_SIZE>
void CircularBuffer<BUFFER_SIZE>::add(uint16_t value) {
    buffer[writeIndex] = value;
    
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