// ColumnTreeItemHandle.cpp

#include "ColumnTreeItemHandle.h"

// constructor
ColumnTreeItemHandle::ColumnTreeItemHandle()
	: fModel(NULL)
{
}

// destructor
ColumnTreeItemHandle::~ColumnTreeItemHandle()
{
}

// SetModel
void
ColumnTreeItemHandle::SetModel(ColumnTreeModel* model)
{
	fModel = model;
}

