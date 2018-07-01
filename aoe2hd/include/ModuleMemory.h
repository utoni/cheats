#ifndef PROCESSMEMORY_H
#define PROCESSMEMORY_H

#include <map>
#include <set>
#include <string>

extern "C" {
#include "native.h"
}


typedef struct target_ptr
{
    unsigned long base;
    unsigned long offset;
    unsigned long ptr;
    bool valid;
    std::string dependency;
    std::set<std::string> children;
} target_ptr;

class ModuleMemory
{
public:
    ModuleMemory(const native_data& nd);
    virtual ~ModuleMemory();
    unsigned long getPtr(const std::string& name);
    unsigned long getPtr(const std::string& name, unsigned long *dest_ptr);
    unsigned long getPtr(const std::string& name, unsigned long base, unsigned long offset);
    unsigned long recheckPtr(const std::string& name);
    void revalidateAllPtr();
    bool ptrSetDependency(const std::string& name, const std::string& dependency);
    bool getData(const std::string& name, void *buffer, unsigned long siz);
    std::string toString();
    std::string toStringStats();
private:
    const native_data& nd;
    std::map<std::string, target_ptr> ptr_map;
    unsigned long ptr_read_count;
    unsigned long ptr_invalid_count;
    bool ptrExists(const std::string& name)
    {
        return ptr_map.find(name) != ptr_map.end();
    }
    bool ptrValid(const std::string& name)
    {
        if (ptrExists(name) && ptr_map[name].valid)
        {
            return true;
        }
        else ++ptr_invalid_count;
        return false;
    }
};

#endif // PROCESSMEMORY_H
