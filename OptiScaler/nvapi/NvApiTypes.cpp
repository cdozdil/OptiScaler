#include "NvApiTypes.h"

#include <nvapi_interface.h>

constexpr NVAPI_INTERFACE_TABLE fakenvapi_extra[] =
{
    {"Fake_InformFGState", 0x21382138 },
    {"Fake_InformPresentFG", 0x21392139 },
    { "Fake_GetAntiLagCtx", 0x21402140 }
};

NvApiTypes::NvApiTypes() {
    for (const auto& entry : nvapi_interface_table) {
        lookupTable[entry.func] = entry.id;
    }
    for (const auto& entry : fakenvapi_extra) {
        lookupTable[entry.func] = entry.id;
    }
}
NvApiTypes& NvApiTypes::Instance() {
    static NvApiTypes instance;
    return instance;
}

unsigned int NvApiTypes::getId(const std::string& name) const {
    auto it = lookupTable.find(name);
    if (it != lookupTable.end()) {
        return it->second;
    }
    LOG_TRACE("Not a known nvapi interface");
    return 0;
}
