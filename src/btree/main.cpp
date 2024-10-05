//
//  main.cpp
//  BTree
//
//  Created by Hao Zhang on 1/12/24.
//

#include <iostream>
#include "btree.h"


BufferCache BufferCache(100);

static void test_serialization() {
    unsigned char page[1000];
    auto pos = 0;
    auto str1 = "This is a test string";
    auto str2 = "test string number two";
    double d1 = 3.141592653589793;
    int i1 = 123456789;
    
    serialize<std::string>(str1, page);
    pos += get_serialized_size<std::string>(str1);
    
    serialize(d1, page+pos);
    pos += get_serialized_size<double>(d1);
    
    serialize(i1, page+pos);
    pos += get_serialized_size<int>(i1);
    
    serialize<std::string>(str2, page+pos);
    pos += get_serialized_size<std::string>(str2);
    
    pos = 0;
    auto str1d = deserialize<std::string>(page + pos);
    assert(str1 == str1d);
    pos += get_serialized_size<std::string>(str1d);
    
    auto d1d = deserialize<double>(page + pos);
    assert(d1 = d1d);
    pos += get_serialized_size<double>(d1d);
    
    auto i1d = deserialize<int>(page+pos);
    assert(i1 = i1d);
    pos += get_serialized_size<int>(i1d);
    
    auto str2d = deserialize<std::string>(page + pos);
    assert(str2 == str2d);
}


void test_root_node() {
    unsigned char* page;
    auto rootPid = BufferCache.init_next_free_page(&page);
    assert(rootPid == 0);
    //auto node = reinterpret_cast<BTreeNode<int,int>*>(page);
    auto node = new BTreeNode<int,int>();
    node->insert(1,1, false);
    // node->insert(1,1);
    //node->insert(2,2);
    //node->insert(3,3);
}

int main(int argc, const char * argv[]) {
    //test_serialization();
    test_root_node();
}

/*
template <typename TKey, typename TVal>
void createBTPage(unsigned char* ptr, 
                  uint32_t pid,
                  uint32_t p_pid,
                  uint32_t l_pid,
                  uint32_t r_pid,
                  const std::vector<std::tuple<TKey, TVal>>* kvPairs,
                  uint16_t nodeType) {
    
    BTreePagerHeader header;
    header._info = nodeType;
    header._version = 1;
    header._fillFactor = 80;
    header._items_count = kvPairs->size();
    header._crc = 0xF0F0F0F0;
    header._pid = pid;
    header._p_pid = p_pid;
    header._l_pid = l_pid;
    header._r_pid = r_pid;
    header._upper = PageSize;
    header._padding = 0xFFFF;
    
    for (auto& [index, pair] : *kvPairs) {
        if (std::get<0>(pair) != UINT32_MAX) {
            auto keySize = getSerializedKeySize(std::get<0>(pair));
            auto valueSize = getSerializedValue(std::get<1>(pair));
            auto keyValueSize = (keySize + valueSize);
            auto itemOffSet = PageSize - (index+1) * keyValueSize;
            serializeKey(std::get<0>(pair), static_cast<unsigned char*>(ptr + itemOffSet));
            serializeValue(std::get<1>(pair), static_cast<unsigned char*>(ptr + itemOffSet + keySize));
            header._upper -= keyValueSize;
            
            auto indexPointer = static_cast<void*>(ptr + sizeof(BTreePagerHeader) +  index * sizeof(uint16_t));
            *reinterpret_cast<uint16_t*>(indexPointer) = header._upper ;
        } else {
            assert(nodeType != LeafNode);
            header._right_child_pid = std::get<1>(pair);
        }
    }
}



void test(){
    auto pgNumber = 0;
    auto ptr = new unsigned char[10 * PageSize];
    auto rootKvs = std::make_shared<std::vector<std::tuple<uint32_t, uint32_t>>>();
    rootKvs->emplace_back(100, 0);
    rootKvs->emplace_back(UINT32_MAX, 1);
    createBTPage(ptr + pgNumber * PageSize, pgNumber, pgNumber, UINT32_MAX, UINT32_MAX, rootKvs.get(), RootNode);
    pgNumber++;
    
    auto interKvs1 = std::make_shared<std::vector<std::tuple<uint32_t, uint32_t>>>();
    interKvs1->emplace_back(30, 3);
    interKvs1->emplace_back(60, 4);
    interKvs1->emplace_back(UINT32_MAX, 5);
    createBTPage(ptr + pgNumber * PageSize, pgNumber, 0, UINT32_MAX, UINT32_MAX, interKvs1.get(), IntermediateNode);
    pgNumber++;
    
    auto interKvs2 = std::make_shared<std::vector<std::tuple<uint32_t, uint32_t>>>();
    interKvs2->emplace_back(200, 6);
    interKvs2->emplace_back(400, 7);
    interKvs2->emplace_back(UINT32_MAX, 8);
    createBTPage(ptr + pgNumber * PageSize, pgNumber, 0, UINT32_MAX,  UINT32_MAX, interKvs2.get(), IntermediateNode);
    pgNumber++;
    
    auto leafKvs3 = std::make_shared<std::vector<std::tuple<uint32_t, std::string>>>();
    leafKvs3->emplace_back(10, "10AAA");
    leafKvs3->emplace_back(20, "20BBB");
    leafKvs3->emplace_back(30, "30CCC");
    createBTPage(ptr + pgNumber * PageSize, pgNumber, 1, UINT32_MAX, 4, leafKvs3.get(), LeafNode);
    pgNumber++;
    
    auto leafKvs4 = std::make_shared<std::vector<std::tuple<uint32_t, std::string>>>();
    leafKvs4->emplace_back(41, "41DDD");
    leafKvs4->emplace_back(45, "45EEE");
    leafKvs4->emplace_back(60, "60FFF");
    createBTPage(ptr + pgNumber * PageSize, pgNumber, 1, 3, 5, leafKvs4.get(), LeafNode);
    pgNumber++;
    
    auto leafKvs5 = std::make_shared<std::vector<std::tuple<uint32_t, std::string>>>();
    leafKvs5->emplace_back(70, "70adfasd");
    leafKvs5->emplace_back(90, "90EghsfEE");
    leafKvs5->emplace_back(100, "100FFghdgfhF");
    createBTPage(ptr + pgNumber * PageSize, pgNumber, 1, 4, 6, leafKvs5.get(), LeafNode);
    pgNumber++;
    
    auto leafKvs6 = std::make_shared<std::vector<std::tuple<uint32_t, std::string>>>();
    leafKvs6->emplace_back(130, "130adfasd");
    leafKvs6->emplace_back(145, "145EghsfEE");
    leafKvs6->emplace_back(200, "200FFghdgfhF");
    createBTPage(ptr + pgNumber * PageSize, pgNumber, 2, 5, 7, leafKvs6.get(), LeafNode);
    pgNumber++;
    
    auto leafKvs7 = std::make_shared<std::vector<std::tuple<uint32_t, std::string>>>();
    leafKvs7->emplace_back(256, "256adfasd");
    leafKvs7->emplace_back(345, "345EghsfEE");
    leafKvs7->emplace_back(400, "400FFghdgfhF");
    createBTPage(ptr + pgNumber * PageSize, pgNumber, 2, 6, 8, leafKvs7.get(), LeafNode);
    pgNumber++;
    
    auto leafKvs8 = std::make_shared<std::vector<std::tuple<uint32_t, std::string>>>();
    leafKvs8->emplace_back(1245, "1245adfasd");
    leafKvs8->emplace_back(5001, "5001adfasd");
    createBTPage(ptr + pgNumber * PageSize, pgNumber, 2, 7, UINT32_MAX, leafKvs8.get(), LeafNode);
}

int main(int argc, const char * argv[]) {
    
    BTreePagerHeader header;
    header._info = 0xFFFF;
    header._version = 1;
    header._fillFactor = 80;
    header._items_count = 0;
    header._upper = 1024;
    header._p_pid = 1;
    header._l_pid = UINT32_MAX;
    header._r_pid = 2;
    header._crc = 0xF0F0F0F0;
    header._pid = 2;
    header._right_child_pid = 3;
    header._padding = 0xFFFF;
    
    std::shared_ptr<BTreeNode<int32_t, int32_t>> node = 
        std::make_shared<BTreeNode<int32_t, int32_t>>();
    node->set_header(header);
    
    unsigned char *byteArray = new unsigned char[4096];
    
    std::cout<<"Size of BTreePagerHeader is "<<sizeof(BTreePagerHeader)<<"\n";
    std::memcpy(byteArray, &header, sizeof(BTreePagerHeader));
    std::memset(byteArray + sizeof(BTreePagerHeader), 0xFF, 4096 - sizeof(BTreePagerHeader));
    
    BTreeNode<int32_t, int32_t>* p = reinterpret_cast<BTreeNode<int32_t, int32_t>*>(byteArray);
    std::cout<<"Fill factor:"<<p->getHeader()._fillFactor<<"\n";
}*/

