#include "PickToolState.h"

#include <Message.h>

#include "ChangeAreaCommand.h"
#include "CommandStack.h"
#include "Document.h"
#include "Layer.h"
#include "View.h"

enum {
	DRAGGING_NONE = 0,
	DRAGGING_LEFT_TOP,
	DRAGGING_LEFT_BOTTOM,
	DRAGGING_RIGHT_TOP,
	DRAGGING_RIGHT_BOTTOM,
	DRAGGING_ALL,
};

enum {
	MSG_RECT_AREA_CHANGED		= 'racd',
	MSG_RECT_DELETED			= 'rdlt',

	MSG_SHAPE_AREA_CHANGED		= 'sacd',
	MSG_SHAPE_DELETED			= 'sdlt',
};

// constructor
PickToolState::RectLOAdapater::RectLOAdapater(BHandler* handler)
	: RectListener()
	, AbstractLOAdapter(handler)
{
}

// destructor
PickToolState::RectLOAdapater::~RectLOAdapater()
{
}

// AreaChanged
void
PickToolState::RectLOAdapater::AreaChanged(Rect* rect, const BRect& oldArea,
	const BRect& newArea)
{
	BMessage message(MSG_RECT_AREA_CHANGED);
	message.AddPointer("rect", rect);
	message.AddRect("area", oldArea | newArea);

	DeliverMessage(message);
}

// Deleted
void
PickToolState::RectLOAdapater::Deleted(Rect* rect)
{
	BMessage message(MSG_RECT_DELETED);
	message.AddPointer("rect", rect);

	DeliverMessage(message);
}

// #pragma mark -

// constructor
PickToolState::ShapeLOAdapater::ShapeLOAdapater(BHandler* handler)
	: ShapeListener()
	, AbstractLOAdapter(handler)
{
}

// destructor
PickToolState::ShapeLOAdapater::~ShapeLOAdapater()
{
}

// AreaChanged
void
PickToolState::ShapeLOAdapater::AreaChanged(Shape* shape, const BRect& oldArea,
	const BRect& newArea)
{
	BMessage message(MSG_SHAPE_AREA_CHANGED);
	message.AddPointer("shape", shape);
	message.AddRect("area", oldArea | newArea);

	DeliverMessage(message);
}

// Deleted
void
PickToolState::ShapeLOAdapater::Deleted(Shape* shape)
{
	BMessage message(MSG_SHAPE_DELETED);
	message.AddPointer("shape", shape);

	DeliverMessage(message);
}

// #pragma mark -

// constructor
PickToolState::PickToolState(View* parent, Layer* layer, Document* document)
	: ViewState(parent)
	, fDocument(document)
	, fLayer(layer)

	, fRect(NULL)
	, fRectLOAdapter(parent)

	, fShape(NULL)
	, fShapeLOAdapter(parent)

	, fDragMode(DRAGGING_NONE)
	, fLastDragPos(0.0, 0.0)
{
}

// destructor
PickToolState::~PickToolState()
{
	SetRect(NULL);
	SetShape(NULL);
}

// MessageReceived
bool
PickToolState::MessageReceived(BMessage* message, Command** _command)
{
	bool handled = true;

	switch (message->what) {
		case MSG_RECT_AREA_CHANGED: {
			Rect* rect;
			BRect area;
			if (message->FindPointer("rect", (void**)&rect) == B_OK
				&& message->FindRect("area", &area) == B_OK)
				if (rect == fRect)
					fView->Invalidate(area);
			break;
		}
		case MSG_RECT_DELETED: {
			Rect* rect;
			if (message->FindPointer("rect", (void**)&rect) == B_OK)
				if (rect == fRect)
					SetRect(NULL);
			break;
		}

		case MSG_SHAPE_AREA_CHANGED: {
			Shape* shape;
			BRect area;
			if (message->FindPointer("shape", (void**)&shape) == B_OK
				&& message->FindRect("area", &area) == B_OK)
				if (shape == fShape)
					fView->Invalidate(area);
			break;
		}
		case MSG_SHAPE_DELETED: {
			Shape* shape;
			if (message->FindPointer("shape", (void**)&shape) == B_OK)
				if (shape == fShape)
					SetShape(NULL);
			break;
		}

		default:
			handled = ViewState::MessageReceived(message, _command);
	}

	return handled;
}

// MouseDown
void
PickToolState::MouseDown(BPoint where, uint32 buttons, uint32 clicks)
{
	if (!fDocument->ReadLock())
		return;

	fDragMode = DRAGGING_NONE;

	BRect area(0, 0, -1, -1);
	if (fRect)
		area = fRect->Area();
	else if (fShape)
		area = fShape->Area();

	if (area.IsValid()) {
		if (_HitTest(where, area.LeftTop()))
			fDragMode = DRAGGING_LEFT_TOP;
		else if (_HitTest(where, area.LeftBottom()))
			fDragMode = DRAGGING_LEFT_BOTTOM;
		else if (_HitTest(where, area.RightTop()))
			fDragMode = DRAGGING_RIGHT_TOP;
		else if (_HitTest(where, area.RightBottom()))
			fDragMode = DRAGGING_RIGHT_BOTTOM;
		else if (area.Contains(where)) {
			fLastDragPos = where;
			fDragMode = DRAGGING_ALL;
		}
	}
	if (fDragMode == DRAGGING_NONE) {
		int32 count = fLayer->CountObjects();
		bool foundOther = false;
		for (int32 i = count - 1; i >= 0; i--) {
			Object* object = fLayer->ObjectAtFast(i);
			Rect* rect = dynamic_cast<Rect*>(object);
			if (rect) {
				if (rect->Area().Contains(where)) {
					SetRect(rect);
					foundOther = true;
					fLastDragPos = where;
					fDragMode = DRAGGING_ALL;
					break;
				}
			}
			Shape* shape = dynamic_cast<Shape*>(object);
			if (shape) {
				if (shape->Area().Contains(where)) {
					SetShape(shape);
					foundOther = true;
					fLastDragPos = where;
					fDragMode = DRAGGING_ALL;
					break;
				}
			}
		}
		if (!foundOther) {
			SetRect(NULL);
			SetShape(NULL);
		}
	}

	fDocument->ReadUnlock();

	fView->SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
}

// MouseMoved
void
PickToolState::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	if (fDragMode == DRAGGING_NONE)
		return;

	if (fRect) {
		_DragObject(fRect, where);
	} else if (fShape) {
		_DragObject(fShape, where);
	}

}

// MouseUp
Command*
PickToolState::MouseUp()
{
	fDragMode = DRAGGING_NONE;
	return NULL;
}

// #pragma mark -

// Draw
void
PickToolState::Draw(BView* view, BRect updateRect)
{
	if (!fDocument->ReadLock())
		return;

	if (fRect) {
		view->StrokeRect(fRect->Area());
	} else if (fShape) {
		view->StrokeRect(fShape->Area());
	}

	fDocument->ReadUnlock();
}

// #pragma mark -

// SetRect
void
PickToolState::SetRect(Rect* rect)
{
	if (rect == fRect)
		return;

	if (fRect) {
		fView->Invalidate(fRect->Area());
		fRect->RemoveListener(&fRectLOAdapter);
	}

	fRect = rect;

	if (fRect) {
		SetShape(NULL);

		fRect->AddListener(&fRectLOAdapter);
		fView->Invalidate(fRect->Area());
	}
}

// SetShape
void
PickToolState::SetShape(Shape* shape)
{
	if (shape == fShape)
		return;

	if (fShape) {
		fView->Invalidate(fShape->Area());
		fShape->RemoveListener(&fShapeLOAdapter);
	}

	fShape = shape;

	if (fShape) {
		SetRect(NULL);

		fShape->AddListener(&fShapeLOAdapter);
		fView->Invalidate(fShape->Area());
	}
}

// #pragma mark -

// _HitTest
bool
PickToolState::_HitTest(const BPoint& where, const BPoint& point)
{
	BRect test(point.x - 5, point.y - 5, point.x + 5, point.y + 5);
	return test.Contains(where);
}

// _DragObject
template<class ObjectType>
void
PickToolState::_DragObject(ObjectType* object, BPoint where)
{
	if (!fDocument->ReadLock())
		return;

	BRect area = object->Area();

	switch (fDragMode) {
		case DRAGGING_LEFT_TOP:
			area.SetLeftTop(where);
			break;
		case DRAGGING_LEFT_BOTTOM:
			area.SetLeftBottom(where);
			break;
		case DRAGGING_RIGHT_TOP:
			area.SetRightTop(where);
			break;
		case DRAGGING_RIGHT_BOTTOM:
			area.SetRightBottom(where);
			break;
		case DRAGGING_ALL:
			area.OffsetBy(where - fLastDragPos);
			fLastDragPos = where;
			break;
	};
	if (area.left > area.right) {
		float temp = area.left;
		area.left = area.right;
		area.right = temp;
		if (fDragMode == DRAGGING_LEFT_TOP)
			fDragMode = DRAGGING_RIGHT_TOP;
		else if (fDragMode == DRAGGING_LEFT_BOTTOM)
			fDragMode = DRAGGING_RIGHT_BOTTOM;
		else if (fDragMode == DRAGGING_RIGHT_TOP)
			fDragMode = DRAGGING_LEFT_TOP;
		else if (fDragMode == DRAGGING_RIGHT_BOTTOM)
			fDragMode = DRAGGING_LEFT_BOTTOM;
	}
	if (area.top > area.bottom) {
		float temp = area.top;
		area.top = area.bottom;
		area.bottom = temp;
		if (fDragMode == DRAGGING_LEFT_TOP)
			fDragMode = DRAGGING_LEFT_BOTTOM;
		else if (fDragMode == DRAGGING_LEFT_BOTTOM)
			fDragMode = DRAGGING_LEFT_TOP;
		else if (fDragMode == DRAGGING_RIGHT_TOP)
			fDragMode = DRAGGING_RIGHT_BOTTOM;
		else if (fDragMode == DRAGGING_RIGHT_BOTTOM)
			fDragMode = DRAGGING_RIGHT_TOP;
	}

	fDocument->ReadUnlock();

	// TODO: hold reference to "object"
	// TODO: writelock before perform?

	ChangeAreaCommand<ObjectType>* command
		= new ChangeAreaCommand<ObjectType>(fLayer, object, area);
	fDocument->CommandStack()->Perform(command);
}

