#pragma once
#include "../Gateware/Gateware.h"
#include "RendererStructs.h"

struct BoundingBox
{
	GW::MATH::GVECTORF center, extents;

	Vertex* GetVertices()
	{

		GW::MATH::GVECTORF halfDimensions = {extents.x, extents.y, extents.z, 0};

		Vertex* vertices = new Vertex[]
		{
			{{center.x + halfDimensions.x, center.y + halfDimensions.y, center.z + halfDimensions.z}, {}, {}},
			{{center.x + halfDimensions.x, center.y + halfDimensions.y, center.z - halfDimensions.z}, {}, {}},
			{{center.x + halfDimensions.x, center.y - halfDimensions.y, center.z + halfDimensions.z}, {}, {}},
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
		int* indices = new int[24]
		{
			0, 1,
			1, 3,
			3, 2,
			2, 0,
			4, 5,
			5, 7,
			7, 6,
			6, 4,
			0, 4,
			1, 5, 
			2, 6,
			3, 7
		};

		return indices;
	}
};