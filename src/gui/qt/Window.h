#ifndef WINDOW_H
#define WINDOW_H


#include <Window.h>

#include "CurrentColor.h"
#include "ListenerAdapter.h"
#include "Selection.h"


class CanvasView;
class Document;
class IconButton;
class Layer;
class ObjectTreeView;
class RenderManager;
class ResourceTreeView;
class SwatchGroup;
class Tool;


namespace Ui {
class Window;
}

class Window : public BWindow
{
	Q_OBJECT
	
public:
	explicit					Window(BRect frame, const char* title,
									Document* document, Layer* layer);
								~Window();

	virtual	void				MessageReceived(BMessage* message);

			void				AddTool(Tool* tool);

			status_t			StoreSettings(BMessage& settings) const;
			void				RestoreSettings(const BMessage& settings);

			void				Show();

private:
			void				_InitTools();

			void				_ObjectChanged(const Notifier* object);

			void				_ReplaceWidget(QWidget*& toReplace, QWidget* replacement);

private slots:
			void				_NewWindowActionInvoked();
			void				_QuitActionInvoked();
			void				_UndoActionInvoked();
			void				_RedoActionInvoked();

private:
			Ui::Window*			fUi;

			CanvasView*			fView;

			Document*			fDocument;
			RenderManager*		fRenderManager;
			ListenerAdapter		fCommandStackListener;
			Selection			fSelection;
			CurrentColor		fCurrentColor;

			SwatchGroup*		fSwatchGroup;
			ObjectTreeView*		fLayerTreeView;
			ResourceTreeView*	fResourceTreeView;

			IconButton*			fUndoIcon;
			IconButton*			fRedoIcon;
			IconButton*			fConfirmIcon;
			IconButton*			fCancelIcon;

			BList				fTools;
};


#endif // WINDOW_H
