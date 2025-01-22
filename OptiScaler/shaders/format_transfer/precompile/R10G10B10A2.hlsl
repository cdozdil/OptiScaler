Texture2D<float4> SourceTexture : register(t0);
RWTexture2D<uint> DestinationTexture : register(u0); // R10G10B10A2_UNORM destination texture

// Shader to perform the conversion
[numthreads(512, 1, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Read from the source texture
    float4 srcColor = SourceTexture.Load(int3(dispatchThreadID.xy, 0));

    // Clamp color channels to [0, 1] range, as R10G10B10A2_UNORM is normalized
    srcColor = saturate(srcColor); // Ensures all values are between 0 and 1

    // Convert each channel to its corresponding bit range
    uint R = (uint) (srcColor.r * 1023.0f); // 10 bits for Red
    uint G = (uint) (srcColor.g * 1023.0f); // 10 bits for Green
    uint B = (uint) (srcColor.b * 1023.0f); // 10 bits for Blue
    uint A = (uint) (srcColor.a * 3.0f); // 2 bits for Alpha

    // Pack the values into a single 32-bit unsigned int
    uint packedColor = R | G << 10 | B << 20 | A << 30;

    // Write the packed color to the destination texture
    DestinationTexture[dispatchThreadID.xy] = packedColor;
}
