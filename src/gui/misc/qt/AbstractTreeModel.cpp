/*
 * Copyright 2012, Ingo Weinhold <ingo_weinhold@gmx.de>.
 * All rights reserved.
 */


#include "AbstractTreeModel.h"

#include <QSet>


// #pragma mark - AbstractTreeModel


AbstractTreeModel::AbstractTreeModel(QObject* parent)
	:
	QAbstractItemModel(parent),
	fRootNode(new RootNode),
	fDragSortable(false),
	fCheckable(false),
	fCheckNodesMode(CHECK_NODES_MODE_INDIVIDUALLY)
{
}


AbstractTreeModel::~AbstractTreeModel()
{
	delete fRootNode;
}


bool
AbstractTreeModel::IsDragSortable() const
{
	return fDragSortable;
}


void
AbstractTreeModel::SetDragSortable(bool dragSortable)
{
	fDragSortable = dragSortable;
}

void
AbstractTreeModel::SetCheckable(bool checkable)
{
	fCheckable = checkable;
}


AbstractTreeModel::CheckNodesMode
AbstractTreeModel::GetCheckNodesMode() const
{
	return fCheckNodesMode;
}


void
AbstractTreeModel::SetCheckNodesMode(CheckNodesMode mode)
{
	fCheckNodesMode = mode;
}


void
AbstractTreeModel::SetHeaderName(int section, Qt::Orientation orientation,
	const QString& name)
{
	if (name.isEmpty())
		fHeaderNames.remove(std::make_pair(section, orientation));
	else
		fHeaderNames.insert(std::make_pair(section, orientation), name);

	headerDataChanged(orientation, section, section);
}


void
AbstractTreeModel::clear(bool notify)
{
	if (notify)
		beginResetModel();

	fRootNode->MakeEmpty();

	if (notify)
		endResetModel();
}


QModelIndex
AbstractTreeModel::index(int row, int column, const QModelIndex& parent) const
{
	Node* parentNode = GetNode(parent);
	if (Node* childNode = parentNode->ChildAt(row))
		return createIndex(row, column, (void*)childNode);

	return QModelIndex();
}


QModelIndex
AbstractTreeModel::parent(const QModelIndex& index) const
{
	Node* node = GetNode(index);
	Node* parent = node->Parent();
	if (parent != NULL && parent != fRootNode)
		return createIndex(parent->Index(), 0, (void*)parent);

	return QModelIndex();
}


int
AbstractTreeModel::rowCount(const QModelIndex& i_parent) const
{
	const Node* node = GetNode(i_parent);
	return node->CountChildren();
}


int
AbstractTreeModel::columnCount(const QModelIndex& i_parent) const
{
	return GetNode(i_parent)->CountColumns();
}


Qt::ItemFlags
AbstractTreeModel::flags(const QModelIndex& index) const
{
	Node* node = GetNode(index);
	Qt::ItemFlags flags = NodeFlags(node, index.column());

	// Clear enabled flags, if check mode is
	// CHECK_NODES_MODE_RECURSIVELY_AND_DISABLE and a ancestor node is checked.
	if (flags.testFlag(Qt::ItemIsEnabled)
		&& fCheckNodesMode == CHECK_NODES_MODE_RECURSIVELY_AND_DISABLE
		&& node->Parent() != NULL && IsNodeChecked(node->Parent())) {
		flags &= ~Qt::ItemFlags(Qt::ItemIsEnabled);
	}

	return flags;
}


QVariant
AbstractTreeModel::data(const QModelIndex& index, int role) const
{
	Node* node = GetNode(index);

	if (role == Qt::CheckStateRole
		&& flags(index).testFlag(Qt::ItemIsUserCheckable)) {
		return IsNodeChecked(node) ? Qt::Checked : Qt::Unchecked;
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
		return node->Data(index.column());

	return QVariant();
}


bool
AbstractTreeModel::setData(const QModelIndex& index, const QVariant& value,
	int role)
{
	Node* node = GetNode(index);

	if (role == Qt::CheckStateRole) {
		bool checked = value.toInt() == Qt::Checked;
		if (checked != IsNodeChecked(node))
		{
			SetNodeChecked(node, checked);
			dataChanged(index, index);
		}

		return true;
	}

	if (role == Qt::EditRole)
		return node->SetData(index.column(), value);

	return false;
}


QVariant
AbstractTreeModel::headerData(int section, Qt::Orientation orientation,
	int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	QString name = fHeaderNames.value(std::make_pair(section, orientation),
		QString());
	if (name.isEmpty())
		name = DefaultHeaderName(section, orientation);

	return name.isEmpty() ? QVariant() : QVariant(name);
}


Qt::DropActions
AbstractTreeModel::supportedDragActions() const
{
	return fDragSortable ? Qt::MoveAction : Qt::DropActions(0);
}


Qt::DropActions
AbstractTreeModel::supportedDropActions() const
{
	return fDragSortable ? Qt::MoveAction : Qt::DropActions(0);
}


QStringList
AbstractTreeModel::mimeTypes() const
{
	if (!fDragSortable)
		return QStringList();

	QStringList types;
	types += QString::fromAscii(NodeMimeData::MimeString());
	return types;
}


QMimeData*
AbstractTreeModel::mimeData(const QModelIndexList& indexes) const
{
	if (!fDragSortable)
		return NULL;

	NodeMimeData* mimeData = new NodeMimeData;
	QSet<Node*> nodes;

	foreach (QModelIndex index, indexes) {
		Node* node = GetNode(index);
		if (!nodes.contains(node)) {
			if (flags(ModelIndexOfNode(node)).testFlag(Qt::ItemIsDragEnabled))
				mimeData->AddNode(node);

			nodes.insert(node);
		}
	}

	return mimeData;
}


bool
AbstractTreeModel::dropMimeData(const QMimeData* _data,
	Qt::DropAction /*action*/, int row, int /*column*/,
	const QModelIndex& parent)
{
	const NodeMimeData* data = dynamic_cast<const NodeMimeData*>(_data);
	if (data == NULL)
		return false;

	return DropNodes(GetNode(parent), row, data->Nodes());
}


Qt::ItemFlags
AbstractTreeModel::NodeFlags(Node* node, int column) const
{
	return node->Flags(column);
}


bool
AbstractTreeModel::IsNodeChecked(Node* node) const
{
	if (node->IsChecked())
		return true;

	switch (fCheckNodesMode) {
		case CHECK_NODES_MODE_RECURSIVELY:
		case CHECK_NODES_MODE_RECURSIVELY_AND_DISABLE:
			return node->Parent() != NULL && IsNodeChecked(node->Parent());

		case CHECK_NODES_MODE_INDIVIDUALLY:
		default:
			return false;
	}
}


bool
AbstractTreeModel::SetNodeChecked(Node* i_node, bool checked)
{
	i_node->SetChecked(checked);

	switch (fCheckNodesMode) {
		case CHECK_NODES_MODE_INDIVIDUALLY:
		default:
			break;

		case CHECK_NODES_MODE_RECURSIVELY:
		case CHECK_NODES_MODE_RECURSIVELY_AND_DISABLE:
		{
			int childCount = i_node->CountChildren();
			for (int i = 0; i < childCount; i ++)
			{
				Node* child = i_node->ChildAt(i);
				if (SetNodeChecked(child, checked))
				{
					QModelIndex childIndex = ModelIndexOfNode(child);
					dataChanged(childIndex, childIndex);
				}
			}
			break;
		}
	}

	return true;
}


QString
AbstractTreeModel::DefaultHeaderName(int /*section*/,
	Qt::Orientation /*orientation*/) const
{
	return QString();
}


bool
AbstractTreeModel::DropNodes(Node* /*parentNode*/, int /*row*/,
	const QList<Node*>& /*nodes*/)
{
	return false;
}


void
AbstractTreeModel::InsertNode(Node* parent, Node* node, bool notify)
{
	int rowIndex = parent->FindChildInsertionIndex(node);
	if (notify)
		beginInsertRows(ModelIndexOfNode(parent), rowIndex, rowIndex);

	parent->InsertChild(rowIndex, node);

	if (notify)
		endInsertRows();
}


void
AbstractTreeModel::ReparentNode(Node* node, Node* oldParentNode,
	Node* newParentNode)
{
	int newRowIndex = newParentNode->FindChildInsertionIndex(node);
	int oldRowIndex = node->Index();

	beginMoveRows(ModelIndexOfNode(oldParentNode), oldRowIndex, oldRowIndex,
		ModelIndexOfNode(newParentNode), newRowIndex);
	newParentNode->AppendChild(oldParentNode->RemoveChild(oldRowIndex));
	endMoveRows();
}


AbstractTreeModel::Node*
AbstractTreeModel::GetNode(const QModelIndex& index) const
{
	return index.isValid()
		? static_cast<Node*>(index.internalPointer()) : fRootNode;
}


QModelIndex
AbstractTreeModel::ModelIndexOfNode(const Node* node, int column) const
{
	if (node == NULL || node == fRootNode)
		return QModelIndex();

	return createIndex(node->Index(), column, (void*)node);
}


QList<AbstractTreeModel::Node*>
AbstractTreeModel::CheckedNodes() const
{
	QList<Node*> nodes;
	GetCheckedNodes(fRootNode, nodes);
	return nodes;
}


void
AbstractTreeModel::GetCheckedNodes(Node* node, QList<Node*>& _nodes) const
{
	if (IsNodeChecked(node))
		_nodes.append(node);

	int childCount = node->CountChildren();
	for (int i = 0; i < childCount; i ++)
		GetCheckedNodes(node->ChildAt(i), _nodes);
}


// #pragma mark - AbstractTreeModel::SortObject


AbstractTreeModel::SortObject::SortObject(int priority, const void* object)
	:
	fPriority(priority),
	fObject(object)
{
}


AbstractTreeModel::SortObject::SortObject(const void* object)
	:
	fPriority(0),
	fObject(object)
{
}


int
AbstractTreeModel::SortObject::Priority() const
{
	return fPriority;
}


const void*
AbstractTreeModel::SortObject::Object() const
{
	return fObject;
}


bool
AbstractTreeModel::SortObject::operator <(const SortObject& other)
{
	return fPriority > other.fPriority ||
		(fPriority == other.fPriority && fObject < other.fObject);
}


// #pragma mark - AbstractTreeModel::Node


AbstractTreeModel::Node::Node()
	:
	fParent(NULL),
	fChecked(false)
{
}


AbstractTreeModel::Node::~Node()
{
}


int
AbstractTreeModel::Node::Index() const
{
	return fParent != NULL ? fParent->IndexOfChild(this) : 0;
}


AbstractTreeModel::Node*
AbstractTreeModel::Node::Parent() const
{
	return fParent;
}


void
AbstractTreeModel::Node::SetParent(Node* parent)
{
	fParent = parent;
}


bool
AbstractTreeModel::Node::IsChecked() const
{
	return fChecked;
}


void
AbstractTreeModel::Node::SetChecked(bool checked)
{
	fChecked = checked;
}


int
AbstractTreeModel::Node::CountChildren() const
{
	return 0;
}


AbstractTreeModel::Node*
AbstractTreeModel::Node::ChildAt(int /*index*/) const
{
	return NULL;
}


int
AbstractTreeModel::Node::IndexOfChild(const Node* /*child*/) const
{
	return -1;
}


void
AbstractTreeModel::Node::AppendChild(Node* child)
{
	delete child;
}


void
AbstractTreeModel::Node::InsertChild(int /*index*/, Node* child)
{
	delete child;
}


AbstractTreeModel::Node*
AbstractTreeModel::Node::RemoveChild(int /*index*/)
{
	return NULL;
}


int
AbstractTreeModel::Node::FindChildInsertionIndex(Node* /*child*/) const
{
	return -1;
}


int
AbstractTreeModel::Node::FindChangedChildIndex(Node* /*child*/) const
{
	return -1;
}


int
AbstractTreeModel::Node::CountColumns() const
{
	return 1;
}


Qt::ItemFlags
AbstractTreeModel::Node::Flags(int /*column*/) const
{
	return Qt::ItemIsEnabled;
}


QVariant
AbstractTreeModel::Node::Data(int /*column*/) const
{
	return QVariant();
}


bool
AbstractTreeModel::Node::SetData(int /*column*/, const QVariant& /*value*/)
{
	return false;
}


AbstractTreeModel::SortObject
AbstractTreeModel::Node::GetSortObject() const
{
	return SortObject(this);
}


bool
AbstractTreeModel::Node::Less(const Node* other) const
{
	return GetSortObject() < other->GetSortObject();
}


bool
AbstractTreeModel::Node::StaticLess(const Node* a, const Node* b)
{
	return a->Less(b);
}


// #pragma mark - AbstractTreeModel::ContainerNode


AbstractTreeModel::ContainerNode::~ContainerNode()
{
	MakeEmpty();
}


void
AbstractTreeModel::ContainerNode::MakeEmpty()
{
	foreach (Node* child, fChildren)
		delete child;
	fChildren.clear();
}


int
AbstractTreeModel::ContainerNode::CountChildren() const
{
	return fChildren.count();
}


AbstractTreeModel::Node*
AbstractTreeModel::ContainerNode::ChildAt(int index) const
{
	return index >= 0 && index < fChildren.count() ? fChildren.at(index) : NULL;
}


int
AbstractTreeModel::ContainerNode::IndexOfChild(const Node* child) const
{
	return fChildren.indexOf(const_cast<Node*>(child));
}


void
AbstractTreeModel::ContainerNode::AppendChild(Node* child)
{
	InsertChild(ContainerNode::FindChildInsertionIndex(child), child);
}


void
AbstractTreeModel::ContainerNode::InsertChild(int index, Node* child)
{
	fChildren.insert(index, child);
	child->SetParent(this);
}


AbstractTreeModel::Node*
AbstractTreeModel::ContainerNode::RemoveChild(int index)
{
	if (index < 0 || index >= fChildren.count())
	{
		return NULL;
	}

	Node* node = fChildren.takeAt(index);
	if (node != NULL)
	{
		node->SetParent(NULL);
	}

	return node;
}


int
AbstractTreeModel::ContainerNode::FindChildInsertionIndex(Node* child) const
{
	return qLowerBound(fChildren.begin(), fChildren.end(), child,
			&Node::StaticLess)
		- fChildren.begin();
}


int
AbstractTreeModel::ContainerNode::FindChangedChildIndex(Node* child) const
{
	int oldIndex = IndexOfChild(child);
	int newIndex = oldIndex;
	while (newIndex > 0 && StaticLess(child, fChildren.at(newIndex - 1)))
		newIndex --;

	if (newIndex == oldIndex) {
		while (newIndex + 1 < fChildren.count()
			&& StaticLess(fChildren.at(newIndex + 1), child)) {
			newIndex ++;
		}

		if (newIndex > oldIndex)
			newIndex ++;
	}

	return newIndex;
}


// #pragma mark - AbstractTreeModel::NodeMimeData


AbstractTreeModel::NodeMimeData::NodeMimeData()
{
}


void
AbstractTreeModel::NodeMimeData::AddNode(Node* node)
{
	fNodes.append(node);
}


const QList<AbstractTreeModel::Node*>&
AbstractTreeModel::NodeMimeData::Nodes() const
{
	return fNodes;
}


QStringList
AbstractTreeModel::NodeMimeData::formats() const
{
	QStringList types;
	types += QString::fromAscii(NodeMimeData::MimeString());
	return types;
}


bool
AbstractTreeModel::NodeMimeData::hasFormat(const QString& mimeType) const
{
	return mimeType == QString::fromAscii(MimeString());
}


const char*
AbstractTreeModel::NodeMimeData::MimeString()
{
	return "application/x-vnd.yellowbites-tree-model-base-node";
}
