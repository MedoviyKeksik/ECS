#pragma once

#include <cassert>
#include <list>

#include "../api.h"
#include "allocators/pool_allocator.h"

namespace ecs
{
namespace memory
{

template <class OBJECT_TYPE, std::size_t MAX_CHUNK_OBJECTS>
class ECS_API MemoryChunkAllocator : protected memory::GlobalMemoryUser
{
    static const std::size_t MAX_OBJECTS = MAX_CHUNK_OBJECTS;

    // Byte size to fit approx. MAX_CHUNK_OBJECTS objects.
    static const std::size_t ALLOCATE_SIZE =
        (sizeof(OBJECT_TYPE) + alignof(OBJECT_TYPE)) * MAX_OBJECTS;

    const char* allocatorTag;

public:
    using Allocator  = memory::allocator::PoolAllocator;
    using ObjectList = std::list<OBJECT_TYPE*>;

    // Summary:	Helper struct to capsule an allocator and object list. The
    // object list is used to keep track of objects start addresses in memory
    // managed by the allocator.
    class MemoryChunk
    {
    public:
        Allocator* allocator;
        ObjectList objects;

        uptr chunkStart;
        uptr chunkEnd;

        MemoryChunk(Allocator* allocaor)
            : allocator(allocaor)
        {
            this->chunkStart =
                reinterpret_cast<uptr>(allocator->GetMemoryAddress());
            this->chunkEnd = this->chunkStart + ALLOCATE_SIZE;
            this->objects.clear();
        }

    }; // class EntityMemoryChunk

    using MemoryChunks = std::list<MemoryChunk*>;

    // Summary:	An iterator for linear search actions in allocated memory
    // chungs.
    class iterator
        : public std::iterator<std::forward_iterator_tag, OBJECT_TYPE>
    {
        typename MemoryChunks::iterator currentChunk;
        typename MemoryChunks::iterator end;

        typename ObjectList::iterator currentObject;

    public:
        iterator(typename MemoryChunks::iterator begin,
                 typename MemoryChunks::iterator end)
            : currentChunk(begin)
            , end(end)
        {
            if (begin != end)
            {
                assert((*this->currentChunk) != nullptr);
                this->currentObject = (*this->currentChunk)->objects.begin();
            }
            else
            {
                this->currentObject = (*std::prev(this->end))->objects.end();
            }
        }

        inline iterator& operator++()
        {
            // move to next object in current chunk
            this->currentObject++;

            // if we reached end of list, move to next chunk
            if (this->currentObject == (*this->currentChunk)->objects.end())
            {
                this->currentChunk++;

                if (this->currentChunk != this->end)
                {
                    // set object iterator to begin of next chunk list
                    assert((*this->currentChunk) != nullptr);
                    this->currentObject =
                        (*this->currentChunk)->objects.begin();
                }
            }

            return *this;
        }

        inline OBJECT_TYPE& operator*() const { return *this->currentObject; }
        inline OBJECT_TYPE* operator->() const { return *this->currentObject; }

        inline bool operator==(iterator& other)
        {
            return ((this->currentChunk == other.currentChunk) &&
                    (this->currentObject == other.currentObject));
        }
        inline bool operator!=(iterator& other)
        {
            return ((this->currentChunk != other.currentChunk) &&
                    (this->currentObject != other.currentObject));
        }

    }; // ComponentContainer::iterator

protected:
    MemoryChunks chunks;

public:
    MemoryChunkAllocator(const char* allocatorTag = nullptr)
        : allocatorTag(allocatorTag)
    {

        // create initial chunk
        Allocator* allocator =
            new Allocator(ALLOCATE_SIZE,
                          Allocate(ALLOCATE_SIZE, allocatorTag),
                          sizeof(OBJECT_TYPE),
                          alignof(OBJECT_TYPE));
        this->chunks.push_back(new MemoryChunk(allocator));
    }

    virtual ~MemoryChunkAllocator()
    {
        // make sure all entities will be released!
        for (auto chunk : this->chunks)
        {
            for (auto obj : chunk->objects)
                ((OBJECT_TYPE*)obj)->~OBJECT_TYPE();

            chunk->objects.clear();

            // free allocated allocator memory
            Free((void*)chunk->allocator->GetMemoryAddress());
            delete chunk->allocator;
            chunk->allocator = nullptr;

            // delete helper chunk object
            delete chunk;
            chunk = nullptr;
        }
    }

    void* CreateObject()
    {
        void* slot = nullptr;

        // get next free slot
        for (auto chunk : this->chunks)
        {
            if (chunk->objects.size() > MAX_OBJECTS)
                continue;

            slot = chunk->allocator->Allocate(sizeof(OBJECT_TYPE),
                                              alignof(OBJECT_TYPE));
            if (slot != nullptr)
            {
                chunk->objects.push_back((OBJECT_TYPE*)slot);
                break;
            }
        }

        // all chunks are full... allocate a new one
        if (slot == nullptr)
        {
            Allocator* allocator =
                new Allocator(ALLOCATE_SIZE,
                              Allocate(ALLOCATE_SIZE, this->m_AllocatorTag),
                              sizeof(OBJECT_TYPE),
                              alignof(OBJECT_TYPE));
            MemoryChunk* newChunk = new MemoryChunk(allocator);

            // put new chunk in front
            this->chunks.push_front(newChunk);

            slot = newChunk->allocator->Allocate(sizeof(OBJECT_TYPE),
                                                 alignof(OBJECT_TYPE));

            assert(slot != nullptr &&
                   "Unable to create new object. Out of memory?!");
            newChunk->objects.clear();
            newChunk->objects.push_back((OBJECT_TYPE*)slot);
        }

        return slot;
    }

    void DestroyObject(void* object)
    {
        uptr adr = reinterpret_cast<uptr>(object);

        for (auto chunk : this->chunks)
        {
            if (chunk->chunkStart <= adr && adr < chunk->chunkEnd)
            {
                // note: no need to call d'tor since it was called already by
                // 'delete'
                chunk->objects.remove((OBJECT_TYPE*)object);
                chunk->allocator->Free(object);
                return;
            }
        }

        assert(false && "Failed to delete object. Memory corruption?!");
    }

    inline iterator begin()
    {
        return iterator(this->chunks.begin(), this->chunks.end());
    }
    inline iterator end()
    {
        return iterator(this->chunks.end(), this->chunks.end());
    }

}; // MemoryChunkAllocator

} // namespace memory
} // namespace ecs
