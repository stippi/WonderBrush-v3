#ifndef OBJECT_TREE_VIEW_H
#define OBJECT_TREE_VIEW_H

#include "ColumnTreeView.h"
#include "EasyColumnTreeItem.h"


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
								ObjectTreeView(BRect frame);
	virtual						~ObjectTreeView();

	virtual	void				MouseDown(BPoint where);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				MessageReceived(BMessage* message);

private:
			void				_HandleRenameSelectedItem();
			void				_HandleRenameItem(int32 index);
			void				_HandleRenameObject(BMessage* message);
};


#endif // OBJECT_TREE_VIEW_H
