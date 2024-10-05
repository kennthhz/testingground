//
//  btree.cpp
//  BTree
//
//  Created by Hao Zhang on 1/14/24.
//

#include <stdio.h>
#include "btree.h"

extern BufferCache BufferCache;

template <typename TKey, typename TVal>
uint32_t BTreeNode<TKey, TVal>::insert(const TKey& key, const TVal& value, bool foundNode) {
    if (!foundNode) {
        auto findResult = find(key, true);
        assert(findResult.pid != InvalidPid);
        
        if (findResult.pid != _header._pid) {
            auto pLeafNode = reinterpret_cast<BTreeNode<TKey,TVal>*>(BufferCache.get(findResult.pid));
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
        auto keySize = getSerializedKeySize(key);
        _header._upper -= keySize + getSerializedValueSize(value);
        auto p = reinterpret_cast<unsigned char*>((unsigned char*)this + _header._upper);
        serializeKey(key, p);
        p += keySize;
        serializeValue(value, p);
        
        // insert into the item offset array by using binary search to find the position.
        auto pos = findItemInsertPosition(key);
        auto src =  reinterpret_cast<unsigned char*>(((unsigned char*)this + BTreePagerHeaderSize) + pos * sizeof(uint16_t));
        if (_header._items_count - pos > 0)
            std::memcpy(src + sizeof(uint16_t), src, _header._items_count - pos);
        setUInt16AtAddress(src, _header._upper);
        _header._items_count++;
    }
    
    return _header._pid;
}

template <typename TKey, typename TVal>
FindResult<TVal> BTreeNode<TKey, TVal>::find(const TKey& key, bool forInsert) {
    auto found = false;
    auto low = 0, mid = -1, high = _header._items_count - 1;
    while (low <= high) {
        mid = (low + high) / 2;
        auto midKey = getItemKey(mid);
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
        if (!forInsert)
            return found ? FindResult<TVal>(_header._pid, getItemValue(low)) : FindResult<TVal>(InvalidPid);
        else
            return FindResult<TVal>(_header._pid);
    } else {
        auto pid = low > _header._items_count - 1 ? _header._right_child_pid :
            (uint32_t)getItemValue(low);
        auto pNode = reinterpret_cast<BTreeNode<TKey,TVal>*>(BufferCache.get(pid));
        return pNode->find(key, forInsert);
    }
}

template <typename TKey, typename TVal>
uint16_t BTreeNode<TKey, TVal>::findItemInsertPosition(const TKey& key, bool* append) {
    auto low = 0;
    auto high = _header._items_count - 1;
    *append = false;

    if (key > getItemKey(high)) {
        *append = true;
        return high + 1;
    } else if (key < getItemKey(low)) {
        return low;
    } else {
        auto mid = (low + high) / 2;
        while (low <= high) {
            mid = (low + high) / 2;
            auto midKey = getItemKey(mid);
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

template <typename TKey, typename TVal>
void BTreeNode<TKey, TVal>::to_string() {
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
        std::cout<<i<<"  "<<"Key:"<<key<<" "<<"Val:"<<deserialize<TVal>(ptr_current + get_serialized_size(key))<<std::endl;
    }
}
