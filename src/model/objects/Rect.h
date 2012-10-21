/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
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

	virtual	bool				HitTest(const BPoint& canvasPoint);

	// Rect
			void				SetArea(const BRect& area);
			BRect				Area() const;
	virtual	BRect				Bounds();

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
