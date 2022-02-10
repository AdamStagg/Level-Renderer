

[[vk::push_constant]]
cbuffer MESH_INDEX
{
    uint mesh_id;
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
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
    float3 wld : WORLD;
};

// TODO: 2i
// an ultra simple hlsl vertex shader
// TODO: Part 2b


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
    float4 lightDir, lightCol, ambLight, camPos;
    matrix view, proj;

    matrix worlds[MAX_SUBMESH_PER_DRAW];
    OBJ_ATTRIBUTES attributes[MAX_SUBMESH_PER_DRAW];
};

StructuredBuffer<ShaderData> frameData;
// TODO: Part 4g
// TODO: Part 2i
// TODO: Part 3e
// TODO: Part 4a
// TODO: Part 1f
// TODO: Part 4b
PS_INPUT main(VS_INPUT input) : SV_POSITION
{
    // TODO: Part 1h
    PS_INPUT output = (PS_INPUT) 0;
    output.pos = float4(input.pos, 1);
    output.uvw = input.uvw;
    output.nrm = input.nrm;
	// TODO: Part 2i
    output.pos = mul(frameData[0].worlds[mesh_id], output.pos);
    output.wld = output.pos.xyz;
    output.pos = mul(frameData[0].view, output.pos);
    output.pos = mul(frameData[0].proj, output.pos);
		// TODO: Part 4e
	// TODO: Part 4b
    output.nrm = mul(frameData[0].worlds[mesh_id], float4(output.nrm, 1));
		// TODO: Part 4e
    return output;
}