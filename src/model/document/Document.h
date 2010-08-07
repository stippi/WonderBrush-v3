/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <List.h>
#include <Rect.h>

#include "NotifyingList.h"
#include "RWLocker.h"

class BaseObject;
class CommandStack;
class Layer;
class Path;
class Style;

typedef NotifyingList<BaseObject> ResourceList;

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

	inline	ResourceList&		GlobalResources()
									{ return fGlobalResources; }
	inline	const ResourceList&	GlobalResources() const
									{ return fGlobalResources; }

private:
			bool				_HasLayer(Layer* parent, Layer* child) const;

			::CommandStack*		fCommandStack;
			Layer*				fRootLayer;

			ResourceList		fGlobalResources;

			BList				fListeners;
};

#endif // DOCUMENT_H
