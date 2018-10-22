#include <assert.h>

#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "ModuleMemory.h"

ModuleMemory::ModuleMemory(const native_data& nd)
    : nd(nd), ptr_map(), ptr_read_count(0), ptr_invalid_count(0)
{
    assert(nd.read_fn && nd.iterate_fn);
}

ModuleMemory::~ModuleMemory()
{
}

unsigned long ModuleMemory::scanProcMem(const std::string& name, const std::string& pattern, long offset, bool retPtr)
{
    unsigned long ret = scanPattern((unsigned long) nd.proc.modbase, nd.proc.modsize, pattern);
    if (ret != (unsigned long) -1)
    {
        return (retPtr ? getPtr(name, (unsigned long) nd.proc.modbase, offset + ret) :
                ((unsigned long) nd.proc.modbase + offset + ret));
    }
    return (unsigned long) -1;
}

unsigned long ModuleMemory::scanMappedMem(const std::string& name, const std::string& pattern, long offset, bool retPtr)
{
    unsigned long addr = 0;
    unsigned long size = 0;
    unsigned long ret;
    do
    {
        addr += size;
        iterate_mem(&nd, &addr, &size);
        ret = scanPattern(addr, size, pattern);
        if (ret != (unsigned long) -1)
            return (retPtr ? getPtr(name, addr, offset + ret) : (addr + offset + ret));
    }
    while (size);
    return (unsigned long) -1;
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
                                   long offset)
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

unsigned long ModuleMemory::scanPattern(unsigned long addr, unsigned long size, const std::string& pattern)
{
    unsigned char *buffer;
    std::string new_pattern(pattern);
    std::vector<unsigned char> binary_pattern;
    unsigned long i, match_required, match_found;

    if (!size)
        return (unsigned long) -1;

    buffer = new unsigned char [size];
    if (!nd.read_fn(&nd, addr, buffer, size))
        goto error;

    new_pattern.erase(std::remove_if(new_pattern.begin(), new_pattern.end(), isspace), new_pattern.end());

    if (new_pattern.length() % 2)
        goto error;

    match_required = 0;
    for (i = 0; i < new_pattern.length(); i += 2)
    {
        char *endptr;
        std::string byteString = new_pattern.substr(i, 2);

        if (byteString[0] == '?' && byteString[1] == '?')
        {
            binary_pattern.push_back(0x00);
            continue;
        }

        unsigned char byte = (unsigned char) strtoul(byteString.c_str(), &endptr, 16);

        if (endptr == byteString.c_str() || *endptr != 0x00)
            goto error;

        match_required++;
        binary_pattern.push_back(byte);
    }

    match_found = 0;
    for (i = 0; i < size; ++i)
    {
        for (size_t j = 0; i < size && j < binary_pattern.size(); ++j)
        {
            if (new_pattern[j*2] == '?' && new_pattern[j*2+1] == '?')
            {
                i++;
                continue;
            }

            if (buffer[i] == binary_pattern[j])
            {
                i++;
                match_found++;
            }
            else
            {
                match_found = 0;
                break;
            }

            if (match_found == match_required)
            {
                goto success;
            }
        }
    }

success:
    delete buffer;
    return (match_found == match_required ? i - binary_pattern.size() : (unsigned long) -1);
error:
    delete buffer;
    return (unsigned long) -1;
}
