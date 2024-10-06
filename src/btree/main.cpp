#include <iostream>
#include <random>
#include <utility>
#include <vector>
#include "btree.h"
#include "buffercache.h"

BufferCache BufferCacheInstance(100);

const size_t MaxStrKeyLength = 30;
const size_t MaxStrValLength = 70;
constexpr int32_t Min_int32_value = std::numeric_limits<int32_t>::min();
constexpr int32_t Max_int32_value = std::numeric_limits<int32_t>::max();

// Define the Unicode range (e.g., Basic Latin, Latin-1 Supplement, Cyrillic, etc.)
const uint32_t Min_utf8_range = 0x0020;  // Space character (start of printable ASCII)
const uint32_t Max_utf8_range = 0xFFFF;  // Maximum for Basic Multilingual Plane (BMP)

// Function to generate a random Unicode code point
uint32_t generate_random_unicode_code_point() {
    // Create random number generator
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<uint32_t> distribution(Min_utf8_range, Max_utf8_range);
    return distribution(generator);
}

static std::string code_point_to_utf8(uint32_t code_point) {
    std::string utf8_string;
    
    if (code_point <= 0x7F) {
        // 1-byte UTF-8
        utf8_string += static_cast<char>(code_point);
    } else if (code_point <= 0x7FF) {
        // 2-byte UTF-8
        utf8_string += static_cast<char>((code_point >> 6) | 0xC0);
        utf8_string += static_cast<char>((code_point & 0x3F) | 0x80);
    } else if (code_point <= 0xFFFF) {
        // 3-byte UTF-8
        utf8_string += static_cast<char>((code_point >> 12) | 0xE0);
        utf8_string += static_cast<char>(((code_point >> 6) & 0x3F) | 0x80);
        utf8_string += static_cast<char>((code_point & 0x3F) | 0x80);
    } else if (code_point <= 0x10FFFF) {
        // 4-byte UTF-8
        utf8_string += static_cast<char>((code_point >> 18) | 0xF0);
        utf8_string += static_cast<char>(((code_point >> 12) & 0x3F) | 0x80);
        utf8_string += static_cast<char>(((code_point >> 6) & 0x3F) | 0x80);
        utf8_string += static_cast<char>((code_point & 0x3F) | 0x80);
    }
    
    return utf8_string;
}

// Function to generate a random Unicode string of specified length
static std::string generateRandomUnicodeString(const size_t maxLength) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int32_t> int32_distribution(0, maxLength);
    auto length = int32_distribution(generator);
    std::string result;
    for (size_t i = 0; i < length; ++i) {
        uint32_t code_point = generate_random_unicode_code_point();
        result += code_point_to_utf8(code_point);
    }

    return result;
}

template <typename TKey, typename TVal>
static void generateRandomTestData(const u_int32_t size, std::vector<TKey>& keys, std::vector<TVal>& values) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int32_t> int32_distribution(Min_int32_value, Max_int32_value);

    if constexpr (std::is_same_v<TKey, int32_t>)
        for (u_int32_t i = 0; i < size; i++)
            keys.push_back(int32_distribution(generator));
    else if constexpr (std::is_same_v<TKey, std::string>)
        for (u_int32_t i = 0; i < size; i++)
            keys.push_back(generateRandomUnicodeString(MaxStrKeyLength));

    if constexpr (std::is_same_v<TVal, int32_t>)
        for (u_int32_t i = 0; i < size; i++)
            values.push_back(int32_distribution(generator));
    else if constexpr (std::is_same_v<TVal, std::string>)
        for (u_int32_t i = 0; i < size; i++)
            values.push_back(generateRandomUnicodeString(MaxStrValLength));
}

static void testSerialization() {
    unsigned char page[1000];
    auto pos = 0;
    auto str1 = "This is a test string";
    auto str2 = "test string number two";
    double d1 = 3.141592653589793;
    int i1 = 123456789;
    
    serialize<std::string>(str1, page);
    pos += getSerializedSize<std::string>(str1);
    
    serialize(d1, page+pos);
    pos += getSerializedSize<double>(d1);
    
    serialize(i1, page+pos);
    pos += getSerializedSize<int>(i1);
    
    serialize<std::string>(str2, page+pos);
    pos += getSerializedSize<std::string>(str2);
    
    pos = 0;
    auto str1d = deserialize<std::string>(page + pos);
    assert(str1 == str1d);
    pos += getSerializedSize<std::string>(str1d);
    
    auto d1d = deserialize<double>(page + pos);
    assert(d1 = d1d);
    pos += getSerializedSize<double>(d1d);
    
    auto i1d = deserialize<int>(page+pos);
    assert(i1 = i1d);
    pos += getSerializedSize<int>(i1d);
    
    auto str2d = deserialize<std::string>(page + pos);
    assert(str2 == str2d);

    std::cout<<"testSerialization succeeded"<<"\n";
}

void testOneNodeOnly() {
    unsigned char* page;
    auto pid = BufferCacheInstance.initNextFreePage(&page);
    auto btreeNode = reinterpret_cast<BTreeNode<int32_t,std::string>*>(page);
    SetNodeType(&(btreeNode->getHeader()->_info), RootNode | LeafNode);
    btreeNode->getHeader()->_upper = PageSize;
    btreeNode->getHeader()->_padding = PageHeaderPadding;
    assert(IsRootNode(btreeNode->getHeader()->_info) && IsLeafNode(btreeNode->getHeader()->_info)
        && !IsIntermiediateNode(btreeNode->getHeader()->_info));

    size_t size = 10;
    std::vector<int32_t> keys;
    std::vector<std::string> values;
    generateRandomTestData<int32_t, std::string>(size, keys, values);

    for (auto i = 0; i < size; i++)
        btreeNode->insert(keys[i], values[i]);

    for (auto i = 0; i < size; i++) {
        auto result = btreeNode->find(keys[i], false);
        assert(result.pid == 0 && result.data == values[i]);
    }

    std::cout<<"testRootOnly succeeded"<<"\n";
}

int main(int argc, const char * argv[]) {
    testSerialization();
    testOneNodeOnly();
}

