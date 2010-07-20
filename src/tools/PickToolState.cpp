#include "PickToolState.h"

#include <Message.h>
#include <Window.h>

#include "CanvasView.h"
#include "ChangeAreaCommand.h"
#include "CommandStack.h"
#include "Document.h"
#include "Layer.h"

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
PickToolState::PickToolState(StateView* parent, Layer* layer,
		Document* document, Selection* selection)
	:
	ViewState(parent),
	fDocument(document),
	fSelection(selection),
	fLayer(layer),

	fRect(NULL),
	fRectLOAdapter(parent),

	fShape(NULL),
	fShapeLOAdapter(parent),

	fDragMode(DRAGGING_NONE),
	fLastDragPos(0.0, 0.0)
{
	fSelection->AddListener(this);
}

// destructor
PickToolState::~PickToolState()
{
	fSelection->RemoveListener(this);
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
				if (rect == fRect) {
					_Invalidate(area);
				}
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
				if (shape == fShape) {
					_Invalidate(area);
				}
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

	fView->ConvertToCanvas(&where);

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
// This method would prefer picking objects on the same layer as
// the current one.
//		Object* pickedObject = _PickObject(fLayer, where, false);
//		if (pickedObject == NULL)
//			pickedObject = _PickObject(fDocument->RootLayer(), where, true);
		Object* pickedObject = _PickObject(fDocument->RootLayer(), where, true);

		Layer* layer = fLayer;
		if (pickedObject) {
			fDragMode = DRAGGING_ALL;
			fLastDragPos = where;
			layer = pickedObject->Parent();
		}
		SetObject(layer, pickedObject);
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

	fView->ConvertToCanvas(&where);

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

	BRect r;
	if (fRect)
		r = fRect->Area();
	else if (fShape)
		r = fShape->Area();

	fDocument->ReadUnlock();

	if (r.IsValid()) {
		fView->ConvertFromCanvas(&r);
		view->SetHighColor(0, 0, 0);
		view->StrokeRect(r);
		r.InsetBy(-1, -1);
		view->SetHighColor(255, 255, 255);
		view->StrokeRect(r);
	}
}

// Bounds
BRect
PickToolState::Bounds() const
{
	BRect r;

	if (!fDocument->ReadLock())
		return r;

	if (fRect)
		r = fRect->Area();
	else if (fShape)
		r = fShape->Area();

	fDocument->ReadUnlock();

	if (r.IsValid()) {
		fView->ConvertFromCanvas(&r);
		r.InsetBy(-1, -1);
	}

	return r;
}

// #pragma mark -

// ObjectSelected
void
PickToolState::ObjectSelected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	Object* object = dynamic_cast<Object*>(selectable.Get());
	Layer* layer = object != NULL ? object->Parent() : NULL;
	SetObject(layer, object);
}

// ObjectDeselected
void
PickToolState::ObjectDeselected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	Object* object = dynamic_cast<Object*>(selectable.Get());
	if (object == fRect || object == fShape)
		SetObject(NULL, NULL);
}

// #pragma mark -

// SetObject
void
PickToolState::SetObject(Layer* layer, Object* object)
{
	fLayer = layer;
	if (Rect* rect = dynamic_cast<Rect*>(object)) {
		SetRect(rect);
	} else if (Shape* shape = dynamic_cast<Shape*>(object)) {
		SetShape(shape);
	} else {
		SetRect(NULL);
		SetShape(NULL);
	}
}

// SetRect
void
PickToolState::SetRect(Rect* rect)
{
	if (rect == fRect)
		return;

	if (fRect) {
		_Invalidate(fRect->Area());
		fRect->RemoveListener(&fRectLOAdapter);
	}

	fRect = rect;

	if (fRect) {
		SetShape(NULL);

		fRect->AddListener(&fRectLOAdapter);
		_Invalidate(fRect->Area());
	}

	_SendPickNotification();
}

// SetShape
void
PickToolState::SetShape(Shape* shape)
{
	if (shape == fShape)
		return;

	if (fShape) {
		_Invalidate(fShape->Area());
		fShape->RemoveListener(&fShapeLOAdapter);
	}

	fShape = shape;

	if (fShape) {
		SetRect(NULL);

		fShape->AddListener(&fShapeLOAdapter);
		_Invalidate(fShape->Area());
	}

	_SendPickNotification();
}

// #pragma mark -

// _PickObject
Object*
PickToolState::_PickObject(const Layer* layer, BPoint where,
	bool recursive) const
{
	// search sublayers first
	int32 count = layer->CountObjects();
	for (int32 i = count - 1; i >= 0; i--) {
		Object* object = layer->ObjectAtFast(i);
		if (recursive) {
			Layer* subLayer = dynamic_cast<Layer*>(object);
			if (subLayer) {
				Object* objectOnSubLayer = _PickObject(subLayer, where, true);
				if (objectOnSubLayer)
					return objectOnSubLayer;
			}
		}
		Rect* rect = dynamic_cast<Rect*>(object);
		if (rect && rect->Area().Contains(where))
			return object;
		Shape* shape = dynamic_cast<Shape*>(object);
		if (shape && shape->Area().Contains(where))
			return object;
	}

	return NULL;
}

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
	if (!fDocument->WriteLock())
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

	// TODO: hold reference to "object"

	ChangeAreaCommand<ObjectType>* command
		= new ChangeAreaCommand<ObjectType>(object, area);
	fDocument->CommandStack()->Perform(command);

	UpdateBounds();

	fDocument->WriteUnlock();
}

// _Invalidate
void
PickToolState::_Invalidate(BRect area)
{
	area.left = floorf(area.left);
	area.top = floorf(area.top);
	area.right = ceilf(area.right);
	area.bottom = ceilf(area.bottom);
	fView->ConvertFromCanvas(&area);
	area.InsetBy(-2, -2);
	fView->InvalidateCanvas(area);
}

// _SendPickNotification
void
PickToolState::_SendPickNotification()
{
//	if (fRect != NULL)
//		fSelection->Select(Selectable(fRect), this);
//	else if (fShape != NULL)
//		fSelection->Select(Selectable(fShape), this);
//	else
//		fSelection->DeselectAll(this);
}

