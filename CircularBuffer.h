#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <array>
#include <cstdint>
#include <cstddef> // Pour size_t

template <size_t BUFFER_SIZE>
class CircularBuffer {
private:
    std::array<uint16_t, BUFFER_SIZE> buffer = {0};
    size_t writeIndex = 0;
    size_t currentSize = 0;
    uint32_t runningSum = 0;

public:
    CircularBuffer() = default;
    void add(uint16_t value) ;
    void reset();

    inline size_t size() const { return currentSize; }
    inline bool isEmpty() const { return currentSize == 0; }
    inline bool isFull() const { return currentSize == BUFFER_SIZE; }

    float getMean();
};

#endif