/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef LAYER_OBSERVER_H
#define LAYER_OBSERVER_H


#include "AbstractLOAdapter.h"
#include "Layer.h"


class LayerObserver : public Layer::Listener, public AbstractLOAdapter {
 public:
	enum {
		MSG_OBJECT_ADDED		= 'obja',
		MSG_OBJECT_REMOVED		= 'objr',
		MSG_OBJECT_CHANGED		= 'objc',
		MSG_AREA_INVALIDATED	= 'ainv'
	};

							LayerObserver(BHandler* handler);
	virtual					~LayerObserver();

	virtual	void			ObjectAdded(Layer* layer, Object* object,
								int32 index);
	virtual	void			ObjectRemoved(Layer* layer, Object* object,
								int32 index);
	virtual	void			ObjectChanged(Layer* layer, Object* object,
								int32 index);

	virtual	void			AreaInvalidated(Layer* layer,
								const BRect& area);
};

#endif // LAYER_OBSERVER_H
