#ifndef CODEINJECTOR_H
#define CODEINJECTOR_H

#include <vector>
#include <map>
#include <string>

extern "C" {
#include "native.h"
}


typedef struct code_bin
{
    unsigned long addr;
    unsigned long siz;
    bool operator<(const code_bin& a) const
    {
        return addr < a.addr;
    }
} code_bin;

typedef struct code_seg
{
    unsigned long addr;
    unsigned long siz;
    std::map<const std::string, code_bin> children;
} code_seg;

class CodeInjector
{
public:
    CodeInjector(const native_data& nd);
    virtual ~CodeInjector();
    bool allocCodeSegment(const std::string& name,
                          unsigned long siz = 4096);
    bool addCode(const std::string& name, const std::string& code_name,
                 const std::vector<unsigned char>& code);
    bool addCode(const std::string& name, const std::string& code_name,
                 const std::string& code);
    bool addCode(const std::string& name, const std::string& code_name,
                 unsigned long siz);
    bool setCode(const std::string& name, const std::string& code_name,
                 const std::vector<unsigned char>& code,
                 unsigned long offset = 0);
    bool delCode(const std::string& name, const std::string& code_name);
    unsigned long getCodeAddr(const std::string& name, const std::string& code_name);
    bool getCodeSeg(const std::string& name, code_seg *seg);
    bool getCodeBin(const std::string& name, const std::string& code_name, code_bin *bin);
    std::string toString();
private:
    const native_data& nd;
    std::map<std::string, code_seg> code_map;
    bool codeSegExists(const std::string& name)
    {
        return code_map.find(name) != code_map.end();
    }
    bool codeBinExists(const std::string& name, const std::string& code_name);
    std::vector<code_bin> convertCodeSegChildren(const std::string& name);
    unsigned long findCodeCave(const std::string& name, unsigned long siz);
};

#endif // CODEINJECTOR_H
