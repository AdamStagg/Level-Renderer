#pragma once
#include "../Gateware/Gateware.h"
#include "RendererStructs.h"

struct BoundingBox
{
	GW::MATH::GVECTORF center, extents;

	Vertex* GetVertices()
	{

		GW::MATH::GVECTORF halfDimensions = {extents.x - center.x, extents.y - center.y, extents.z - center.z, 0};

		Vertex vertices[] =
		{
			{{center.x + halfDimensions.x, center.y + halfDimensions.y, center.z + halfDimensions.z}, {}, {}},
			{{center.x + halfDimensions.x, center.y - halfDimensions.y, center.z - halfDimensions.z}, {}, {}},
			{{center.x + halfDimensions.x, center.y + halfDimensions.y, center.z + halfDimensions.z}, {}, {}},
			{{center.x + halfDimensions.x, center.y - halfDimensions.y, center.z - halfDimensions.z}, {}, {}},
			{{center.x - halfDimensions.x, center.y + halfDimensions.y, center.z + halfDimensions.z}, {}, {}},
			{{center.x - halfDimensions.x, center.y + halfDimensions.y, center.z - halfDimensions.z}, {}, {}},
			{{center.x - halfDimensions.x, center.y - halfDimensions.y, center.z + halfDimensions.z}, {}, {}},
			{{center.x - halfDimensions.x, center.y - halfDimensions.y, center.z - halfDimensions.z}, {}, {}},
		};
		
		return vertices;
	}

	int* GetIndices()
	{
		int indices[] =
		{
			0, 1,
			1, 2,
			2, 3,
			3, 0,
			4, 5,
			5, 6,
			6, 7,
			7, 4,
			0, 4,
			1, 5,
			2, 6,
			3, 7
		};

		return indices;
	}
};