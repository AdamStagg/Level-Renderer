#pragma once
#include "BoundingBox.h"
class Tree
{
public:
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
	Node* root = nullptr;
	unsigned count;

	void Clear(Node* branch);
	void getDrawInfo(Node* _curr, std::vector<Vertex>& vertices, std::vector<int>& indices);

public:
	~Tree();
	Tree();
	void Clear();
	unsigned Count();
	void GetDrawInfo(std::vector<Vertex>& vertices, std::vector<int>& indices);
	void AddNode(BoundingBox data);
	void AddNode(BoundingBox data, Node* _curr, Node* _parent);
	BoundingBox GenerateNewBox(BoundingBox box1, BoundingBox box2);
	float ManhattanCost(BoundingBox box1, BoundingBox box2);

};