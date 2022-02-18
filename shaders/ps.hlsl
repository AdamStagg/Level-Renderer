[[vk::push_constant]]
cbuffer MESH_INDEX
{
    uint mesh_id;
    uint model_id;
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
float4 main(PS_INPUT input) : SV_TARGET
{
    input.nrm = normalize(input.nrm);
    
	//Directional
    float lightRat = clamp(dot(-frameData[0].lightDir.xyz, input.nrm), 0, 1);

	//Ambient
    lightRat += frameData[0].ambLight.x;

	//Specular
    float3 viewdir = normalize(frameData[0].camPos.xyz - input.wld.xyz); //shader swizzling
    float3 vec = normalize((-frameData[0].lightDir.xyz) + viewdir);
    float intensity = max(pow(clamp(dot(input.nrm, vec), 0, 1), frameData[0].attributes[mesh_id].Ns), 0);
    float3 reflectedLight = frameData[0].lightCol.xyz * frameData[0].attributes[mesh_id].Ks * intensity;

    float3 light = lightRat * frameData[0].attributes[mesh_id].Kd * frameData[0].lightCol.xyz;
    
    light += reflectedLight;
    return saturate(float4(light, 0));
}