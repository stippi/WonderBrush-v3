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

#include "RWLocker.h"

class CommandStack;
class Layer;

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

 private:
			bool				_HasLayer(Layer* parent, Layer* child) const;

			::CommandStack*		fCommandStack;
			Layer*				fRootLayer;
			BList				fListeners;
};

#endif // DOCUMENT_H
