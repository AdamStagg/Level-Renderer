// TODO: Part 2b
[[vk::push_constant]]
cbuffer MESH_INDEX
{
    uint mesh_id;
};
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
    float3 wld : WORLD;
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
    float4 lightDir, lightCol, ambLight, camPos;
    matrix view, proj;

    matrix worlds[MAX_SUBMESH_PER_DRAW];
    OBJ_ATTRIBUTES attributes[MAX_SUBMESH_PER_DRAW];
};

StructuredBuffer<ShaderData> frameData;
// TODO: Part 4g
// TODO: Part 2i
// TODO: Part 3e
// an ultra simple hlsl pixel shader
// TODO: Part 4b
float4 main(PS_INPUT input) : SV_TARGET
{
	//return float4(frameData[0].attributes[mesh_id].Kd, 1); // TODO: Part 1a
	// TODO: Part 3a
	// TODO: Part 4c //shader swizzling

	//Directional
    float3 lightRat = clamp(dot(-frameData[0].lightDir.xyz, input.nrm), 0, 1);

	//Ambient
    lightRat += frameData[0].ambLight;

	// TODO: Part 4g (half-vector or reflect method your choice)
	//Specular
    float3 viewdir = normalize(frameData[0].camPos.xyz - input.wld.xyz); //shader swizzling
    float3 vec = normalize((-frameData[0].lightDir.xyz) + viewdir);
    float3 intensity = max(pow(clamp(dot(input.nrm, vec), 0, 1), frameData[0].attributes[mesh_id].Ns), 0);
    float3 reflectedLight = frameData[0].lightCol.xyz * frameData[0].attributes[mesh_id].Ks * intensity;

    float3 light = float4(lightRat * frameData[0].attributes[mesh_id].Kd * frameData[0].lightCol.xyz, 1);

    light += float4(reflectedLight, 0);
    return light; //float4(reflectedLight, 1);
}