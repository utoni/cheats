#ifndef CODEPATCHER_H
#define CODEPATCHER_H

#include <vector>
#include <map>

extern "C" {
#include "native.h"
}


typedef struct code_patch
{
    unsigned long addr;
    std::vector<unsigned char> old_code;
    std::vector<unsigned char> new_code;
    long new_offset;
    long suspend;
} code_patch;

class CodePatcher
{
public:
    CodePatcher(const native_data& nd);
    virtual ~CodePatcher();
    bool addPatch(const std::string& name,
                  unsigned long addr,
                  const std::vector<unsigned char>& old_code,
                  const std::vector<unsigned char>& new_code,
                  long new_offset = 0);
    void setPatchSuspend(const std::string& name, long doSuspend);
    bool doPatch(const std::string& name, int doUnPatch);
    bool autoPatch(const std::string& name);
    std::string toString();
private:
    const native_data& nd;
    std::map<std::string, code_patch> patch_map;
    bool codePatchExists(const std::string& name)
    {
        return patch_map.find(name) != patch_map.end();
    }
    bool codeCmp(unsigned long addr, std::vector<unsigned char> code);
};

#endif // CODEPATCHER_H
