/*
 * Copyright 2007-2013 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <Window.h>

#include "CurrentColor.h"
#include "EditContext.h"
#include "ListenerAdapter.h"
#include "Selection.h"

class BCardLayout;
class BMenu;
class BSplitView;
class CanvasView;
class ColumnTreeModel;
class Document;
class DocumentSaver;
class Exporter;
class IconButton;
class IconOptionsControl;
class InspectorView;
class Layer;
class Object;
class ObjectColumnTreeItem;
class ObjectTreeView;
class RenderManager;
class ResourceTreeView;
class SwatchGroup;
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

			status_t			StoreSettings(BMessage& settings) const;
			void				RestoreSettings(const BMessage& settings);

private:
			void				_InitTools();
			void				_SetTool(int32 index);

			void				_ObjectChanged(const Notifier* object);

			BMenu*				_CreateImageMenu() const;
			void				_AddFilterMenuItem(BMenu* menu,
									const char* label,
									int32 filterID) const;
			BMenu*				_CreateObjectMenu() const;

			void				_RunResizeImageDialog();

			bool				_GetInsertionPosition(Layer** _layer,
									int32* _index) const;
			void				_AddLayer();
			void				_AddFilter(int32 filterID);
			void				_AddObject(Layer* parentLayer,
									int32 insertIndex, Object* object);
			void				_RemoveObjects();
			void				_ResetTransformation();

			bool				_CheckSaveDocument(
									const BMessage* currentMessage);
			void				_PickUpActionBeforeSave();

			void				_Save();
			void				_Save(Exporter* exporter) const;

			DocumentSaver*		_CreateSaver(const entry_ref& ref,
									uint32 exportMode) const;

			const char*			_FileName(bool preferExporter) const;
			void				_UpdateWindowTitle();

private:
			class SelectionListener;
			class ToolListener;

			CanvasView*			fView;
			Reference<Document>	fDocument;
			RenderManager*		fRenderManager;
			ListenerAdapter		fEditManagerListener;
			Selection			fSelection;
			SelectionListener*	fSelectionListener;
			EditContext			fEditContext;
			CurrentColor		fCurrentColor;

			BMenu*				fFileMenu;
			BMenu*				fEditMenu;
			BMenu*				fImageMenu;
			BMenu*				fObjectMenu;
			BMenu*				fResourceMenu;
			BMenu*				fPropertyMenu;

			BMenuItem*			fUndoMI;
			BMenuItem*			fRedoMI;
			BMenuItem*			fRemoveMI;

			IconOptionsControl*	fToolIconControl;
			BCardLayout*		fToolConfigLayout;
			SwatchGroup*		fSwatchGroup;
			ObjectTreeView*		fLayerTreeView;
//			ColumnTreeModel*	fLayerTreeModel;
			ResourceTreeView*	fResourceTreeView;
			InspectorView*		fInspectorView;

			BSplitView*			fHorizontalSplit;
			BSplitView*			fVerticalSplit;

			IconButton*			fUndoIcon;
			IconButton*			fRedoIcon;
			IconButton*			fConfirmIcon;
			IconButton*			fCancelIcon;

			BList				fTools;
			int32				fCurrentToolIndex;
			Tool*				fCurrentTool;
			ToolListener*		fToolListener;

			BMessage*			fMessageAfterSave;
			Exporter*			fExporter;
};

#endif // WINDOW_H
