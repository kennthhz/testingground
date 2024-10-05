//
//  btree.h
//  BTree
//
//  Created by Hao Zhang on 1/12/24.
//

#ifndef btree_h
#define btree_h
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <format>

#include "buffercache.h"

const uint32_t InvalidPid = UINT32_MAX;
const uint16_t RootNode = 0x0;
const uint16_t IntermediateNode = 0x1;
const uint16_t LeafNode = 0x3;
const double MaxFillFactor = 0.9;

constexpr bool IsRootNode(uint16_t info) {
    return (info & RootNode) == RootNode;
}

constexpr bool IsIntermiediateNode(uint16_t info) {
    return (info & IntermediateNode) == IntermediateNode;
}

constexpr bool IsLeafNode(uint16_t info) {
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
    uint16_t _upper = PageSize;
    
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
    static_assert(std::is_same<T, int>::value ||
                  std::is_same<T, float>::value || std::is_same<T, double>::value ||
                  std::is_same<T, long double>::value || std::is_same<T, bool>::value ||
                  std::is_same<T, long>::value || std::is_same<T, long long>::value,
                  "Type is not supported");
    std::memcpy(addr, &data, sizeof(T));
}

template <>
inline void serialize<std::string>(const std::string& str, unsigned char* addr) {
    auto str_size = str.size();
    std::memcpy(addr, &str_size, sizeof(size_t));
    std::memcpy(addr + sizeof(size_t), str.data(), str_size);
}

template <typename T>
inline const T deserialize(unsigned char* addr) {
    return *(reinterpret_cast<T*>(addr));
}

template <>
inline const std::string deserialize<std::string>(unsigned char* addr) {
    return std::string(reinterpret_cast<const char*>(addr + sizeof(size_t)), *addr);
}

template <typename T>
inline const size_t get_serialized_size(const T& data) {
        return sizeof(T);
}

template <>
inline const size_t get_serialized_size(const std::string& str) {
    return str.size() + sizeof(size_t);
}

// BTree node maps to a BTree page which has a fixed length of 8k
template <typename TKey, typename TVal>
class BTreeNode {
public:    
    uint32_t insert(const TKey& key, const TVal& value, bool foundNode);
    bool find(const TKey& key, TVal** value);
    bool remove(const TKey& key);
    void to_string();
    void set_header(const BTreePagerHeader& header) { _header = header; }
    BTreePagerHeader getHeader() { return _header; }
private:
    FindResult<TVal> find(const TKey& key, bool forInsert);
    
    // Find the key based on item array's position
    const TKey getItemKey (uint16_t index) {
        auto addr =  reinterpret_cast<unsigned char*>(((unsigned char*)this + BTreePagerHeaderSize) + index * sizeof(uint16_t));
        if (std::is_same<TKey, int32_t>::value || std::is_same<TKey, uint32_t>::value)
            return *(reinterpret_cast<TKey*>(addr));
        else if (std::is_same<TKey, std::string>::value)
            return deserialize<std::string>(addr);
        else
            throw std::runtime_error("Not supported TKey type:" + std::string(typeid(TKey).name()));
    }
    
    TVal getItemValue(uint16_t index) {
        // TODO
        return TVal();
    }
    
    bool isLeaf() { return true; }
    
    uint16_t findItemInsertPosition(const TKey& key, bool* append);
    
    BTreePagerHeader _header; // 40 bytes
};


/*
static void setUInt16AtAddress(unsigned char* address, uint16_t value) {
    // Copy the bytes of uint16_t value to the memory address
    address[0] = static_cast<unsigned char>(value & 0xFF);       // Low byte
    address[1] = static_cast<unsigned char>((value >> 8) & 0xFF); // High byte
}*/

template <typename TKey, typename TVal>
static BTreeNode<TKey, TVal> GetBTreeNode(uint32_t pid) {
    return BTreeNode<TKey, TVal>();
}

/*
template <typename TKey, typename TVal>
static BTreeNode<TKey, TVal> deserializeBTreeNodeBuffer(const unsigned char* buffer) {
    reinterpret_cast<BTreeNode<TKey, TVal>*>(buffer);
}*/
#endif /* btree_h */
