/*
 * Copyright 2018 Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 * Distributed under the terms of the MIT License.
 *		
 */
#include "ShapeObserver.h"

#include <Message.h>


// constructor
ShapeObserver::ShapeObserver(BHandler* handler)
	: Shape::Listener()
	, AbstractLOAdapter(handler)
{
}

// destructor
ShapeObserver::~ShapeObserver()
{
}

// PathAdded
void
ShapeObserver::PathAdded(const Shape* shape, const PathInstanceRef& path,
	int32 index)
{
	BMessage message(MSG_PATH_ADDED);
	message.AddPointer("shape", shape);
	message.AddPointer("path", path.Get());
	message.AddInt32("index", index);
	DeliverMessage(message);
}

// PathRemoved
void
ShapeObserver::PathRemoved(const Shape* shape, const PathInstanceRef& path,
	int32 index)
{
	BMessage message(MSG_PATH_REMOVED);
	message.AddPointer("shape", shape);
	message.AddPointer("path", path.Get());
	message.AddInt32("index", index);
	DeliverMessage(message);
}

