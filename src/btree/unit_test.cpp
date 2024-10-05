//
//  unit_test.cpp
//  BTree
//
//  Created by Hao Zhang on 3/3/24.
//

#include "btree.h"
#include "unit_test.h"

static void test_serialization() {
    unsigned char page[1000];
    auto pos = 0;
    auto str1 = "This is a test string";
    auto str2 = "test string number two";
    double d1 = 3.141592653589793;
    int i1 = 123456789;
    
    serialize<std::string>(str1, page);
    pos += get_serialized_size<std::string>(str1);
    
    serialize<double>(d1, page+pos);
    pos += get_serialized_size<double>(d1);
    
    serialize<int>(i1, page+pos);
    pos += get_serialized_size<int>(i1);
    
    serialize<std::string>(str2, page+pos);
    pos += get_serialized_size<std::string>(str2);
    
    /*
    pos = 0;
    auto str1d = deserialize<std::string>(page + pos);
    assert(str1 == str1d);
    pos += get_serialized_size<std::string>(str1d);
    
    auto d1d = deserialize<double>(page + pos);
    assert(d1 = d1d);
    pos += get_serialized_size<double>(d1d);
    
    auto i1d = deserialize<double>(page+pos);
    assert(i1 = i1d);
    pos += get_serialized_size<int>(i1d);
    
    auto str2d = deserialize<std::string>(page + pos);
    assert(str2 == str2d);*/
}
