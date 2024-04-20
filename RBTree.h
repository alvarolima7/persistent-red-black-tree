#pragma once

#include <iostream>

class RBTree
{
public:
	RBTree() : m_Root(nullptr) {}

	class Node
	{
	public:
		enum class Color{ Red, Black };

		Node(int data, Color color) : Data(data), m_Color(color) {}

		inline Node* Right(int version = INT32_MAX) const { return m_Right; }

		inline Node* Left(int version = INT32_MAX) const { return m_Left; }
		
		inline Node* Parent(int version = INT32_MAX) const { return m_Parent; }
		inline bool HasParent(int version = INT32_MAX) const { return m_Parent; }
		
		inline bool IsRed(int version = INT32_MAX) const { return m_Color == Color::Red; }
		inline bool IsBlack(int version = INT32_MAX) const { return m_Color == Color::Black; }
		
		inline void SetBlack(int version) { m_Color = Color::Black; }
		inline void SetRed(int version) { m_Color = Color::Red; }

		inline void SetLeft(Node* left, int version) { m_Left = left; }
		inline void SetRight(Node* right, int version) { m_Right = right; }
		inline void SetParent(Node* parent, int version) { m_Parent = parent; }
		
		inline virtual bool IsNil() const { return false; }

		inline bool Equals(Node* other) const { return Data == other->Data; }

		inline bool IsLeftChildOf(Node* parent, int version = INT32_MAX) const 
{
			if (!parent)
				return false;

			Node* parentLeft = parent->Left(version);
			if (!parentLeft)
				return false;

			return Equals(parentLeft);
		}

		inline bool IsRightChildOf(Node* parent, int version = INT32_MAX) const 
		{
			if (!parent)
				return false;

			Node* parentRight = parent->Right(version);
			if (!parentRight)
				return false;

			return Equals(parentRight);
		}

		inline Node* Uncle(Node* parent, int version = INT32_MAX) const
		{
			Node* thisParent = Parent(version);
			if (!thisParent)
				return nullptr;
		
			Node* grandParent = thisParent->Parent(version);
			if (!grandParent)
				return nullptr;

			if (parent->IsLeftChildOf(grandParent, version))
				return grandParent->Right(version);
			
			return grandParent->Left(version);
		}

		inline Node* Sibling(int version = INT32_MAX) const
		{
			Node* parent = Parent(version);
			if (!parent)
				return nullptr;

			if (this->IsLeftChildOf(parent, version))
				return parent->Right(version);

			return parent->Left(version);
		}

	public:
		int Data;

	private:
		Node* m_Left{ nullptr };
		Node* m_Right{ nullptr };
		Node* m_Parent{ nullptr };

		Color m_Color;
	};

	class Nil : public Node
	{
	public:
		Nil() : Node(-1, Node::Color::Black) {}
	
		inline bool IsNil() const override { return true; }
	};

	Node* Search(int data, int version = INT32_MAX) const
	{
		Node* current = GetRoot(version);
		while (current && current->Data != data)
			current = data < current->Data ? current->Left(version) : current->Right(version);

		return current;
	}

	void Print(int version = INT32_MAX) const 
	{ 
		Node* root = GetRoot(version);
		if (!root)
			return;

		std::cout << ' ' << root->Data << (root->IsBlack(version) ? " (B)" : " (R)") << std::endl;

		PrintHelper(root->Right(version), 8, version, false);
		PrintHelper(root->Left(version), 8, version, true);
	}

	void Insert(int key)
	{
		Node* current = GetRoot();
		Node* parent = nullptr;

		m_CurrentVersion++;

		while (current)
		{
			parent = current;
			current = key < current->Data ? current->Left() : current->Right();
		}

		Node* newNode = new Node(key, Node::Color::Red);

		if (!parent)
			SetRoot(newNode, m_CurrentVersion);
		else if (key < parent->Data)
			parent->SetLeft(newNode, m_CurrentVersion);
		else
			parent->SetRight(newNode, m_CurrentVersion);

		newNode->SetParent(parent, m_CurrentVersion);

		InsertFixup(newNode);
	}

	void Delete(int key)
	{
		Node* node = Search(key);
		if (!node)
			return;

		m_CurrentVersion++;

		Node* movedUpNode;
		bool deletedNodeWasBlack;
		if (!node->Left() || !node->Right())
		{
			movedUpNode = DeleteNodeWithZeroOrOneChild(node);
			deletedNodeWasBlack = node->IsBlack();
		}
		else
		{
			Node* successor = GetMinimun(node->Right());
			deletedNodeWasBlack = successor->IsBlack();
			node->Data = successor->Data;
			movedUpNode = DeleteNodeWithZeroOrOneChild(successor);
		}

		if (deletedNodeWasBlack)
			DeleteFixup(movedUpNode);

		if (movedUpNode && movedUpNode->IsNil())
			SwapParentsChild(movedUpNode->Parent(), movedUpNode, nullptr);
	}

private:
	void SwapParentsChild(Node* parent, Node* oldChild, Node* newChild)
	{
		if (!parent)
			SetRoot(newChild, m_CurrentVersion);
		else if (oldChild->IsLeftChildOf(parent))
			parent->SetLeft(newChild, m_CurrentVersion);
		else if (oldChild->IsRightChildOf(parent))
			parent->SetRight(newChild, m_CurrentVersion);
		else
			throw std::exception("Invalid parent-child relationship");

		if (newChild)
			newChild->SetParent(parent, m_CurrentVersion);
	}

	void RotateRight(Node* node)
	{
		Node* parent = node->Parent();
		Node* left = node->Left();

		node->SetLeft(left->Right(), m_CurrentVersion);
		if (left->Right())
			left->Right()->SetParent(node, m_CurrentVersion);

		left->SetRight(node, m_CurrentVersion);
		node->SetParent(left, m_CurrentVersion);

		SwapParentsChild(parent, node, left);
	}

	void RotateLeft(Node* node)
	{
		Node* parent = node->Parent();
		Node* right = node->Right();

		node->SetRight(right->Left(), m_CurrentVersion);
		if (right->Left())
			right->Left()->SetParent(node, m_CurrentVersion);

		right->SetLeft(node, m_CurrentVersion);
		node->SetParent(right, m_CurrentVersion);

		SwapParentsChild(parent, node, right);
	}

	Node* DeleteNodeWithZeroOrOneChild(Node* node)
	{
		if (node->Left())
		{
			SwapParentsChild(node->Parent(), node, node->Left());
			return node->Left();
		}
		else if (node->Right())
		{
			SwapParentsChild(node->Parent(), node, node->Right());
			return node->Right();
		}

		Node* newNode = node->IsBlack() ? new Nil() : nullptr;
		SwapParentsChild(node->Parent(), node, newNode);

		return newNode;
	}

	void InsertFixup(Node* node)
	{
		Node* parent = node->Parent();

		if (!parent)
		{
			node->SetBlack(m_CurrentVersion);
			return;
		}

		if (parent->IsBlack())
			return;

		Node* grandParent = parent->Parent();
		if (!grandParent)
		{
			parent->SetBlack(m_CurrentVersion);
			return;
		}

		Node* uncle = node->Uncle(parent);
		if (uncle && uncle->IsRed())
		{
			parent->SetBlack(m_CurrentVersion);
			grandParent->SetRed(m_CurrentVersion);
			uncle->SetBlack(m_CurrentVersion);

			InsertFixup(grandParent);
		}
		else if (parent->IsLeftChildOf(grandParent))
		{
			if (node->IsRightChildOf(parent))
			{
				RotateLeft(parent);
				parent = node;
			}

			RotateRight(grandParent);

			parent->SetBlack(m_CurrentVersion);
			grandParent->SetRed(m_CurrentVersion);
		}
		else
		{
			if (node->IsLeftChildOf(parent))
			{
				RotateRight(parent);
				parent = node;
			}

			RotateLeft(grandParent);

			parent->SetBlack(m_CurrentVersion);
			grandParent->SetRed(m_CurrentVersion);
		}
	}

	void DeleteFixup(Node* node) 
	{
		if (GetRoot()->Equals(node))
		{
			node->SetBlack(m_CurrentVersion);
			return;
		}

		Node* sibling = node->Sibling();
		if (sibling->IsRed())
		{
			HandleRedSibling(node, sibling);
			sibling = node->Sibling();
		}

		if (NodeIsBlack(sibling->Left()) && NodeIsBlack(sibling->Right())) 
		{
			sibling->SetRed(m_CurrentVersion);

			if (node->Parent()->IsRed())
				node->Parent()->SetBlack(m_CurrentVersion);
			else
				DeleteFixup(node->Parent());
		}
		else
			HandleBlackSiblingWithAtLeastOneRedChild(node, sibling);
	}

	void HandleRedSibling(Node* node, Node* sibling)
	{
		sibling->SetBlack(m_CurrentVersion);
		node->Parent()->SetRed(m_CurrentVersion);

		if (node->IsLeftChildOf(node->Parent()))
			RotateLeft(node->Parent());
		else
			RotateRight(node->Parent());
	}

	void HandleBlackSiblingWithAtLeastOneRedChild(Node* node, Node* sibling)
	{
		bool isLeftChild = node->IsLeftChildOf(node->Parent());

		if (NodeIsBlack(sibling->Right()) && isLeftChild)
		{
			sibling->Left()->SetBlack(m_CurrentVersion);
			sibling->SetRed(m_CurrentVersion);
			RotateRight(sibling);
			sibling = node->Parent()->Right();
		}
		else if (NodeIsBlack(sibling->Left()) && !isLeftChild)
		{
			sibling->Right()->SetBlack(m_CurrentVersion);
			sibling->SetRed(m_CurrentVersion);
			RotateLeft(sibling);
			sibling = node->Parent()->Left();
		}

		if (node->Parent()->IsRed())
			sibling->SetRed(m_CurrentVersion);
		else
			sibling->SetBlack(m_CurrentVersion);
		node->Parent()->SetBlack(m_CurrentVersion);
		if (isLeftChild)
		{
			sibling->Right()->SetBlack(m_CurrentVersion);
			RotateLeft(node->Parent());
		}
		else
		{
			sibling->Left()->SetBlack(m_CurrentVersion);
			RotateRight(node->Parent());
		}
	}

	void PrintHelper(Node* node, int ident, int version, bool isLeftChild) const
	{
		if (!node)
			return;

		std::cout << std::string(ident, ' ')
				  << (isLeftChild ? "L" : "R")
			      << node->Data 
				  << (node->IsBlack(version) ? " (B)" : " (V)") << std::endl;
		PrintHelper(node->Right(version), ident + 8, version, false);
		PrintHelper(node->Left(version), ident + 8, version, true);
	}

	void SetRoot(Node* root, int version) { m_Root = root; }
	Node* GetRoot(int version = INT32_MAX) const { return m_Root; }

	Node* GetMinimun(Node* node, int version = INT32_MAX) const
	{
		while (node->Left(version))
			node = node->Left(version);

		return node;
	}

	bool NodeIsBlack(Node* node, int version = INT32_MAX) const
	{
		return !node || node->IsBlack(version);
	}

private:
	int m_CurrentVersion{ 0 };
	Node* m_Root;
};