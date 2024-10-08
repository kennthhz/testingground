#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <format>
#include "buffercache.h"

extern BufferCache BufferCacheInstance;

const uint32_t InvalidPid = UINT32_MAX;
const uint16_t PageHeaderPadding = 0xEFEF;
const uint16_t RootNode = 0x1;
const uint16_t IntermediateNode = 0x2;
const uint16_t LeafNode = 0x4;
const uint16_t unUsed = 0x8; 
const uint16_t exNodeTypeMask = ~(RootNode | IntermediateNode | LeafNode | unUsed);
const double MaxFillFactor = 0.9;

static void SetNodeType (uint16_t *info, uint16_t type) {
    *info &= exNodeTypeMask;
    *info |= type;
}

static bool IsRootNode(uint16_t info) {
    return (info & RootNode) == RootNode;
}

static bool IsIntermiediateNode(uint16_t info) {
    return (info & IntermediateNode) == IntermediateNode;
}

static bool IsLeafNode(uint16_t info) {
    return (info & LeafNode) == LeafNode;
}

struct BTreePagerHeader {
    // 0-1  Node type: Root, Intermediate, Leaf
    // 2-3  Lock mode: shared lock, exclusive lock, no lock
    // 4-7 Compression mechanism: 0-None, ....
    // 8-15 Resvered
    uint16_t _info;
    
    // Page version
    uint16_t _version;
    
    // Effective fill factor of the current page
    uint16_t _fillFactor;
    
    // Number of the item pointers
    uint16_t _items_count;
    
    // The offset of top item slot. The slot is growing backward from page end.
    uint16_t _upper;
    
    // Parent PID
    uint32_t _p_pid;
    
    // Left sibling PID
    uint32_t _l_pid;
    
    // Right sibling PID
    uint32_t _r_pid;
    
    // Compression CRC checksum
    uint32_t _crc;
    
    // Current page's PID
    uint32_t _pid;
    
    // The PID of rightmost child
    uint32_t _right_child_pid;
    
    // Padding reserved
    uint16_t _padding;
};

constexpr uint32_t BTreePagerHeaderSize = sizeof(BTreePagerHeader);
constexpr uint32_t MaxPageSlotSpace = PageSize - BTreePagerHeaderSize;

// There 2 scenarios:
// 1. Find the BTree node to insert, only pid will be set
// 2. Find the value based on key, data will be set
template <typename TVal>
struct FindResult {
    // The index PID that contains the result.
    // InvalidPid means result not found
    uint32_t pid;
    TVal data;
    
    FindResult(uint32_t p, TVal d):pid(p), data(d) {}
    FindResult(uint32_t p):pid(p) {}
};

template <typename T>
inline void serialize(const T& data, unsigned char* addr) {
    static_assert(std::is_same<T, int>::value || std::is_same<T, std::string>::value ||
                  std::is_same<T, float>::value || std::is_same<T, double>::value ||
                  std::is_same<T, long double>::value || std::is_same<T, bool>::value ||
                  std::is_same<T, long>::value || std::is_same<T, long long>::value,
                  "Type is not supported");

    if constexpr (std::is_same_v<T, std::string>) {
        auto str_size = data.size();
        std::memcpy(addr, &str_size, sizeof(size_t));
        std::memcpy(addr + sizeof(size_t), data.data(), str_size);
    } else {
        std::memcpy(addr, &data, sizeof(T));
    }
}

template <typename T>
inline const T deserialize(unsigned char* addr) {
    if constexpr (std::is_same_v<T, std::string>)
        return std::string(reinterpret_cast<const char*>(addr + sizeof(size_t)), *addr);
    else
        return *(reinterpret_cast<T*>(addr));
}

template <typename T>
inline const size_t getSerializedSize(const T& data) { 
    if constexpr (std::is_same_v<T, std::string>) 
        return data.size() + sizeof(size_t);
    else
        return sizeof(T); 
}

// BTree node maps to a BTree page which has a fixed length of 8k
template <typename TKey, typename TVal>
class BTreeNode {
public:    
    uint32_t insert(const TKey& key, const TVal& value, bool foundNode = false) {
        if (!foundNode) {
            auto findResult = find(key, true);
            assert(findResult.pid != InvalidPid);
            
            if (findResult.pid != _header._pid) {
                auto pLeafNode = reinterpret_cast<BTreeNode<TKey,TVal>*>(BufferCacheInstance.get(findResult.pid));
                return pLeafNode->insert(key, value, true);
            }
        }
        
        // if we are over the fill factor, split the node and insert into the new node
        // recursively insert the split key into parent node
        auto needSplit = false;
        auto freeSpace = MaxPageSlotSpace - (_header._items_count * sizeof(uint16_t) + (PageSize - _header._upper));
        if (1.0 - (double)freeSpace / MaxPageSlotSpace > MaxFillFactor) {
            needSplit = true;
        } else if (sizeof(key) + sizeof(value) >= freeSpace) {
            needSplit = true;
        }
        
        if (needSplit) {
            std::runtime_error("Split not implemented yet");
        } else {
            // insert into the slot by growing the _upper offset
            auto keySize = getSerializedSize(key);
            auto valueSize = getSerializedSize(value);
            auto currentPtr = reinterpret_cast<unsigned char*>((unsigned char*)this + _header._upper - (keySize + valueSize));
            serialize(key, currentPtr);
            currentPtr += keySize;
            serialize(value, currentPtr);
            _header._upper -= keySize + valueSize;
            
            // insert into the item offset array by using binary search to find the position.
            bool append = false;
            auto pos = findItemInsertPosition(key, &append);
            auto srcPtr =  reinterpret_cast<unsigned char*>(((unsigned char*)this + BTreePagerHeaderSize) + pos * sizeof(uint16_t));
            if (!append && _header._items_count - pos > 0)
                std::memcpy(srcPtr + sizeof(uint16_t), srcPtr, (_header._items_count - pos) * sizeof(uint16_t));

            uint16_t *upperPtr = reinterpret_cast<uint16_t*>(srcPtr);
            *upperPtr = _header._upper;
            _header._items_count++;
        }
        
        return _header._pid;
    }
    
    FindResult<TVal> find(const TKey& key, bool forInsert) {
        auto found = false;
        auto low = 0, mid = -1, high = _header._items_count - 1;
        while (low <= high) {
            mid = (low + high) / 2;
            auto midKey = getItemKeyValue(mid, nullptr);
            if (key > midKey)
                low = mid + 1;
            else if (key < midKey)
                high = mid - 1;
            else {
                low = mid;
                found = true;
                break;
            }
        }
        
        if (isLeaf()) {
            if (!forInsert) {
                if (found) {
                    TVal data;
                    getItemKeyValue(low, &data);
                    return FindResult<TVal>(_header._pid, data);
                } else
                    return FindResult<TVal>(InvalidPid);
            }
            else 
                return FindResult<TVal>(_header._pid);
        } else {
            auto pid = InvalidPid;
            if (low > _header._items_count - 1)
                pid = _header._right_child_pid;
            else {
                TVal data;
                getItemKeyValue(low, &data);
                // pid = (uint32_t)data; TODO: for intermediate node the data type must be dynamic cast or will fail compiling.
            }
            auto pNode = reinterpret_cast<BTreeNode<TKey,TVal>*>(BufferCacheInstance.get(pid));
            return pNode->find(key, forInsert);
        }
    }

    bool remove(const TKey& key) { 
        // TODO: 
        return true;
    }
    
    void to_string() {
        std::string nodeType;
        if ((getHeader()._info & RootNode) == RootNode)
            nodeType = "Root";
        if ((getHeader()._info & IntermediateNode) == IntermediateNode)
            nodeType = "Intermediate";
        if ((getHeader()._info & LeafNode) == LeafNode) {
            if (nodeType.size() > 0)
                nodeType.append(" Leaf");
            else
                nodeType = "Leaf";
        }
        
        if (nodeType.size() == 0)
            throw std::runtime_error("Invalid node type:");
        
        std::cout<<"=========="<<getHeader()._pid<<"==========="<<std::endl;
        std::cout<<"Type:"<<nodeType<<std::endl;
        std::cout<<"Items Count:"<<getHeader()._items_count<<std::endl;
        std::cout<<"Parent:"<<getHeader()._p_pid<<std::endl;
        std::cout<<"Left Sibling:"<<getHeader()._l_pid<<std::endl;
        std::cout<<"Right Sibling:"<<getHeader()._r_pid<<std::endl;
        std::cout<<"Slot Offset:"<<getHeader()._upper<<std::endl;
        std::cout<<"Right Child:"<<getHeader()._right_child_pid<<std::endl;
        std::cout<<"Keys:"<<std::endl;
        
        auto ptr_base = (unsigned char*)(this) + BTreePagerHeaderSize;
        for (auto i = 0; i < getHeader()._items_count; i++) {
            auto item_offset = *((uint16_t*)(ptr_base + i * sizeof(uint16_t)));
            auto ptr_current = (unsigned char*)(this) + item_offset;
            auto key = deserialize<TKey>(ptr_current);
            std::cout<<i<<"  "<<"Key:"<<key<<" "<<"Val:"<<deserialize<TVal>(ptr_current + getSerializedSize(key))<<std::endl;
        }
    }

    void set_header(const BTreePagerHeader& header) { _header = header; }

    BTreePagerHeader* getHeader() { return &_header; }

private:
    // Find the key based on item array's position
    const TKey getItemKeyValue (uint16_t index, TVal* data) {
        auto addr =  reinterpret_cast<unsigned char*>(((unsigned char*)this + BTreePagerHeaderSize) + index * sizeof(uint16_t));
        auto keyOffset = reinterpret_cast<uint16_t*>(addr);
        auto key = deserialize<TKey>(reinterpret_cast<unsigned char*>((unsigned char*)this + *keyOffset));
        if (data != nullptr) {
            auto dataOffset = *keyOffset + getSerializedSize(key);
            *data = deserialize<TVal>(reinterpret_cast<unsigned char*>((unsigned char*)this + dataOffset));
        }

        return key;
    }

    bool isLeaf() { return true; }
    
    uint16_t findItemInsertPosition(const TKey& key, bool* append) {
        auto low = 0;
        auto high = _header._items_count - 1;
        *append = false;

        if (high < 0) {
            *append = true;
            return 0;
        }
        else if (key > getItemKeyValue(high, nullptr)) {
            *append = true;
            return high + 1;
        } else if (key < getItemKeyValue(low, nullptr)) {
            return low;
        } else {
            auto mid = (low + high) / 2;
            while (low <= high) {
                mid = (low + high) / 2;
                auto midKey = getItemKeyValue(mid, nullptr);
                if (key > midKey)
                    low = mid + 1;
                else if (key < midKey)
                    high = mid - 1;
                else {
                    low = mid;
                    break;
                }
            }
            
            return low;
        }
    }
    
    BTreePagerHeader _header; // 40 bytes
};


template <typename TKey, typename TVal>
static BTreeNode<TKey, TVal> GetBTreeNode(uint32_t pid) {
    return BTreeNode<TKey, TVal>();
}

