#pragma once

#include "api.h"

#include <limits.h>

#pragma warning(push)

#pragma warning(disable : 4293)

namespace ecs
{
namespace util
{
namespace internal
{

template <typename handle_value_type, std::size_t version_bits, std::size_t index_bits>
union Handle
{
    static_assert(sizeof(handle_value_type) * CHAR_BIT >= (version_bits + index_bits),
                  "Invalid handle layout. More bits used than base value type can hold!");

    using value_type = handle_value_type;

    static constexpr std::size_t NUM_VERSION_BITS{ version_bits };
    static constexpr std::size_t NUM_INDEX_BITS{ index_bits };

    static constexpr value_type MIN_VERISON{ 0 };
    static constexpr value_type MAX_VERSION{ (1U << NUM_VERSION_BITS) - 2U };
    static constexpr value_type MAX_INDICES{ ((value_type)1U << NUM_INDEX_BITS) - 2U };

    static constexpr value_type INVALID_HANDLE{ std::numeric_limits<value_type>::max() };

public:
    struct
    {
        value_type index : NUM_INDEX_BITS;
        value_type version : NUM_VERSION_BITS;
    };

    Handle() = default;

    Handle(value_type value)
        : value(value)
    {
    }

    Handle(value_type index, value_type version)
        : index(index)
        , version(version)
    {
    }

    inline operator value_type() const { return value; }

private:
    value_type value;
};
} // namespace internal

using Handle32 = internal::Handle<u32, 12, 20>;
#ifdef ECS_64BIT
using Handle64 = internal::Handle<u64, 24, 40>;
#else
using Handle64 = Handle32;
#endif
template <class T, class handle_type, std::size_t grow = 1024>
class ECS_API HandleTable
{
    using Handle = handle_type;

public:
    HandleTable() { this->GrowTable(); }

    ~HandleTable() = default;

    Handle AqcuireHandle(T* rawObject)
    {
        typename Handle::value_type i = 0;
        for (; i < this->m_Table.size(); ++i)
        {
            if (this->m_Table[i].second == nullptr)
            {
                this->m_Table[i].second = rawObject;

                this->m_Table[i].first = ((this->m_Table[i].first + 1) > Handle::MAX_VERSION)
                                             ? Handle::MIN_VERISON
                                             : this->m_Table[i].first + 1;

                return Handle(i, this->m_Table[i].first);
            }
        }
        this->GrowTable();

        this->m_Table[i].first  = 1;
        this->m_Table[i].second = rawObject;

        return Handle(i, this->m_Table[i].first);
    }

    void ReleaseHandle(Handle handle)
    {
        assert((handle.index < this->m_Table.size() && handle.version == this->m_Table[handle.index].first) &&
               "Invalid handle!");
        this->m_Table[handle.index].second = nullptr;
    }

    inline bool IsExpired(Handle handle) const { return this->m_Table[handle.index].first != handle.version; }

    inline Handle operator[](typename Handle::value_type index) const
    {
        assert(index < this->m_Table.size() && "Invalid handle!");
        return Handle(index, this->m_Table[index].first);
    }

    inline T* operator[](Handle handle)
    {
        assert((handle.index < this->m_Table.size() && handle.version == this->m_Table[handle.index].first) &&
               "Invalid handle!");
        return (this->m_Table[handle.index].first == handle.version ? this->m_Table[handle.index].second : nullptr);
    }

private:
    using TableEntry = std::pair<typename Handle::value_type, T*>;

    std::vector<TableEntry> m_Table;

    void GrowTable()
    {
        std::size_t oldSize = this->m_Table.size();

        assert(oldSize < Handle::MAX_INDICES && "Max table capacity reached!");

        std::size_t newSize = std::min(oldSize + grow, (size_t)Handle::MAX_INDICES);

        this->m_Table.resize(newSize);

        for (typename Handle::value_type i = oldSize; i < newSize; ++i)
            this->m_Table[i] = TableEntry(Handle::MIN_VERISON, nullptr);
    }

}; // class HandleTable
} // namespace util
} // namespace ecs

#pragma warning(pop)
