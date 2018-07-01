#include "CodeGenerator.h"

#include <assert.h>
#include <sstream>
#include <iomanip>

#include "native.h"
#include "utils.h"


std::vector<unsigned char> x86_relJump(unsigned long dst,
                                       unsigned long src)
{
    std::vector<unsigned char> code(5);
    code[0] = 0xE9;
    unsigned long addr = dst - src;
    code[1] = (*((unsigned char *)(&addr)+0));
    code[2] = (*((unsigned char *)(&addr)+1));
    code[3] = (*((unsigned char *)(&addr)+2));
    code[4] = (*((unsigned char *)(&addr)+3));
    return code;
}

CodeGenerator::CodeGenerator(const native_data& nd)
    : nd(nd), codes()
{
}

CodeGenerator::~CodeGenerator()
{
}

CodeGenerator& CodeGenerator::addCode(const std::vector<unsigned char>& code)
{
    codes.push_back(code);
    return *this;
}

CodeGenerator& CodeGenerator::setCode(int index, const std::vector<unsigned char>& code)
{
    codes.at(index) = code;
    return *this;
}

CodeGenerator& CodeGenerator::setCodeSized(int index, const std::vector<unsigned char>& code)
{
    assert(codes.at(index).size() == code.size());
    return setCode(index, code);
}

CodeGenerator& CodeGenerator::setRel32JMP(int index, unsigned long dst, unsigned long src, bool reversed)
{
    if (!reversed)
    {
        dst += nd.proc.modbase - diffRel32JMP(reversed, index);
    }
    else
    {
        src += nd.proc.modbase + diffRel32JMP(reversed, index);
    }
    auto jmp = x86_relJump(dst, src);
    setCodeSized(index, jmp);
    return *this;
}

std::vector<unsigned char>::size_type CodeGenerator::buildSize(int maxCodes)
{
    std::vector<unsigned char>::size_type total = 0;
    for (auto& code : codes)
    {
        total += code.size();
        if (maxCodes-- == 0)
            break;
    }
    return total;
}

std::vector<unsigned char> CodeGenerator::build()
{
    std::vector<unsigned char> result;
    for (auto& code : codes)
    {
        result.insert(result.end(), code.begin(), code.end());
    }
    return result;
}

std::vector<unsigned char> CodeGenerator::buildAndClear()
{
    auto result = build();
    clear();
    return result;
}

std::string CodeGenerator::toString()
{
    std::stringstream out;
    out << "CodeBin: " << utils::convertBinToHexstr(build()) << std::endl;
    return out.str();
}
