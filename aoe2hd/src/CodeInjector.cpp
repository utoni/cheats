#include <assert.h>

#include <sstream>
#include <iomanip>
#include <algorithm>

#include "CodeInjector.h"

CodeInjector::CodeInjector(const native_data& nd)
    : nd(nd)
{
    assert(nd.alloc_fn && nd.write_fn);
}

CodeInjector::~CodeInjector()
{
}

bool CodeInjector::allocCodeSegment(const std::string& name, unsigned long siz)
{
    if (codeSegExists(name))
        return false;
    code_seg seg = {0};
    seg.siz = siz;
    seg.addr = nd.alloc_fn(&nd, siz);
    if (!seg.addr)
        return false;
    code_map[name] = seg;
    return true;
}

bool CodeInjector::addCode(const std::string& name, const std::string& code_name,
                           const std::vector<unsigned char>& code)
{
    assert(code.size());
    if (!codeSegExists(name)
            || codeBinExists(name, code_name))
        return false;
    auto cave = findCodeCave(name, code.size());
    if (!cave)
        return false;
    code_bin bin = {0};
    bin.addr = cave;
    bin.siz = code.size();
    if (!nd.write_fn(&nd, bin.addr, &code[0], bin.siz))
        return false;
    code_map[name].children[code_name] = bin;
    return true;
}

bool CodeInjector::addCode(const std::string& name, const std::string& code_name,
             const std::string& code)
{
    return false;
}

bool CodeInjector::addCode(const std::string& name, const std::string& code_name,
                           unsigned long siz)
{
    assert(siz);
    std::vector<unsigned char> code(siz, 0x90);
    return addCode(name, code_name, code);
}

bool CodeInjector::setCode(const std::string& name, const std::string& code_name,
                           const std::vector<unsigned char>& code,
                           unsigned long offset)
{
    assert(code.size());
    if (!codeSegExists(name)
            || !codeBinExists(name, code_name))
        return false;
    code_bin bin = {0};
    if (!getCodeBin(name, code_name, &bin))
        return false;
    assert(bin.addr && bin.siz);
    if (bin.addr + offset + code.size() > bin.addr + bin.siz)
        return false;
    return nd.write_fn(&nd, bin.addr + offset, &code[0], code.size());
}

bool CodeInjector::delCode(const std::string& name, const std::string& code_name)
{
    if (!codeBinExists(name, code_name))
        return false;
    code_map[name].children[code_name];
    return code_map[name].children.erase(code_name) > 0;
}
unsigned long CodeInjector::getCodeAddr(const std::string& name, const std::string& code_name)
{
    assert(codeBinExists(name, code_name));
    assert(code_map[name].children[code_name].addr);
    return code_map[name].children[code_name].addr;
}

bool CodeInjector::getCodeSeg(const std::string& name, code_seg *seg)
{
    assert(seg);
    if (!codeSegExists(name))
        return false;
    *seg = code_map[name];
    return true;
}

bool CodeInjector::getCodeBin(const std::string& name, const std::string& code_name, code_bin *bin)
{
    assert(bin);
    if (!codeBinExists(name, code_name))
        return false;
    *bin = code_map[name].children[code_name];
    return true;
}

std::string CodeInjector::toString()
{
    std::stringstream out;
    for (auto& code : code_map)
    {
        out << std::setw(16) << code.first << "[ "
            << std::setw(8) << std::hex << code.second.addr << " , "
            << std::setw(8) << std::hex << code.second.siz
            << " ]" << std::endl;
        for (auto& child : code.second.children)
        {
            out << std::setw(18) << child.first << "[ "
                << std::setw(8) << std::hex << child.second.addr << " , "
                << std::setw(6) << std::hex << child.second.siz
                << " ]" << std::endl;
        }
    }
    return out.str();
}

bool CodeInjector::codeBinExists(const std::string& name, const std::string& code_name)
{
    if (!codeSegExists(name))
        return false;
    return code_map[name].children.find(code_name)
           != code_map[name].children.end();
}

std::vector<code_bin> CodeInjector::convertCodeSegChildren(const std::string& name)
{
    auto cs = code_map[name].children;
    std::vector<code_bin> ret;
    ret.reserve(cs.size());
    std::for_each(cs.begin(), cs.end(),  [&ret](std::pair<const std::string, code_bin> element)
    {
        ret.push_back(element.second);
    });
    return ret;
}

unsigned long CodeInjector::findCodeCave(const std::string& name, unsigned long siz)
{
    auto& cs = code_map[name];
    auto cl = convertCodeSegChildren(name);
    std::sort(cl.begin(), cl.end());
    unsigned long end_addr = cs.addr;
    for (auto& el : cl)
    {
        if (el.addr >= end_addr + siz)
        {
            return end_addr;
        }
        end_addr = el.addr + el.siz;
    }
    if (end_addr <= cs.addr + cs.siz)
        return end_addr;
    return 0;
}
