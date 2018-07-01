#include <assert.h>

#include <sstream>
#include <iomanip>

#include "ModuleMemory.h"

ModuleMemory::ModuleMemory(const native_data& nd)
    : nd(nd), ptr_map(), ptr_read_count(0), ptr_invalid_count(0)
{
    assert(nd.read_fn);
}

ModuleMemory::~ModuleMemory()
{
}

unsigned long ModuleMemory::getPtr(const std::string& name)
{
    bool valid = ptrExists(name) && ptrValid(name);
    if (!valid)
        return 0;
    return ptr_map[name].ptr;
}

unsigned long ModuleMemory::getPtr(const std::string& name, unsigned long *dest_ptr)
{
    assert(dest_ptr);
    unsigned long ret = getPtr(name);
    *dest_ptr = ret;
    return ret;
}

unsigned long ModuleMemory::getPtr(const std::string& name, unsigned long base,
                                   unsigned long offset)
{
    target_ptr out = {0};
    out.base = base;
    out.offset = offset;
    out.valid = nd.read_fn(&nd, base + offset, &out.ptr, sizeof(out.ptr));
    ptr_map[name] = out;
    if (out.valid)
    {
        ++ptr_read_count;
        return out.ptr;
    }
    else
    {
        return 0;
    }
}

unsigned long ModuleMemory::recheckPtr(const std::string& name)
{
    if (!ptrExists(name))
        return 0;
    target_ptr old = ptr_map[name];
    unsigned long new_ptr = getPtr(name, old.base, old.offset);
    if (!new_ptr)
    {
        ptr_map[name].valid = false;
    }
    return new_ptr;
}

void ModuleMemory::revalidateAllPtr()
{
}

bool ModuleMemory::getData(const std::string& name, void *buffer, unsigned long siz)
{
    assert(buffer);
    if (!getPtr(name))
        return false;
    if (!nd.read_fn(&nd, ptr_map[name].ptr, buffer, siz))
    {
        ptr_map[name].valid = false;
    }
    return ptr_map[name].valid;
}

bool ModuleMemory::ptrSetDependency(const std::string& name, const std::string& dependency)
{
    if (!getPtr(name) || !getPtr(dependency))
        return false;
    ptr_map[name].dependency = dependency;
    ptr_map[dependency].children.insert(name);
    return true;
}

std::string ModuleMemory::toString()
{
    std::stringstream out;
    for (auto& ptr : ptr_map)
    {
        out << std::setw(16) << ptr.first << "[ "
            << std::setw(8) << std::hex << ptr.second.base << " + "
            << std::setw(8) << std::hex << ptr.second.offset << " = "
            << std::setw(8) << std::hex << ptr.second.ptr << " , "
            << std::setw(5) << (ptr.second.valid ? "TRUE" : "FALSE") << " , "
            << std::setw(16) << (ptr.second.dependency.c_str() ? ptr.second.dependency.c_str() : "")
            << " ]";
        for (auto& child : ptr.second.children)
        {
            out << ", " << child;
        }
        out << std::endl;
    }
    return out.str();
}

std::string ModuleMemory::toStringStats()
{
    std::stringstream out;
    out << "PtrReadCount: " << ptr_read_count << " , "
        << "PtrInvalidCount: " << ptr_invalid_count;
    return out.str();
}
