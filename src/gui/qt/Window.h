#ifndef WINDOW_H
#define WINDOW_H


#include <Window.h>

#include "ListenerAdapter.h"
#include "Selection.h"


class CanvasView;
class Document;
class Layer;
class RenderManager;
class Tool;


namespace Ui {
class Window;
}

class Window : public BWindow
{
	Q_OBJECT
	
public:
	explicit					Window(BRect frame, const char* title,
									Document* document, Layer* layer,
									QWidget* parent = 0);
								~Window();

			void				AddTool(Tool* tool);

			status_t			StoreSettings(BMessage& settings) const;
			void				RestoreSettings(const BMessage& settings);

			void				Show();

private:
			void				_InitTools();

			void				_ObjectChanged(const Notifier* object);

private:
			Ui::Window*			fUi;

			CanvasView*			fView;

			Document*			fDocument;
			RenderManager*		fRenderManager;
			ListenerAdapter		fCommandStackListener;
			Selection			fSelection;

			BList				fTools;
};


#endif // WINDOW_H
