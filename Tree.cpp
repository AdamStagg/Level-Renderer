#include "Tree.h"

Tree::Tree()
{
	root = nullptr;
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

void Tree::AddNode(BoundingBox data)
{
	if (root = nullptr)
	{
		root = new Node(data);
	}

	Tree:AddNode(data, root, nullptr);
}

void Tree::AddNode(BoundingBox data, Node* _curr, Node* _parent)
{
	if (_curr->left == nullptr || _curr->right == nullptr) //reached a leaf, add new node
	{
		_curr->left = new Node(_curr->data, _curr);
		_curr->right = new Node(data, _curr);
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
	float newBoxBoundsX[2] = {
		min(box1.extents.x - box1.center.x, box2.extents.x - box2.center.x),
		max(box1.extents.x, box2.extents.x),
	};
	float newBoxBoundsY[2] = {
		min(box1.extents.y - box1.center.y, box2.extents.y - box2.center.y),
		max(box1.extents.y, box2.extents.y),
	};
	float newBoxBoundsZ[2] = {
		min(box1.extents.z - box1.center.z, box2.extents.z - box2.center.z),
		max(box1.extents.z, box2.extents.z),
	};

	BoundingBox output = {};
	output.center = { (newBoxBoundsX[1] - newBoxBoundsX[0]) / 2.0f, (newBoxBoundsY[1] - newBoxBoundsY[0]) / 2.0f, (newBoxBoundsZ[1] - newBoxBoundsZ[0]) / 2.0f, 0 };
	output.extents = {newBoxBoundsX[1], newBoxBoundsY[1], newBoxBoundsZ[1], 0};

	return output;
}

float Tree::ManhattanCost(BoundingBox box1, BoundingBox box2)
{
	return std::abs((box1.center.x - box2.center.x)) + std::abs((box1.center.y - box2.center.y)) + std::abs((box1.center.z - box2.center.z));
}