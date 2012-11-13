/*
 * Copyright 2012, Ingo Weinhold <ingo_weinhold@gmx.de>.
 * All rights reserved.
 */
#ifndef ABSTRACT_TREE_MODEL_H
#define ABSTRACT_TREE_MODEL_H


#include <QAbstractItemModel>
#include <QMimeData>
#include <QStringList>


class AbstractTreeModel : public QAbstractItemModel {
public:
			enum CheckNodesMode {
				CHECK_NODES_MODE_INDIVIDUALLY,
				CHECK_NODES_MODE_RECURSIVELY,
				CHECK_NODES_MODE_RECURSIVELY_AND_DISABLE
			};

public:
								AbstractTreeModel(QObject* parent = NULL);
	virtual						~AbstractTreeModel();

			bool				IsDragSortable() const;
			void				SetDragSortable(bool dragSortable);

			void				SetCheckable(bool checkable);

			CheckNodesMode		GetCheckNodesMode() const;
			void				SetCheckNodesMode(CheckNodesMode mode);

			void				SetHeaderName(int section,
									Qt::Orientation orientation,
									const QString& name);

	// QAbstractItemModel
	virtual	void				clear(bool notify);
	virtual	QModelIndex			index(int row, int column,
									const QModelIndex& parent = QModelIndex())
									const;
	virtual	QModelIndex			parent(const QModelIndex& index) const;
	virtual	int					rowCount(const QModelIndex& parent) const;
	virtual	int					columnCount(
									const QModelIndex& parent = QModelIndex())
									const;
	virtual	Qt::ItemFlags		flags(const QModelIndex& index) const;
	virtual	QVariant			data(const QModelIndex& index, int role) const;
	virtual	bool				setData(const QModelIndex& index,
									const QVariant& value,
									int role = Qt::EditRole);
	virtual	QVariant			headerData(int section,
									Qt::Orientation orientation,
									int role) const;

	virtual	Qt::DropActions		supportedDragActions() const;
	virtual	Qt::DropActions		supportedDropActions() const;
	virtual	QStringList			mimeTypes() const;
	virtual	QMimeData*			mimeData(const QModelIndexList& indexes) const;
	virtual	bool				dropMimeData(const QMimeData* data,
									Qt::DropAction action, int row, int column,
									const QModelIndex& parent);

protected:
			class SortObject;
			class Node;
			class ContainerNode;
			class RootNode;
			class NodeMimeData;

protected:
	virtual	Qt::ItemFlags		NodeFlags(Node* node, int column) const;

	virtual	bool				IsNodeChecked(Node* node) const;
	virtual	bool				SetNodeChecked(Node* node, bool checked);

	virtual	QString				DefaultHeaderName(int section,
									Qt::Orientation orientation) const;

	virtual	bool				DropNodes(Node* parentNode, int row,
									const QList<Node*>& nodes);

			void				InsertNode(Node* parent, Node* node,
									bool notify);
			void				ReparentNode(Node* node, Node* oldParentNode,
									Node* newParentNode);

			Node*				GetNode(const QModelIndex& index) const;
			QModelIndex			ModelIndexOfNode(const Node* node,
									int column = 0) const;

			QList<Node*>		CheckedNodes() const;
			void				GetCheckedNodes(Node* node,
									QList<Node*>& _nodes) const;

protected:
			RootNode*			fRootNode;
			bool				fDragSortable;
			bool				fCheckable;
			CheckNodesMode		fCheckNodesMode;
			QMap<std::pair<int, Qt::Orientation>, QString> fHeaderNames;
};


class AbstractTreeModel::SortObject {
public:
								SortObject(int priority, const void* object);
	explicit					SortObject(const void* object);

			int					Priority() const;
			const void*			Object() const;

			bool				operator<(const SortObject& other);

private:
			int					fPriority;
			const void*			fObject;
};


class AbstractTreeModel::Node {
public:
								Node();
	virtual						~Node();

			int					Index() const;
			Node*				Parent() const;
			void				SetParent(Node* parent);

			bool				IsChecked() const;
			void				SetChecked(bool checked);

	virtual	int					CountChildren() const;
	virtual	Node*				ChildAt(int index) const;
	virtual	int					IndexOfChild(const Node* child) const;

	virtual	void 				AppendChild(Node* child);
	virtual	void				InsertChild(int index, Node* child);
	virtual	Node*				RemoveChild(int index);

	virtual	int					FindChildInsertionIndex(Node* child) const;
	virtual	int					FindChangedChildIndex(Node* child) const;

	virtual	int					CountColumns() const;

	virtual	Qt::ItemFlags		Flags(int column) const;

	virtual	QVariant			Data(int column) const;
	virtual	bool				SetData(int column, const QVariant& value);

	virtual	SortObject			GetSortObject() const;

	virtual	bool				Less(const Node* other) const;
	static	bool				StaticLess(const Node* a, const Node* b);

private:
			Node*				fParent;
			bool				fChecked;
};


class AbstractTreeModel::ContainerNode : public Node {
public:
								~ContainerNode();

			void				MakeEmpty();

	virtual	int					CountChildren() const;
	virtual	Node*				ChildAt(int index) const;
	virtual	int					IndexOfChild(const Node* child) const;

	virtual	void				AppendChild(Node* child);
	virtual	void				InsertChild(int index, Node* child);
	virtual	Node*				RemoveChild(int index);

	virtual	int					FindChildInsertionIndex(Node* child) const;
	virtual	int					FindChangedChildIndex(Node* child) const;

private:
			QList<Node*>		fChildren;
};


class AbstractTreeModel::RootNode : public ContainerNode {
};


class AbstractTreeModel::NodeMimeData : public QMimeData {
public:
								NodeMimeData();

			void				AddNode(Node* node);
			const QList<Node*>&	Nodes() const;

	virtual	QStringList			formats() const;
	virtual	bool				hasFormat(const QString& mimeType) const;

	static	const char*			MimeString();

private:
			QList<Node*>		fNodes;
};


#endif // ABSTRACT_TREE_MODEL_H
