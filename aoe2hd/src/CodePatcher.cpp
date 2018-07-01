#include <assert.h>

#include <sstream>
#include <iomanip>

#include "CodePatcher.h"
#include "utils.h"
#include "native.h"


CodePatcher::CodePatcher(const native_data& nd)
    : nd(nd)
{
    assert(nd.read_fn && nd.write_fn && nd.suspend_fn);
}

CodePatcher::~CodePatcher()
{
}

bool CodePatcher::addPatch(const std::string& name,
                           unsigned long addr,
                           const std::vector<unsigned char>& old_code,
                           const std::vector<unsigned char>& new_code,
                           long new_offset)
{
    assert(addr);
    assert(old_code.size() == new_code.size());
    if (codePatchExists(name))
        return false;
    code_patch patch = {0};
    patch.addr = nd.proc.modbase + addr;
    patch.old_code = old_code;
    patch.new_code = new_code;
    patch.new_offset = new_offset;
    patch_map[name] = patch;
    return true;
}

void CodePatcher::setPatchSuspend(const std::string& name, long doSuspend)
{
    if (codePatchExists(name))
        patch_map[name].suspend = doSuspend;
}

bool CodePatcher::doPatch(const std::string& name, int doUnPatch)
{
    if (!codePatchExists(name))
        return false;
    auto& patch = patch_map[name];
    if (codeCmp(patch.addr + patch.new_offset, patch.new_code))
        return false;
    if (patch.suspend)
        nd.suspend_fn(&nd, 0);
    bool ret = false;
    if (doUnPatch)
    {
        ret = nd.write_fn(&nd, patch.addr, &patch.old_code[0], patch.old_code.size());
    }
    else
    {
        ret = nd.write_fn(&nd, patch.addr, &patch.new_code[0], patch.new_code.size());
    }
    if (patch.suspend)
        nd.suspend_fn(&nd, 1);
    return ret;
}

bool CodePatcher::autoPatch(const std::string& name)
{
    if (!doPatch(name, 0))
    {
        if (!doPatch(name, 1))
            return false;
        return doPatch(name, 0);
    }
    return true;
}

std::string CodePatcher::toString()
{
    std::stringstream out;
    for (auto& patch : patch_map)
    {
        out << std::setw(16) << patch.first << "[ "
            << std::setw(8) << std::hex << patch.second.addr << " , "
            << std::setw(8) << std::hex << patch.second.new_offset
            << " ]" << std::endl
            << std::setw(23) << "Old: "
            << utils::convertBinToHexstr(patch.second.old_code) << std::endl
            << std::setw(23) << "New: "
            << utils::convertBinToHexstr(patch.second.new_code) << std::endl;
    }
    return out.str();
}

bool CodePatcher::codeCmp(unsigned long addr, std::vector<unsigned char> code)
{
    if (code.size() == 0)
        return true;
    unsigned char buf[code.size()] = {0};
    if (!nd.read_fn(&nd, addr, &buf[0], code.size()))
        return false;
    /* TODO: replace with memcmp? */
    for (unsigned i = 0; i < code.size(); ++i)
    {
        if (buf[i] != code[i])
            return false;
    }
    return true;
}
