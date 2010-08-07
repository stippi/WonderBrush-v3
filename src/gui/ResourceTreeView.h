/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef RESOURCE_TREE_VIEW_H
#define RESOURCE_TREE_VIEW_H

#include "ColumnTreeView.h"
#include "Document.h"
#include "EasyColumnTreeItem.h"
#include "LayerObserver.h"
#include "ListenerAdapter.h"
#include "Selection.h"

class ResourceColumnTreeItem : public EasyColumnTreeItem {
public:
			BaseObject*			object;

								ResourceColumnTreeItem(float height,
									BaseObject* object);
	virtual						~ResourceColumnTreeItem();

			void				Update();
};


class ResourceTreeView : public ColumnTreeView, public Selection::Controller,
	public Selection::Listener {
public:
								ResourceTreeView(BRect frame, Document* document,
									Selection* selection);
#ifdef __HAIKU__
								ResourceTreeView(Document* document,
									Selection* selection);
#endif
	virtual						~ResourceTreeView();

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual	void				MouseDown(BPoint where);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				MessageReceived(BMessage* message);

	// ColumnTreeView interface
	virtual	void				SelectionChanged();

	// Selection::Listener interface
	virtual	void				ObjectSelected(const Selectable& object,
									const Selection::Controller* controller);
	virtual	void				ObjectDeselected(const Selectable& object,
									const Selection::Controller* controller);

private:
			void				_Init();

			void				_HandleRenameSelectedItem();
			void				_HandleRenameItem(int32 index);
			void				_HandleRenameObject(BMessage* message);

			void				_ObjectAdded(const ResourceList* list,
									BaseObject* object, int32 index);
			void				_ObjectRemoved(const ResourceList* list,
									BaseObject* object, int32 index);
			void				_ObjectChanged(const ResourceList* list,
									BaseObject* object, int32 index);
			void				_ObjectSelected(BaseObject* object,
									bool selected);

			ResourceColumnTreeItem* _FindLayerTreeViewItem(
									const BaseObject* object);

			void				_AddItems(const ResourceList& list,
									ResourceColumnTreeItem* item);

private:
			Document*			fDocument;
			Selection*			fSelection;
			ResourceList::Observer fResourceObserver;
			bool				fIgnoreSelectionChanged;
};


#endif // RESOURCE_TREE_VIEW_H
