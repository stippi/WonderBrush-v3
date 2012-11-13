/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>.
 * Copyright 2012, Ingo Weinhold <ingo_weinhold@gmx.de>.
 * All rights reserved.
 */
#ifndef OBJECT_TREE_VIEW_H
#define OBJECT_TREE_VIEW_H


#include <View.h>

#include <QTreeView>

#include "LayerObserver.h"
#include "ListenerAdapter.h"
#include "Selection.h"


class Document;
class Object;


class ObjectTreeView : public BView, public Selection::Controller,
		public Selection::Listener {
	Q_OBJECT

public:
								ObjectTreeView(Document* document,
									Selection* selection);
	virtual						~ObjectTreeView();

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

	// ObjectTreeView
			void				SelectItem(Object* object);

private:
			struct TreeModel;

private:
			void				_HandleRenameSelectedItem();

			void				_ObjectAdded(Layer* layer, Object* object,
									int32 index);
			void				_ObjectRemoved(Layer* layer, Object* object,
									int32 index);
			void				_ObjectChanged(Layer* layer, Object* object,
									int32 index);
			void				_ObjectSelected(Object* object,
									bool selected);

			void				_RecursiveAddItems(Layer* layer,
									const QModelIndex& layerNodeIndex);
			void				_ExpandItem(const QModelIndex& index);

private slots:
			void				_SelectionChanged();

private:
			QTreeView*			fTree;
			TreeModel*			fTreeModel;
			Document*			fDocument;
			Selection*			fSelection;
			LayerObserver		fLayerObserver;
			bool				fIgnoreSelectionChanged;
};


#endif // OBJECT_TREE_VIEW_H
