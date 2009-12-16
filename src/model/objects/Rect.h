/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef RECT_H
#define RECT_H

#include <GraphicsDefs.h>
#include <List.h>
#include <Rect.h>

#include "Styleable.h"

class Rect;

class RectListener {
public:
								RectListener();
	virtual						~RectListener();

	virtual	void				AreaChanged(Rect* rect,
									const BRect& oldArea,
									const BRect& newArea);
	virtual	void				Deleted(Rect* rect);
};


class Rect : public Styleable {
public:
								Rect();
								Rect(const BRect& area,
									const rgb_color& color);
	virtual						~Rect();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	const char*			DefaultName() const;

	// Rect
			void				SetArea(const BRect& area);
	virtual	BRect				Area() const;

			bool				AddListener(RectListener* listener);
			void				RemoveListener(RectListener* listener);

private:
			void				_NotifyAreaChanged(const BRect& oldArea,
									const BRect& newArea);

			void				_NotifyDeleted();

private:
			BRect				fArea;

			BList				fListeners;
};

#endif // RECT_H
