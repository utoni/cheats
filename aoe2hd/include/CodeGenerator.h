#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <string>
#include <vector>

#include "CodeInjector.h"


std::vector<unsigned char> x86_relJump(unsigned long dst,
                                       unsigned long src);

class CodeGenerator
{
public:
    CodeGenerator(const native_data& nd);
    virtual ~CodeGenerator();
    void clear()
    {
        codes.clear();
    }
    bool hasCode(int index);
    CodeGenerator& addCode(const std::vector<unsigned char>& code);
    CodeGenerator& setCode(int index, const std::vector<unsigned char>& code);
    CodeGenerator& setCodeSized(int index, const std::vector<unsigned char>& code);
    CodeGenerator& setRel32JMP(int index, unsigned long dst, unsigned long src, bool reversed = false);
    std::vector<unsigned char>::size_type buildSize(int maxCodes = -1);
    std::vector<unsigned char> build();
    std::vector<unsigned char> buildAndClear();
    std::string toString();
private:
    const native_data& nd;
    std::vector<std::vector<unsigned char>> codes;
    unsigned long diffRel32JMP(bool reversed, int index = -1)
    {
        return (!reversed ? buildSize(index) - 0x5 : buildSize(index));
    }
};

#endif // CODEGENERATOR_H
