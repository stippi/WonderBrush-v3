/*
 * Copyright 2009-2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef OBJECT_TREE_VIEW_H
#define OBJECT_TREE_VIEW_H

#include "ColumnTreeView.h"
#include "EasyColumnTreeItem.h"
#include "LayerObserver.h"
#include "ListenerAdapter.h"
#include "Selection.h"
#include "ShapeObserver.h"


class BaseObject;
class Document;
class EditContext;
class Shape;


class ObjectColumnTreeItem : public EasyColumnTreeItem, private Listener {
public:
			BaseObject*			object;

								ObjectColumnTreeItem(float height,
									BaseObject* object);
	virtual						~ObjectColumnTreeItem();

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

			void				Update();
};


class ObjectTreeView : public ColumnTreeView, public Selection::Controller,
	public Selection::Listener {
public:
								ObjectTreeView(BRect frame, Document* document,
									Selection* selection,
									EditContext& editContext);
#ifdef __HAIKU__
								ObjectTreeView(Document* document,
									Selection* selection,
									EditContext& editContext);
#endif
	virtual						~ObjectTreeView();

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual	void				MouseDown(BPoint where);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				MessageReceived(BMessage* message);

	// ColumnTreeView interface
	virtual	bool				InitiateDrag(BPoint point, int32 index,
									bool wasSelected,
									BMessage* _message = NULL);
	virtual	bool				GetDropInfo(BPoint point,
									const BMessage& dragMessage,
									ColumnTreeItem** _super,
									int32* _subItemIndex, int32* _itemIndex);
	virtual	void				HandleDrop(const BMessage& dragMessage,
									ColumnTreeItem* super, int32 subItemIndex,
									int32 itemIndex);

	virtual	void				SelectionChanged();
	virtual void				ItemSelectedTwice(int32 index);

	// Selection::Listener interface
	virtual	void				ObjectSelected(const Selectable& object,
									const Selection::Controller* controller);
	virtual	void				ObjectDeselected(const Selectable& object,
									const Selection::Controller* controller);

	// ObjectTreeView
			void				SelectItem(BaseObject* object);

private:
			void				_Init();

			void				_HandleRenameSelectedItem();
			void				_HandleRenameItem(int32 index);
			void				_HandleRenameObject(BMessage* message);

			void				_ObjectAdded(Layer* layer, Object* object,
									int32 index);
			void				_ObjectRemoved(Layer* layer, Object* object,
									int32 index);
			void				_ObjectChanged(Layer* layer, Object* object,
									int32 index);
			
			void				_ObjectSelected(BaseObject* object,
									bool selected);

			void				_PathAdded(Shape* shape, PathInstance* path,
									int32 index);
			void				_PathRemoved(Shape* shape, PathInstance* path,
									int32 index);

			ObjectColumnTreeItem* _FindLayerTreeViewItem(
									const BaseObject* object);

			void				_RecursiveAddItems(Layer* layer,
									ObjectColumnTreeItem* item);
			void				_RecursiveRemoveItems(Layer* layer,
									ObjectColumnTreeItem* item);
			void				_AddPathItems(Shape* shape,
									ObjectColumnTreeItem* layerItem);

			void				_DropObjects(const BMessage& dragMessage,
									int32 count, Layer* insertionLayer,
									int32 subItemIndex, int32 itemIndex);
			void				_DropPaths(const BMessage& dragMessage,
									int32 count, Shape* insertionShape,
									int32 subItemIndex, int32 itemIndex);

private:
			Document*			fDocument;
			Selection*			fSelection;
			EditContext&		fEditContext;
			LayerObserver		fLayerObserver;
			ShapeObserver		fShapeObserver;
			bool				fIgnoreSelectionChanged;
};


#endif // OBJECT_TREE_VIEW_H
