#ifndef ALLOCMEMORY_H
#define ALLOCMEMORY_H

#include <iostream>
#include <memory>

using namespace std;
class MemoryPool {
    unsigned char* m_memPool; // Pointer to allocated memory pool
    size_t m_poolSize; // Total pool size
    size_t m_currentOffset; // current offset
public:
    explicit MemoryPool(size_t size = 10240) :m_poolSize (size), m_currentOffset(0) {
        m_memPool = new unsigned char[m_poolSize];
        //cout << "Memory pool created with size of " << " bytes \n";
    }

    ~MemoryPool() {
        delete[] m_memPool;
    }

    // We need to consider alignment due to padding
    // allocate(sizeof(object), alignof(object) )
    void* allocate(size_t size, size_t alignment) {
        void* currentPtr = m_memPool + m_currentOffset;
        size_t memAvailable = m_poolSize - m_currentOffset;
        //NOTE: std::align will automatically subtract the space used from memAvailable here
        // std::align returns a ptr to the first byte of aligned storage
        void* alignedPtr = align(alignment, size, currentPtr, memAvailable );

        if (alignedPtr == nullptr) {
            cerr << "Not enough memory available\n";
            throw bad_alloc();
        }

        size_t alignedOffset = static_cast<unsigned char *>(alignedPtr) - m_memPool;
        m_currentOffset = alignedOffset + size;

        return alignedPtr;
    }

    void deallocate(void* ptr) {
        cout << "IMPLEMENT ME\n";
    }
};


#endif
