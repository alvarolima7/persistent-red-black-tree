#pragma once

#include <cassert>
#include <vector>
#include <iostream>

class RBTree
{
public:
	RBTree()
	{
		m_Roots.reserve(MaxOperations);
		m_Roots.emplace_back(nullptr, 0);
	}

	class Node
	{
	public:
		enum class Color { Black, Red };

		class Modification
		{
		public:
			union Field
			{
				Color TheColor;
				Node* Pointer;

				Field(Node* pointer) : Pointer(pointer) {}
				Field(Color color) : TheColor(color) {}
			};

			enum class Type { Left, Right, Parent, Color };

			static bool IsPointer(Type fieldType) { return fieldType != Type::Color; }

			Modification(Type fieldType, Field field, int version) : FieldType(fieldType), TheField(field), Version(version) {}

		public:
			Type FieldType;
			int Version;
			Field TheField;
		};

		Node(int data, Color color) : Data(data), m_Color(color) { m_Mods.reserve(ModificationsLimit); }
		Node(int data, Color color, Node* left, Node* right, Node* parent, Node* returnLeft, Node* returnRight, Node* returnParent)
			: Data(data), m_Color(color), m_Left(left), m_Right(right), m_Parent(parent), m_ReturnLeft(returnLeft),
			m_ReturnRight(returnRight), m_ReturnParent(returnParent)
		{
			m_Mods.reserve(ModificationsLimit);
		}

		static constexpr int ModificationsLimit = 6;

		inline virtual bool IsNil() const { return false; }

		inline Color GetColor(int version = INT32_MAX) const { return GetField(Modification::Type::Color, version).TheColor; }
		inline bool IsRed(int version = INT32_MAX) const { return GetColor(version) == Color::Red; }
		inline bool IsBlack(int version = INT32_MAX) const { return GetColor(version) == Color::Black; }

		inline Node* Left(int version = INT32_MAX) const
		{
			return GetField(Modification::Type::Left, version).Pointer;
		}
		inline Node* Right(int version = INT32_MAX) const
		{
			return GetField(Modification::Type::Right, version).Pointer;
		}
		inline Node* Parent(int version = INT32_MAX) const
		{
			return GetField(Modification::Type::Parent, version).Pointer;
		}

		inline void SetBlack(int version) { MakeModification(Modification::Type::Color, Color::Black, version); }
		inline void SetRed(int version) { MakeModification(Modification::Type::Color, Color::Red, version); }
		inline void CopyColor(Node* other, int version) { MakeModification(Modification::Type::Color, other->GetColor(version), version); }

		inline void SetLeft(Node* left, int version)
		{
			MakeModification(Modification::Type::Left, left, version);
		}
		inline void SetRight(Node* right, int version)
		{
			MakeModification(Modification::Type::Right, right, version);
		}
		inline void SetParent(Node* parent, int version)
		{
			MakeModification(Modification::Type::Parent, parent, version);
		}

		inline bool Equals(Node* other) const { return Data == other->Data; }

		inline bool IsLeftChildOf(Node* node, int version = INT32_MAX) const
		{
			if (!node)
				return false;

			Node* nodeLeft = node->Left(version);
			if (!nodeLeft)
				return false;

			return Equals(nodeLeft);
		}
		inline bool IsRightChildOf(Node* node, int version = INT32_MAX) const
		{
			if (!node)
				return false;

			Node* nodeRight = node->Right(version);
			if (!nodeRight)
				return false;

			return Equals(nodeRight);
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

	private:
		inline Modification::Field GetField(Modification::Type fieldType, int version) const
		{
			if (m_Next && LatestVersion() <= version)
				return m_Next->GetField(fieldType, version);

			for (auto mod = m_Mods.rbegin(); mod != m_Mods.rend(); mod++)
			{
				if (mod->Version <= version && mod->FieldType == fieldType)
					return mod->TheField;
			}

			switch (fieldType)
			{
			case Modification::Type::Left:
				return m_Left;
			case Modification::Type::Right:
				return m_Right;
			case Modification::Type::Parent:
				return m_Parent;
			case Modification::Type::Color:
				return m_Color;
			}
		}

		inline void MakeModification(Modification::Type fieldType, Modification::Field field, int version)
		{
			if (m_Next)
			{
				m_Next->MakeModification(fieldType, field, version);
				return;
			}

			assert(m_Mods.size() < ModificationsLimit && "Node exceeded or will exceed limit of modifications");

			m_Mods.emplace_back(fieldType, field, version);

			if (Modification::IsPointer(fieldType))
				SwicthReturnPointers(fieldType, this, field.Pointer);

			if (m_Mods.size() == ModificationsLimit)
				CreateNewNode(version);
		}

		inline void CreateNewNode(int version)
		{
			Node* newNode = new Node(Data, GetColor(), Left(), Right(), Parent(), m_ReturnLeft, m_ReturnRight, m_ReturnParent);

			if (m_ReturnLeft)
				m_ReturnLeft->SetParent(newNode, version);
			if (m_ReturnRight)
				m_ReturnRight->SetParent(newNode, version);
			if (m_ReturnParent)
			{
				if (this->IsLeftChildOf(m_ReturnParent))
					m_ReturnParent->SetLeft(newNode, version);
				else if (this->IsRightChildOf(m_ReturnParent))
					m_ReturnParent->SetRight(newNode, version);
			}

			m_Next = newNode;
		}

		inline void SwicthReturnPointers(Modification::Type whichPointer, Node* pointer, Node* pointee)
		{
			switch (whichPointer)
			{
			case Modification::Type::Left:
				pointer->m_ReturnLeft = pointee;
				if (pointee)
					pointee->m_ReturnParent = pointer;
				break;
			case Modification::Type::Right:
				pointer->m_ReturnRight = pointee;
				if (pointee)
					pointee->m_ReturnParent = pointer;
				break;
			case Modification::Type::Parent:
				pointer->m_ReturnParent = pointee;
				if (pointee)
				{
					if (pointer->IsLeftChildOf(pointee))
						pointee->m_ReturnLeft = pointer;
					else if (pointer->IsRightChildOf(pointee))
						pointee->m_ReturnRight = pointer;
					else
						throw std::runtime_error("Node is not son of its parent, SwicthReturnPointers");
				}
				break;
			case Modification::Type::Color:
				throw std::runtime_error("Color is not a pointer, SwicthReturnPointers");
				break;
			}
		}

		inline int LatestVersion() const { return m_Mods.back().Version; }

	public:
		int Data;

	public:
		Node* m_Left{ nullptr };
		Node* m_Right{ nullptr };
		Node* m_Parent{ nullptr };

		Color m_Color;

		std::vector<Modification> m_Mods;

		Node* m_ReturnLeft{ nullptr };
		Node* m_ReturnRight{ nullptr };
		Node* m_ReturnParent{ nullptr };

		Node* m_Next{ nullptr };
	};

	class Nil : public Node
	{
	public:
		Nil() : Node(-1, Color::Black) {}

		inline bool IsNil() const override { return true; }
	};

	static constexpr int MaxOperations = 100;

	inline Node* Search(int data, int version = INT32_MAX) const
	{
		Node* current = Root(version);
		while (current && current->Data != data)
			current = data < current->Data ? current->Left(version) : current->Right(version);

		return current;
	}
	
	inline int Successor(int data, int version = INT32_MAX) const
	{
		Node* node = Search(data, version);
		if (!node)
			return INT32_MAX;

		Node* right = node->Right(version);
		if (!right)
			return node->Data;

		return Minimun(right, version)->Data;
	}

	inline void Print(int version = INT32_MAX) const
	{
		Node* root = Root(version);
		if (!root)
			return;
		std::cout << root->Data << (root->IsBlack(version) ? " (B)" : " (R)") << std::endl;

		PrintHelper(root->Right(version), 8, version, false);
		PrintHelper(root->Left(version), 8, version, true);
	}

	inline void FPrint(int version, std::ostream& outFileStream) const
	{
		FPrintHelper(Root(version), version, 0, outFileStream);
		outFileStream << '\n';
	}

	inline int CurrentVersion() const { return m_CurrentVersion; }

	inline void Insert(int key)
	{
		Node* current = Root();
		Node* parent = nullptr;

		m_CurrentVersion++;

		while (current)
		{
			assert(key != current->Data && "this Red-Black-Tree do not suport repeated keys.");

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

	inline void Remove(int key)
	{
		Node* node = Search(key);
		if (!node)
			return;

		m_CurrentVersion++;

		Node* movedUpNode;
		bool deletedNodeWasBlack;
		if (!node->Left() || !node->Right())
		{
			deletedNodeWasBlack = node->IsBlack();
			movedUpNode = RemoveNodeWithZeroOrOneChild(node);
		}
		else
		{
			Node* successor = Minimun(node->Right());
			if (!successor->Right())
			{
				successor->SetRight(new Nil(), m_CurrentVersion);
				successor->Right()->SetParent(successor, m_CurrentVersion);
			}

			Node* oldSuccessorRight = successor->Right();
			if (!successor->IsRightChildOf(node))
			{
				SwapParentsChild(successor->Parent(), successor, successor->Right());
				successor->SetRight(node->Right(), m_CurrentVersion);
				successor->Right()->SetParent(successor, m_CurrentVersion);
			}

			SwapParentsChild(node->Parent(), node, successor);
			successor->SetLeft(node->Left(), m_CurrentVersion);
			successor->Left()->SetParent(successor, m_CurrentVersion);

			deletedNodeWasBlack = successor->IsBlack();
			successor->CopyColor(node, m_CurrentVersion);

			movedUpNode = oldSuccessorRight;
		}
		
		if (deletedNodeWasBlack)
			RemoveFixup(movedUpNode);

		if (movedUpNode && movedUpNode->IsNil())
			SwapParentsChild(movedUpNode->Parent(), movedUpNode, nullptr);
	}

private:
	inline void SwapParentsChild(Node* parent, Node* oldChild, Node* newChild)
	{
		if (!parent)
			SetRoot(newChild, m_CurrentVersion);
		else if (oldChild->IsLeftChildOf(parent))
			parent->SetLeft(newChild, m_CurrentVersion);
		else if (oldChild->IsRightChildOf(parent))
			parent->SetRight(newChild, m_CurrentVersion);
		else
			throw std::runtime_error("Node is not child of its parent, SwapParentsChild");

		if (newChild)
			newChild->SetParent(parent, m_CurrentVersion);
	}

	inline void RotateRight(Node* node)
	{
		Node* parent = node->Parent();
		Node* left = node->Left();
		Node* leftRight = left->Right();

		node->SetLeft(leftRight, m_CurrentVersion);
		if (leftRight)
			leftRight->SetParent(node, m_CurrentVersion);

		left->SetRight(node, m_CurrentVersion);
		node->SetParent(left, m_CurrentVersion);

		SwapParentsChild(parent, node, left);
	}

	inline void RotateLeft(Node* node)
	{
		Node* parent = node->Parent();
		Node* right = node->Right();
		Node* rightLeft = right->Left();

		node->SetRight(rightLeft, m_CurrentVersion);
		if (rightLeft)
			rightLeft->SetParent(node, m_CurrentVersion);

		right->SetLeft(node, m_CurrentVersion);
		node->SetParent(right, m_CurrentVersion);

		SwapParentsChild(parent, node, right);
	}

	inline void InsertFixup(Node* node)
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

	inline void RemoveFixup(Node* node)
	{
		if (Root()->Equals(node))
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
				RemoveFixup(node->Parent());
		}
		else
			HandleBlackSiblingWithAtLeastOneRedChild(node, sibling);
	}

	inline void HandleRedSibling(Node* node, Node* sibling)
	{
		sibling->SetBlack(m_CurrentVersion);
		node->Parent()->SetRed(m_CurrentVersion);

		if (node->IsLeftChildOf(node->Parent()))
			RotateLeft(node->Parent());
		else
			RotateRight(node->Parent());
	}

	inline void HandleBlackSiblingWithAtLeastOneRedChild(Node* node, Node* sibling)
	{
		bool nodeIsLeftChild = node->IsLeftChildOf(node->Parent());

		if (nodeIsLeftChild && NodeIsBlack(sibling->Right()))
		{
			sibling->Left()->SetBlack(m_CurrentVersion);
			sibling->SetRed(m_CurrentVersion);
			RotateRight(sibling);
			sibling = node->Parent()->Right();
		}
		else if (!nodeIsLeftChild && NodeIsBlack(sibling->Left()))
		{
			sibling->Right()->SetBlack(m_CurrentVersion);
			sibling->SetRed(m_CurrentVersion);
			RotateLeft(sibling);
			sibling = node->Parent()->Left();
		}

		sibling->CopyColor(node->Parent(), m_CurrentVersion);
		node->Parent()->SetBlack(m_CurrentVersion);
		if (nodeIsLeftChild)
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

	inline Node* RemoveNodeWithZeroOrOneChild(Node* node)
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

	inline Node* Minimun(Node* node, int version = INT32_MAX) const
	{
		while (node->Left(version))
			node = node->Left(version);

		return node;
	}

	inline bool NodeIsBlack(Node* node, int version = INT32_MAX) const
	{
		return !node || node->IsBlack(version);
	}

	inline void PrintHelper(Node* node, int ident, int version, bool isLeftChild) const
	{
		if (!node)
			return;

		std::cout << std::string(ident, ' ')
			<< node->Data
			<< (isLeftChild ? "L" : "R")
			<< (node->IsBlack(version) ? " (B)" : " (R)") << std::endl;
		PrintHelper(node->Right(version), ident + 8, version, false);
		PrintHelper(node->Left(version), ident + 8, version, true);
	}
	void FPrintHelper(Node* node, int version, int depth, std::ostream& outFileStream) const
	{
		if (!node)
			return;
		FPrintHelper(node->Left(version), version, depth + 1, outFileStream);
		outFileStream << node->Data << ',' << depth << ',' << (node->IsBlack(version) ? "N" : "R") << ' ';
		FPrintHelper(node->Right(version), version, depth + 1, outFileStream);
	}

	struct VersionedRoot
	{
		Node* Root;
		int Version;

		inline VersionedRoot(Node* root, int version) : Root(root), Version(version) {}
	};
	inline void SetRoot(Node* root, int version)
	{
		m_Roots.emplace_back(root, version);
	}

	inline Node* Root(int version = INT32_MAX) const
	{
		for (auto versionedRoot = m_Roots.rbegin(); versionedRoot != m_Roots.rend(); versionedRoot++)
		{
			if (versionedRoot->Version <= version)
				return versionedRoot->Root;
		}

		return nullptr;
	}

private:
	int m_CurrentVersion{ 0 };
	std::vector<VersionedRoot> m_Roots;
};