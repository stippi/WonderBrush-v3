/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <List.h>
#include <Rect.h>

#include "NotifyingList.h"
#include "RWLocker.h"

class CommandStack;
class Layer;
class Path;
class Style;

typedef NotifyingList<Path> PathList;
typedef NotifyingList<Style> StyleList;

class Document : public RWLocker {
public:
	class Listener {
	 public:
								Listener();
		virtual					~Listener();
	};


								Document(const BRect& bounds);
	virtual						~Document();

	inline	::CommandStack*		CommandStack() const
									{ return fCommandStack; }

			status_t			InitCheck() const;

			BRect				Bounds() const;

			bool				AddListener(Listener* listener);
			void				RemoveListener(Listener* listener);

	inline	Layer*				RootLayer() const
									{ return fRootLayer; }
			bool				HasLayer(Layer* layer) const;

	inline	PathList*			GlobalPaths()
									{ return &fGlobalPaths; }

	inline	StyleList*			GlobalStyles()
									{ return &fGlobalStyles; }

private:
			bool				_HasLayer(Layer* parent, Layer* child) const;

			::CommandStack*		fCommandStack;
			Layer*				fRootLayer;

			PathList			fGlobalPaths;
			StyleList			fGlobalStyles;

			BList				fListeners;
};

#endif // DOCUMENT_H
