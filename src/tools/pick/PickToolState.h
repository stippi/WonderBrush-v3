#ifndef PICK_TOOL_STATE_H
#define PICK_TOOL_STATE_H

#include "AbstractLOAdapter.h"
#include "Rect.h"
#include "Selection.h"
#include "Shape.h"
#include "ViewState.h"


class Document;
class Layer;

class PickToolState : public ViewState, public Selection::Controller,
	public Selection::Listener {
private:
	class RectLOAdapater : public RectListener,
						   public AbstractLOAdapter {
	public:
								RectLOAdapater(BHandler* handler);
		virtual					~RectLOAdapater();

		virtual	void			AreaChanged(Rect* rect,
									const BRect& oldArea,
									const BRect& newArea);
		virtual	void			Deleted(Rect* rect);
	};

	class ShapeLOAdapater : public ShapeListener,
							public AbstractLOAdapter {
	public:
								ShapeLOAdapater(BHandler* handler);
		virtual					~ShapeLOAdapater();

		virtual	void			AreaChanged(Shape* shape,
									const BRect& oldArea,
									const BRect& newArea);
		virtual	void			Deleted(Shape* shape);
	};


public:

//	enum {
//		MSG_OBJECT_PICKED	= 'objp'
//	};

								PickToolState(StateView* parent,
									Layer* layer, Document* document,
									Selection* selection);
	virtual						~PickToolState();

	// ViewState interface
	virtual	bool				MessageReceived(BMessage* message,
									Command** _command);

	virtual void				MouseDown(const MouseInfo& info);
	virtual void				MouseMoved(const MouseInfo& info);
	virtual Command*			MouseUp();

	virtual void				Draw(PlatformDrawContext& drawContext);

	virtual	BRect				Bounds() const;

	// Selection::Listener interface
	virtual	void				ObjectSelected(const Selectable& object,
									const Selection::Controller* controller);
	virtual	void				ObjectDeselected(const Selectable& object,
									const Selection::Controller* controller);

	// PickToolState
			void				SetObject(Layer* layer, Object* object);
			void				SetRect(Rect* rect);
			void				SetShape(Shape* shape);

private:
			Object*				_PickObject(const Layer* layer,
									BPoint where, bool recursive) const;

			bool				_HitTest(const BPoint& where,
									const BPoint& point);

			template<class ObjectType>
			void				_DragObject(ObjectType* object,
									BPoint where);

			template<class ObjectType>
			void				_DetermineDragMode(ObjectType* object,
									BPoint where);

			void				_Invalidate(BRect area);

			void				_SendPickNotification();

			Document*			fDocument;
			Selection*			fSelection;
			Layer*				fLayer;

			Rect*				fRect;
			RectLOAdapater		fRectLOAdapter;

			Shape*				fShape;
			ShapeLOAdapater		fShapeLOAdapter;

			uint32				fDragMode;
			BPoint				fLastDragPos;

private:
			class PlatformDelegate;

private:
			PlatformDelegate*	fPlatformDelegate;
};

#endif // PICK_TOOL_STATE_H
