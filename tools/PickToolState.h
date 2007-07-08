#ifndef PICK_TOOL_STATE_H
#define PICK_TOOL_STATE_H

#include "AbstractLOAdapter.h"
#include "Rect.h"
#include "Shape.h"
#include "ViewState.h"


class Document;
class Layer;
class CanvasView;


class PickToolState : public ViewState {

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
								PickToolState(CanvasView* parent,
									Layer* layer, Document* document);
	virtual						~PickToolState();

	// ViewState interface
	virtual	bool				MessageReceived(BMessage* message,
									Command** _command);

	virtual void				MouseDown(BPoint where, uint32 buttons,
										  uint32 clicks);
	virtual void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);
	virtual Command*			MouseUp();

	virtual void				Draw(BView* view, BRect updateRect);

	virtual	BRect				Bounds() const;

	// PickToolState
			void				SetRect(Rect* rect);
			void				SetShape(Shape* shape);

 private:
			bool				_HitTest(const BPoint& where,
									const BPoint& point);

			template<class ObjectType>
			void				_DragObject(ObjectType* object,
									BPoint where);

			template<class ObjectType>
			void				_DetermineDragMode(ObjectType* object,
									BPoint where);

			void				_Invalidate(BRect area);

			Document*			fDocument;
			Layer*				fLayer;

			Rect*				fRect;
			RectLOAdapater		fRectLOAdapter;

			Shape*				fShape;
			ShapeLOAdapater		fShapeLOAdapter;

			uint32				fDragMode;
			BPoint				fLastDragPos;
};

#endif // PICK_TOOL_STATE_H
