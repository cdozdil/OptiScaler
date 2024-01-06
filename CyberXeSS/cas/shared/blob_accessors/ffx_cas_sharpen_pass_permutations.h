#include "ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847.h"
#include "ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a.h"
#include "ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f.h"
#include "ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2.h"
#include "ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77.h"
#include "ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde.h"
#include "ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750.h"
#include "ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65.h"
#include "ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb.h"
#include "ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6.h"

typedef union ffx_cas_sharpen_pass_PermutationKey {
    struct {
        uint32_t FFX_CAS_OPTION_SHARPEN_ONLY : 1;
        uint32_t FFX_CAS_COLOR_SPACE_CONVERSION : 3;
    };
    uint32_t index;
} ffx_cas_sharpen_pass_PermutationKey;

typedef struct ffx_cas_sharpen_pass_PermutationInfo {
    const uint32_t       blobSize;
    const unsigned char* blobData;


    const uint32_t  numConstantBuffers;
    const char**    constantBufferNames;
    const uint32_t* constantBufferBindings;
    const uint32_t* constantBufferCounts;
    const uint32_t* constantBufferSpaces;

    const uint32_t  numSRVTextures;
    const char**    srvTextureNames;
    const uint32_t* srvTextureBindings;
    const uint32_t* srvTextureCounts;
    const uint32_t* srvTextureSpaces;

    const uint32_t  numUAVTextures;
    const char**    uavTextureNames;
    const uint32_t* uavTextureBindings;
    const uint32_t* uavTextureCounts;
    const uint32_t* uavTextureSpaces;

    const uint32_t  numSRVBuffers;
    const char**    srvBufferNames;
    const uint32_t* srvBufferBindings;
    const uint32_t* srvBufferCounts;
    const uint32_t* srvBufferSpaces;

    const uint32_t  numUAVBuffers;
    const char**    uavBufferNames;
    const uint32_t* uavBufferBindings;
    const uint32_t* uavBufferCounts;
    const uint32_t* uavBufferSpaces;

    const uint32_t  numSamplers;
    const char**    samplerNames;
    const uint32_t* samplerBindings;
    const uint32_t* samplerCounts;
    const uint32_t* samplerSpaces;

    const uint32_t  numRTAccelerationStructures;
    const char**    rtAccelerationStructureNames;
    const uint32_t* rtAccelerationStructureBindings;
    const uint32_t* rtAccelerationStructureCounts;
    const uint32_t* rtAccelerationStructureSpaces;
} ffx_cas_sharpen_pass_PermutationInfo;

static const uint32_t g_ffx_cas_sharpen_pass_IndirectionTable[] = {
    3,
    0,
    6,
    2,
    7,
    5,
    4,
    1,
    9,
    8,
    0,
    0,
    0,
    0,
    0,
    0,
};

static const ffx_cas_sharpen_pass_PermutationInfo g_ffx_cas_sharpen_pass_PermutationInfo[] = {
    { g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_size, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_data, 1, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_CBVResourceNames, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_CBVResourceBindings, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_CBVResourceCounts, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_CBVResourceSpaces, 1, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_TextureSRVResourceNames, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_TextureSRVResourceBindings, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_TextureSRVResourceCounts, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_TextureSRVResourceSpaces, 1, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_TextureUAVResourceNames, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_TextureUAVResourceBindings, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_TextureUAVResourceCounts, g_ffx_cas_sharpen_pass_d8fdba1e88140f2ffede2256569e6847_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_size, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_data, 1, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_CBVResourceNames, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_CBVResourceBindings, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_CBVResourceCounts, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_CBVResourceSpaces, 1, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_TextureSRVResourceNames, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_TextureSRVResourceBindings, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_TextureSRVResourceCounts, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_TextureSRVResourceSpaces, 1, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_TextureUAVResourceNames, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_TextureUAVResourceBindings, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_TextureUAVResourceCounts, g_ffx_cas_sharpen_pass_fa3d883becd2c65411840779acf5d69a_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_size, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_data, 1, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_CBVResourceNames, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_CBVResourceBindings, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_CBVResourceCounts, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_CBVResourceSpaces, 1, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_TextureSRVResourceNames, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_TextureSRVResourceBindings, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_TextureSRVResourceCounts, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_TextureSRVResourceSpaces, 1, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_TextureUAVResourceNames, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_TextureUAVResourceBindings, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_TextureUAVResourceCounts, g_ffx_cas_sharpen_pass_88708abc518c21de032439084a94984f_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_size, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_data, 1, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_CBVResourceNames, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_CBVResourceBindings, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_CBVResourceCounts, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_CBVResourceSpaces, 1, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_TextureSRVResourceNames, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_TextureSRVResourceBindings, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_TextureSRVResourceCounts, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_TextureSRVResourceSpaces, 1, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_TextureUAVResourceNames, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_TextureUAVResourceBindings, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_TextureUAVResourceCounts, g_ffx_cas_sharpen_pass_dd67b43cea4b5b5de2be677e3d8fbed2_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_size, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_data, 1, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_CBVResourceNames, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_CBVResourceBindings, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_CBVResourceCounts, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_CBVResourceSpaces, 1, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_TextureSRVResourceNames, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_TextureSRVResourceBindings, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_TextureSRVResourceCounts, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_TextureSRVResourceSpaces, 1, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_TextureUAVResourceNames, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_TextureUAVResourceBindings, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_TextureUAVResourceCounts, g_ffx_cas_sharpen_pass_ee2bdf5ae8a17c2f999bce0f168d7d77_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_size, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_data, 1, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_CBVResourceNames, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_CBVResourceBindings, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_CBVResourceCounts, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_CBVResourceSpaces, 1, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_TextureSRVResourceNames, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_TextureSRVResourceBindings, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_TextureSRVResourceCounts, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_TextureSRVResourceSpaces, 1, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_TextureUAVResourceNames, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_TextureUAVResourceBindings, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_TextureUAVResourceCounts, g_ffx_cas_sharpen_pass_614a6e8563a7739e2e1d1da9a912cfde_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_size, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_data, 1, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_CBVResourceNames, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_CBVResourceBindings, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_CBVResourceCounts, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_CBVResourceSpaces, 1, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_TextureSRVResourceNames, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_TextureSRVResourceBindings, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_TextureSRVResourceCounts, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_TextureSRVResourceSpaces, 1, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_TextureUAVResourceNames, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_TextureUAVResourceBindings, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_TextureUAVResourceCounts, g_ffx_cas_sharpen_pass_88eec194ce38246cc0c6cef61356d750_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_size, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_data, 1, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_CBVResourceNames, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_CBVResourceBindings, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_CBVResourceCounts, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_CBVResourceSpaces, 1, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_TextureSRVResourceNames, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_TextureSRVResourceBindings, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_TextureSRVResourceCounts, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_TextureSRVResourceSpaces, 1, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_TextureUAVResourceNames, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_TextureUAVResourceBindings, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_TextureUAVResourceCounts, g_ffx_cas_sharpen_pass_fe571d470c8b4b84ac904f8b27772e65_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_size, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_data, 1, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_CBVResourceNames, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_CBVResourceBindings, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_CBVResourceCounts, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_CBVResourceSpaces, 1, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_TextureSRVResourceNames, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_TextureSRVResourceBindings, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_TextureSRVResourceCounts, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_TextureSRVResourceSpaces, 1, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_TextureUAVResourceNames, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_TextureUAVResourceBindings, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_TextureUAVResourceCounts, g_ffx_cas_sharpen_pass_713b2dd57ee05ace40553c55be3046cb_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    { g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_size, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_data, 1, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_CBVResourceNames, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_CBVResourceBindings, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_CBVResourceCounts, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_CBVResourceSpaces, 1, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_TextureSRVResourceNames, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_TextureSRVResourceBindings, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_TextureSRVResourceCounts, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_TextureSRVResourceSpaces, 1, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_TextureUAVResourceNames, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_TextureUAVResourceBindings, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_TextureUAVResourceCounts, g_ffx_cas_sharpen_pass_8af9cc39542bf35bb40b4bb8ae4d55b6_TextureUAVResourceSpaces, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};

