#ifndef ALLOCMEMORY_H
#define ALLOCMEMORY_H

#include <iostream>
#include <memory>

using namespace std;

template<typename T>
class MemoryPool {
    struct MemoryChunk {
        unsigned char *m_data;
        MemoryChunk *m_next;
        size_t m_size;

        explicit MemoryChunk(size_t size) {
            m_data = new unsigned char[size];
            m_size = size;
            m_next = nullptr;
        }
    };

    //unsigned char* m_memPool; // Pointer to allocated memory pool
    MemoryChunk *m_currentChunk; // current chunk
    MemoryChunk *m_chunkHead;
    size_t m_poolSize; // Total memory in terms of chunks allocated
    size_t m_currentChunkOffset;
    size_t m_objectsPerChunk;
    size_t m_chunkCount;

public:
    explicit MemoryPool(size_t objectsPerChunk = 64) noexcept : m_currentChunkOffset(0),
                                                       m_objectsPerChunk(objectsPerChunk),
                                                       m_chunkCount(1)  {
        //Allocate our first chunk
        size_t sizeNeeded = sizeof(T) * objectsPerChunk;
        m_currentChunk = new MemoryChunk(sizeNeeded);
        m_chunkHead = m_currentChunk;
        m_poolSize = sizeNeeded;
        //cout << "Memory pool created with size of " << sizeNeeded << " bytes \n";
    }

    ~MemoryPool() {
        MemoryChunk* current = m_chunkHead;
        while (current != nullptr) {
            MemoryChunk* next = current->m_next;
            delete[] current->m_data;
            delete current;
            current = next;
        }
    }

    void *allocate() {
        if (m_currentChunk == nullptr) { //If allocate is called, the current chunk needs to be reallocated
            m_currentChunk = new MemoryChunk(sizeof(T));
            m_chunkCount = 1;
            m_poolSize = sizeof(T) * m_chunkCount;
        }
        size_t memAvailable = m_currentChunk->m_size - m_currentChunkOffset;
        //cout << "Mem available: " << memAvailable << '\n';
        if (memAvailable < sizeof(T)) {
            //cout << "Creating new chunk\n";
            m_currentChunk->m_next = new MemoryChunk(sizeof(T) * m_objectsPerChunk);
            m_currentChunk = m_currentChunk->m_next;
            m_currentChunkOffset = 0;
            memAvailable = m_currentChunk->m_size - m_currentChunkOffset;
            m_chunkCount++;
            m_poolSize = sizeof(T) * m_chunkCount;
        }
        void* currentPtr = m_currentChunk->m_data + m_currentChunkOffset;
        void* alignedPtr = align(alignof(T), sizeof(T), currentPtr, memAvailable);

        if (alignedPtr == nullptr) {
            throw bad_alloc();
        }

        size_t alignedOffset = static_cast<unsigned char*> (alignedPtr) - m_currentChunk->m_data;
        m_currentChunkOffset = alignedOffset + sizeof(T);

        return alignedPtr;
    }

    void clear() noexcept {
        MemoryChunk *chunk = m_chunkHead;
        while (chunk->m_next != nullptr) {
            delete[] chunk->m_data;
            chunk = chunk->m_next;
        }

        m_chunkCount = 0;
        m_currentChunk = nullptr;
        m_currentChunkOffset = 0;
        m_poolSize = 0;
    }


};


#endif
