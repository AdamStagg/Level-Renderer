#include "Tree.h"

Tree::Tree()
{
	root = nullptr;
	count = 0;
}

Tree::~Tree()
{
	Tree::Clear();
}

void Tree::Clear()
{
	Tree::Clear(root);
}

void Tree::Clear(Node* branch)
{
	if (branch == nullptr)
		return;
	Clear(branch->left);
	Clear(branch->right);
	delete branch;
}

unsigned Tree::Count()
{
	return count;
}

void Tree::GetDrawInfo(std::vector<Vertex>& vertices, std::vector<int>& indices)
{
	if (root != nullptr)
	{
		Tree::getDrawInfo(root, vertices, indices);
	}
	else
	{
		vertices = std::vector<Vertex>();
		indices = std::vector<int>();
	}
}
void Tree::getDrawInfo(Node* _curr, std::vector<Vertex>& vertices, std::vector<int>& indices)
{
	if (_curr == nullptr) return;

	getDrawInfo(_curr->left, vertices, indices);
	getDrawInfo(_curr->right, vertices, indices);

	int vertOffset = vertices.size();

	Vertex* verts = _curr->data.GetVertices();
	int* inds = _curr->data.GetIndices();

	for (size_t i = 0; i < 8; i++)
	{
		vertices.push_back(verts[i]);
	}
	for (size_t i = 0; i < 24; i++)
	{
		indices.push_back(inds[i] + vertOffset);
	}

	delete[] verts;
	delete[] inds;
}


void Tree::AddNode(BoundingBox data)
{
	if (root == nullptr)
	{
		count++;
		root = new Node(data);
		return;
	}

Tree:AddNode(data, root, nullptr);
}

void Tree::AddNode(BoundingBox data, Node* _curr, Node* _parent)
{
	if (_curr->left == nullptr || _curr->right == nullptr) //reached a leaf, add new node
	{
		_curr->left = new Node(_curr->data, _curr);
		_curr->right = new Node(data, _curr);
		count += 2;
		_curr->data = GenerateNewBox(_curr->data, data);
	}
	else { //keep searching
		float leftComp = Tree::ManhattanCost(_curr->left->data, data);
		float RightComp = Tree::ManhattanCost(_curr->right->data, data);

		if (leftComp < RightComp) {
			Tree::AddNode(data, _curr->left, _curr);
		}
		else {
			Tree::AddNode(data, _curr->right, _curr);
		}

		_curr->data = GenerateNewBox(_curr->left->data, _curr->right->data);
	}
}

BoundingBox Tree::GenerateNewBox(BoundingBox box1, BoundingBox box2)
{
	GW::MATH::GVECTORF minbound = { FLT_MAX,FLT_MAX,FLT_MAX, 1 };
	GW::MATH::GVECTORF maxbound = { -FLT_MAX,-FLT_MAX,-FLT_MAX, 1 };

	Vertex* box1v = box1.GetVertices();
	Vertex* box2v = box2.GetVertices();

	for (size_t i = 0; i < 8; i++)
	{
		minbound.x = min(minbound.x, min(box1v[i].x, box2v[i].x));
		minbound.y = min(minbound.y, min(box1v[i].y, box2v[i].y));
		minbound.z = min(minbound.z, min(box1v[i].z, box2v[i].z));

		maxbound.x = max(maxbound.x, max(box1v[i].x, box2v[i].x));
		maxbound.y = max(maxbound.y, max(box1v[i].y, box2v[i].y));
		maxbound.z = max(maxbound.z, max(box1v[i].z, box2v[i].z));
	}

	GW::MATH::GVECTORF center = { (minbound.x + maxbound.x) * .5f, (minbound.y + maxbound.y) * .5f, (minbound.z + maxbound.z) * .5f, 0 };
	GW::MATH::GVECTORF extent = { maxbound.x - center.x, maxbound.y - center.y, maxbound.z - center.z, 0 };

	return { center, extent };

	/*float box1Width =  std::abs(box1.extents.x - box1.center.x);
	float box1Height = std::abs(box1.extents.y - box1.center.y);
	float box1Depth =  std::abs(box1.extents.z - box1.center.z);
	float box2Width =  std::abs(box2.extents.x - box2.center.x);
	float box2Height = std::abs(box2.extents.y - box2.center.y);
	float box2Depth =  std::abs(box2.extents.z - box2.center.z);


	float newBoxBoundsX[2] = {
		min(box1.center.x - box1Width, box2.center.x - box2Width),
		max(box1.center.x + box1Width, box2.center.x + box2Width),
	};
	float newBoxBoundsY[2] = {
		min(box1.center.y - box1Height, box2.center.y - box2Height),
		max(box1.center.y + box1Height, box2.center.y + box2Height),
	};
	float newBoxBoundsZ[2] = {
		min(box1.center.z - box1Depth, box2.center.z - box2Depth),
		max(box1.center.z + box1Depth, box2.center.z + box2Depth),
	};

	BoundingBox output = {};
	output.center = { (newBoxBoundsX[1] + newBoxBoundsX[0]) / 2.0f, (newBoxBoundsY[1] + newBoxBoundsY[0]) / 2.0f, (newBoxBoundsZ[1] + newBoxBoundsZ[0]) / 2.0f, 1 };
	output.extents = {newBoxBoundsX[1], newBoxBoundsY[1], newBoxBoundsZ[1], 0};*/

	delete[] box1v;
	delete[] box2v;

	return {};
}

float Tree::ManhattanCost(BoundingBox box1, BoundingBox box2)
{
	return std::abs((box1.center.x - box2.center.x)) + std::abs((box1.center.y - box2.center.y)) + std::abs((box1.center.z - box2.center.z));
}