#pragma once
#include "../Gateware/Gateware.h"
#include "Defines.h"
#include <vector>
#include "h2bParser.h"
#include "vulkan/vulkan_core.h"

struct OBJATTRIBUTES
{
	H2B::VECTOR Kd; float d;
	H2B::VECTOR Ks; float Ns;
	H2B::VECTOR Ka; float sharpness;
	H2B::VECTOR Tf; float Ni;
	H2B::VECTOR Ke; unsigned illum;
};

#define MAX_SUBMESH_PER_DRAW 1024
struct SHADER_MODEL_DATA
{
	GW::MATH::GVECTORF lightDir, lightCol, ambLight, camPos[2];
	GW::MATH::GMATRIXF view[2], proj;

	GW::MATH::GMATRIXF matricies[MAX_SUBMESH_PER_DRAW];
	OBJATTRIBUTES materials[MAX_SUBMESH_PER_DRAW] ;
};

struct Vertex
{
	union
	{
		struct {
			float x, y, z;
		};
		float xyz[3];
	};
	union
	{
		struct {
			float u, v, w;
		};
		float uvw[3];
	};
	union
	{
		struct {
			float n, r, m;
		};
		float nrm[3];
	};
};
struct PUSH_CONSTANTS
{
	unsigned materialIndex, modelIndex;
	unsigned camIndex;
	int padding[29] = {};
};

struct model
{
	std::vector<GW::MATH::GMATRIXF> matrices;
	VkBuffer vertexHandle;
	VkBuffer indexHandle;
	std::vector<Vertex> verts;
	VkDeviceMemory vertexData;
	VkDeviceMemory indexData;
	std::vector<int> indices;
	unsigned materialCount;
	unsigned meshCount;
	std::vector<OBJATTRIBUTES> attributes;
	std::vector<H2B::MESH> meshes;
};

struct modelInfo
{
	std::vector<GW::MATH::GMATRIXF> matrices;
	unsigned matOffset;
	unsigned matrixCount;

	unsigned vertOffset;
	unsigned vertCount;
	
	unsigned indexOffset;
	unsigned indexCount;
	
	unsigned materialOffset;
	unsigned materialCount;

	unsigned meshCount;
	std::vector<H2B::MESH> meshes;

	std::vector<H2B::BATCH> batches;

	GW::MATH::GVECTORF boundBoxCenter, boundBoxExtents;
};