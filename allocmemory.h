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

    MemoryChunk *m_currentChunk; // current chunk
    MemoryChunk *m_chunkHead;
    size_t m_poolSize; // Total memory in terms of chunks allocated
    size_t m_currentChunkOffset;
    size_t m_objectsPerChunk;
    size_t m_chunkCount;
    void* m_nextFreeSlot; // Represents the next free block of memory in the free list

    void calcPoolSize() {
        m_poolSize = sizeof(T) * m_objectsPerChunk * m_chunkCount;
    }

public:
    explicit MemoryPool(size_t objectsPerChunk = 64) noexcept : m_currentChunkOffset(0),
                                                                m_objectsPerChunk(objectsPerChunk),
                                                                m_chunkCount(1) {
        //Allocate our first chunk
        size_t sizeNeeded = sizeof(T) * objectsPerChunk;
        m_currentChunk = new MemoryChunk(sizeNeeded);
        m_chunkHead = m_currentChunk;
        m_poolSize = sizeNeeded;
        m_nextFreeSlot = nullptr;
    }

    ~MemoryPool() {
        MemoryChunk* current = m_chunkHead;
        while (current != nullptr) {
            MemoryChunk *next = current->m_next;
            delete[] current->m_data;
            delete current;
            current = next;
        }
    }

    //TODO: ROf5

    // We take our raw pointer and
    void deallocate(void* ptr) noexcept {
        if (ptr == nullptr) return;

        // This tells the compiler to treat the address as if it were a location of a pointer
        // The result of this cast is a void**
        // We dereference the void** and set it equal to the next slot to keep the freelist working
        // Essentially linking the new node to the old list
        *reinterpret_cast<void**>(ptr) = m_nextFreeSlot;
        // Make the new node the head of the free list
        m_nextFreeSlot = ptr;
    }

    void* allocate() {
        if (m_nextFreeSlot != nullptr) {
            //Use recycled memory from the free list
            //cout << "Using recycled memory\n";
            void* recycledPtr = m_nextFreeSlot;

            //This is similar to what is done in the deallocate function
            // We access the head of the free list using the same method and set m_nextFreeSlot to the "next" ptr
            m_nextFreeSlot = *reinterpret_cast<void**>(recycledPtr);

            return recycledPtr;
        } else { //Otherwise, we do the usual operations
            //If clear is called, the current chunk needs to be reallocated
            if (m_currentChunk == nullptr) {
                m_currentChunk = new MemoryChunk(sizeof(T) * m_objectsPerChunk);
                m_chunkCount = 1;
                calcPoolSize();
                m_chunkHead = m_currentChunk;
            }

            size_t memAvailable = m_currentChunk->m_size - m_currentChunkOffset;
            if (memAvailable < sizeof(T)) { // Need a new memory chunk
                m_currentChunk->m_next = new MemoryChunk(sizeof(T) * m_objectsPerChunk);
                m_currentChunk = m_currentChunk->m_next;
                m_currentChunkOffset = 0;
                memAvailable = m_currentChunk->m_size - m_currentChunkOffset;
                m_chunkCount++;
                calcPoolSize();
            }
            void *currentPtr = m_currentChunk->m_data + m_currentChunkOffset;
            void *alignedPtr = align(alignof(T), sizeof(T), currentPtr, memAvailable);

            //This occurs if std::align fails.
            //A consideration for the future is to try to allocate a new MemoryChunk instead of throwing bad_alloc
            if (alignedPtr == nullptr) {
                throw bad_alloc();
            }

            size_t alignedOffset = static_cast<unsigned char *>(alignedPtr) - m_currentChunk->m_data;
            m_currentChunkOffset = alignedOffset + sizeof(T);

            return alignedPtr;
        }
    }

    //Clears all memory chunks
    void clear() noexcept {
        MemoryChunk *current = m_chunkHead;
        while (current != nullptr) {
            MemoryChunk *next = current->m_next;
            delete[] current->m_data;
            delete current;
            current = next;
        }
        m_chunkCount = 0;
        m_currentChunk = nullptr;
        m_currentChunkOffset = 0;
        m_chunkHead = nullptr;
        m_nextFreeSlot = nullptr;
    }
};


#endif
