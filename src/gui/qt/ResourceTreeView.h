/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>.
 * Copyright 2012, Ingo Weinhold <ingo_weinhold@gmx.de>.
 * All rights reserved.
 */
#ifndef RESOURCE_TREE_VIEW_H
#define RESOURCE_TREE_VIEW_H


#include <View.h>

#include <QTreeView>

#include "Document.h"
#include "LayerObserver.h"
#include "ListenerAdapter.h"
#include "Selection.h"


class ResourceTreeView : public BView, public Selection::Controller,
		public Selection::Listener {
	Q_OBJECT

public:
								ResourceTreeView(Document* document,
									Selection* selection);
	virtual						~ResourceTreeView();

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual	void				MouseDown(BPoint where);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				MessageReceived(BMessage* message);

	// Selection::Listener interface
	virtual	void				ObjectSelected(const Selectable& object,
									const Selection::Controller* controller);
	virtual	void				ObjectDeselected(const Selectable& object,
									const Selection::Controller* controller);

private:
			struct TreeModel;

private:
			void				_Init();

			void				_HandleRenameSelectedItem();

			void				_ObjectAdded(const ResourceList* list,
									BaseObject* object, int32 index);
			void				_ObjectRemoved(const ResourceList* list,
									BaseObject* object, int32 index);
			void				_ObjectChanged(const ResourceList* list,
									BaseObject* object, int32 index);
			void				_ObjectSelected(BaseObject* object,
									bool selected);

			void				_AddItems(const ResourceList& list,
									const QModelIndex& parentNodeIndex);
			void				_ExpandItem(const QModelIndex& index);

private slots:
			void				_SelectionChanged();

private:
			QTreeView*			fTree;
			TreeModel*			fTreeModel;
			Document*			fDocument;
			Selection*			fSelection;
			ResourceList::Observer fResourceObserver;
			bool				fIgnoreSelectionChanged;
};


#endif // RESOURCE_TREE_VIEW_H
