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

#include "Object.h"

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


class Rect : public Object {
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
			BRect				Area() const;

			void				SetColor(const rgb_color& color);
			rgb_color			Color() const;

			bool				AddListener(RectListener* listener);
			void				RemoveListener(RectListener* listener);

 private:
			void				_NotifyAreaChanged(const BRect& oldArea,
									const BRect& newArea);

			void				_NotifyDeleted();

			BRect				fArea;
			rgb_color			fColor;

			BList				fListeners;
};

#endif // RECT_H
