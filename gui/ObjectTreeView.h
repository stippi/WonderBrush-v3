#ifndef OBJECT_TREE_VIEW_H
#define OBJECT_TREE_VIEW_H

#include "ColumnTreeView.h"
#include "EasyColumnTreeItem.h"


class Document;
class Object;


class ObjectColumnTreeItem : public EasyColumnTreeItem {
public:
	Object*	object;

								ObjectColumnTreeItem(float height,
									Object* object);
	virtual						~ObjectColumnTreeItem();

			void				Update();
};


class ObjectTreeView : public ColumnTreeView {
public:
								ObjectTreeView(BRect frame, Document* document);
#ifdef __HAIKU__
								ObjectTreeView(Document* document);
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

private:
			void				_HandleRenameSelectedItem();
			void				_HandleRenameItem(int32 index);
			void				_HandleRenameObject(BMessage* message);

private:
			Document*			fDocument;
};


#endif // OBJECT_TREE_VIEW_H
