#ifndef WINDOW_H
#define WINDOW_H

#include <Window.h>

#include "LayerObserver.h"
#include "ListenerAdapter.h"

class CanvasView;
class ColumnTreeModel;
class ColumnTreeView;
class Document;
class Layer;
class ObjectColumnTreeItem;
class PickToolState;
class RenderManager;

class Window : public BWindow {
 public:
								Window(BRect frame, const char* title,
									Document* document, Layer* layer);
	virtual						~Window();

	// BWindow interface
	virtual	bool				QuitRequested();
	virtual	void				MessageReceived(BMessage* message);

	// Window
			void				SetDocument(Document* document);

 private:
			void				_ObjectChanged(const Notifier* object);

			void				_ObjectAdded(Layer* layer, Object* object,
									int32 index);
			void				_ObjectRemoved(Layer* layer, Object* object,
									int32 index);
			void				_ObjectChanged(Layer* layer, Object* object,
									int32 index);

			ObjectColumnTreeItem* _FindLayerTreeViewItem(const Object* object);

			void				_RecursiveAddItems(Layer* layer,
									ObjectColumnTreeItem* item);
			void				_RecursiveRemoveItems(Layer* layer,
									ObjectColumnTreeItem* item);

			void				_RecursiveAddListener(Layer* layer);
			void				_RecursiveRemoveListener(Layer* layer);


			CanvasView*			fView;
			Document*			fDocument;
			PickToolState*		fPickState;
			RenderManager*		fRenderManager;
			ListenerAdapter		fCommandStackListener;

			BMenuItem*			fUndoMI;
			BMenuItem*			fRedoMI;

			ColumnTreeView*		fLayerTreeView;
			ColumnTreeModel*	fLayerTreeModel;
			LayerObserver		fLayerObserver;
};

#endif // WINDOW_H
