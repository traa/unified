#include "CNWSRules.hpp"
#include "API/Functions.hpp"
#include "Platform/ASLR.hpp"

namespace NWNXLib {

namespace API {

CNWSRules::CNWSRules(const CNWSRules&)
{
    __builtin_trap(); // This is an auto-generated stub. You probably shouldn't use it.
}

CNWSRules& CNWSRules::operator=(const CNWSRules&)
{
    __builtin_trap(); // This is an auto-generated stub. You probably shouldn't use it.
}

CNWSRules::CNWSRules()
{
    CNWSRules__CNWSRulesCtor__0(this);
}

CNWSRules::~CNWSRules()
{
    CNWSRules__CNWSRulesDtor__0(this);
}

void CNWSRules__CNWSRulesCtor__0(CNWSRules* thisPtr)
{
    using FuncPtrType = void(__attribute__((cdecl)) *)(CNWSRules*);
    uintptr_t address = Platform::ASLR::GetRelocatedAddress(Functions::CNWSRules__CNWSRulesCtor__0);
    FuncPtrType func = reinterpret_cast<FuncPtrType>(address);
    func(thisPtr);
}

void CNWSRules__CNWSRulesDtor__0(CNWSRules* thisPtr)
{
    using FuncPtrType = void(__attribute__((cdecl)) *)(CNWSRules*, int);
    uintptr_t address = Platform::ASLR::GetRelocatedAddress(Functions::CNWSRules__CNWSRulesDtor__0);
    FuncPtrType func = reinterpret_cast<FuncPtrType>(address);
    func(thisPtr, 2);
}

}

}