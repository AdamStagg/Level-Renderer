#pragma once
#include "BoundingBox.h"
class Tree
{

	struct Node
	{
		BoundingBox data;
		Node* left, *right, *parent;
		Node()
		{
			data = {}, left = nullptr, right = nullptr, parent = nullptr;
		}
		Node(BoundingBox data, Node* parent = nullptr)
		{
			this->data = data;
			this->left = nullptr;
			this->right = nullptr;
			this->parent = parent;
		}
	};

	Node* root;

	void Clear(Node* branch);

public:
	~Tree();
	Tree();
	void Clear();
	void AddNode(BoundingBox data);
	void AddNode(BoundingBox data, Node* _curr, Node* _parent);
	BoundingBox GenerateNewBox(BoundingBox box1, BoundingBox box2);
	float ManhattanCost(BoundingBox box1, BoundingBox box2);

};