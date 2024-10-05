#pragma once

#include <format>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>

// Page size in bytes
constexpr uint32_t PageSize = 8 * 1024;

class BufferCache {
public:
    BufferCache(uint32_t pages) : _pages(pages), _next_free_page(0) {
        _ptr = new unsigned char[_pages * PageSize];
    }
    
    uint32_t init_next_free_page(unsigned char** page) {
        std::unique_lock<std::shared_mutex> lock(_rwMutex);
        auto pid = 0;
        if (_freeList.size() > 0) {
            pid = _freeList.back();
            _freeList.pop_back();
        } else {
            pid = _next_free_page++;
        }
        
        *page = _ptr +  pid * PageSize;
        return pid;
    }
    
    void free(uint32_t pid) {
        std::unique_lock<std::shared_mutex> lock(_rwMutex);
        _freeList.push_back(pid);
    }
    
    unsigned char* get(uint32_t pid) {
        return _ptr + pid * PageSize;
    }
    
private:
    uint32_t _pages;
    uint32_t _next_free_page;
    std::vector<uint32_t> _freeList;
    unsigned char* _ptr;
    std::shared_mutex _rwMutex;
};
