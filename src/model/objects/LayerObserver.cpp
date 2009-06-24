/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "LayerObserver.h"

#include <Message.h>


// constructor
LayerObserver::LayerObserver(BHandler* handler)
	: Layer::Listener()
	, AbstractLOAdapter(handler)
{
}

// destructor
LayerObserver::~LayerObserver()
{
}

// ObjectAdded
void
LayerObserver::ObjectAdded(Layer* layer, Object* object, int32 index)
{
	BMessage message(MSG_OBJECT_ADDED);
	message.AddPointer("layer", layer);
	message.AddPointer("object", object);
	message.AddInt32("index", index);
	DeliverMessage(message);
}

// ObjectRemoved
void
LayerObserver::ObjectRemoved(Layer* layer, Object* object, int32 index)
{
	BMessage message(MSG_OBJECT_REMOVED);
	message.AddPointer("layer", layer);
	message.AddPointer("object", object);
	message.AddInt32("index", index);
	DeliverMessage(message);
}

// ObjectChanged
void
LayerObserver::ObjectChanged(Layer* layer, Object* object, int32 index)
{
	BMessage message(MSG_OBJECT_CHANGED);
	message.AddPointer("layer", layer);
	message.AddPointer("object", object);
	message.AddInt32("index", index);
	DeliverMessage(message);
}

// AreaInvalidated
void
LayerObserver::AreaInvalidated(Layer* layer, const BRect& area)
{
	BMessage message(MSG_AREA_INVALIDATED);
	message.AddPointer("layer", layer);
	message.AddRect("area", area);
	DeliverMessage(message);
}
