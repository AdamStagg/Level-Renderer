#pragma once
#include "../Gateware/Gateware.h"
#include "RendererStructs.h"

struct BoundingBox
{
	GW::MATH::GVECTORF center, extents;
	std::string name;
	int matrix;

	BoundingBox()
	{
		center = {};
		extents = {};
		name = "";
		matrix = -1;
	}

	BoundingBox(GW::MATH::GVECTORF _center, GW::MATH::GVECTORF _extents, std::string _name = "", int _matrix = 0)
	{
		center = _center;
		extents = _extents;
		name = _name;
		matrix = _matrix;
	}

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

	void AddDrawInfo(std::vector<Vertex>& verts, std::vector<int>& ints)
	{
		Vertex* ver = BoundingBox::GetVertices();
		int* ind = BoundingBox::GetIndices();

		int indOffset = verts.size();

		for (size_t i = 0; i < 8; i++)
		{
			verts.push_back(ver[i]);
		}
		for (size_t i = 0; i < 24; i++)
		{
			ints.push_back(ind[i] + indOffset);
		}

		delete[] ver;
		delete[] ind;
	}
};