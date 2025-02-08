#include "NvApiTypes.h"

#include <nvapi_interface.h>

// Separate to break up a circular dependency 
NvApiTypes::NvApiTypes() {
    for (const auto& entry : nvapi_interface_table) {
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

typedef void* (__stdcall* PFN_NvApi_QueryInterface)(unsigned int InterfaceId);