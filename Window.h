#ifndef WINDOW_H
#define WINDOW_H

#include <Window.h>

#include "LayerObserver.h"
#include "ListenerAdapter.h"

class CanvasView;
class ColumnTreeModel;
class Document;
class IconOptionsControl;
class Layer;
class ObjectColumnTreeItem;
class ObjectTreeView;
class PickToolState;
class RenderManager;
class Tool;

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

			void				AddTool(Tool* tool);

private:
			void				_InitTools();

			void				_ObjectChanged(const Notifier* object);

			CanvasView*			fView;
			Document*			fDocument;
			PickToolState*		fPickState;
			RenderManager*		fRenderManager;
			ListenerAdapter		fCommandStackListener;

			BMenuItem*			fUndoMI;
			BMenuItem*			fRedoMI;

			ObjectTreeView*		fLayerTreeView;
			ColumnTreeModel*	fLayerTreeModel;
			LayerObserver		fLayerObserver;
			IconOptionsControl*	fToolIconControl;

			BList				fTools;
};

#endif // WINDOW_H
