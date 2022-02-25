[[vk::push_constant]]
cbuffer MESH_INDEX
{
    uint mesh_id;
    uint model_id;
    uint cam_id;
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
};
struct PS_INPUT
{
    float4 pos : SV_POSITION;
};

struct OBJ_ATTRIBUTES
{
    float3 Kd;
    float d;
    float3 Ks;
    float Ns;
    float3 Ka;
    float sharpness;
    float3 Tf;
    float Ni;
    float3 Ke;
    uint illum;
};

#define MAX_SUBMESH_PER_DRAW 1024
struct ShaderData
{
    float4 lightDir, lightCol, ambLight, camPos[2];
    matrix view[2], proj;

    matrix worlds[MAX_SUBMESH_PER_DRAW];
    OBJ_ATTRIBUTES attributes[MAX_SUBMESH_PER_DRAW];
};

StructuredBuffer<ShaderData> frameData;

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;

    output.pos = mul(frameData[0].view[cam_id], float4(input.pos, 1));
    output.pos = mul(frameData[0].proj, output.pos);

    return output;
}