/*
 * Copyright 2018 Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 * Distributed under the terms of the MIT License.
 *		
 */
#ifndef SHAPE_OBSERVER_H
#define SHAPE_OBSERVER_H


#include "AbstractLOAdapter.h"
#include "Shape.h"


class ShapeObserver : public Shape::Listener, public AbstractLOAdapter {
 public:
	enum {
		MSG_PATH_ADDED			= 'ptha',
		MSG_PATH_REMOVED		= 'pthr',
	};

							ShapeObserver(BHandler* handler);
	virtual					~ShapeObserver();

	virtual	void			PathAdded(const Shape* shape,
								const PathInstanceRef& path, int32 index);
	virtual	void			PathRemoved(const Shape* shape,
								const PathInstanceRef& path, int32 index);
};

#endif // SHAPE_OBSERVER_H
