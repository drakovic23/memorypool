#include "allocmemory.h"
#include <iostream>
#include <chrono>
#include <thread>

#define NUM_ELEMENTS 1000000

class BasicObject {
public:
    int x;
    char p[16];
    BasicObject(int val) : x(val) {}

    void * operator new(size_t size, MemoryPool<BasicObject> &memoryPool){
        return memoryPool.allocate();
    }

    void operator delete(void* ptr, MemoryPool<BasicObject> &memoryPool){
        memoryPool.deallocate(ptr);
    }
};

class BasicObject_ {
public:
    int x;
    char p[16];
    BasicObject_(int val) : x(val) {
    }

};

int main() {
    //Test an allocate all and deallocate all scenario
    // Using new
    auto startDefault = std::chrono::high_resolution_clock::now();

    for (int i = 0;i < NUM_ELEMENTS; i++) {
        BasicObject_* bObj = new BasicObject_(i);
        delete bObj;
    }

    auto endDefault = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> resultDefault = endDefault - startDefault;
    cout << "Default allocator: " << resultDefault.count() << '\n';

    //Using memory pool
    MemoryPool<BasicObject> memoryPool(64);
    auto startPool = std::chrono::high_resolution_clock::now();
    for (int i = 0;i < NUM_ELEMENTS;i++) {
        BasicObject* obj = new (memoryPool) BasicObject(i);
        obj->~BasicObject();
        BasicObject::operator delete(obj, memoryPool);
    }
    auto endPool = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> poolResult = endPool - startPool;
    cout << "Memory Pool: " << poolResult.count() << '\n';

    //Roughly 4x faster
}