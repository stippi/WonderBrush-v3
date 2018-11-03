/*
 * Copyright 2007-2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef PATH_INSTANCE_H
#define PATH_INSTANCE_H

#include "Path.h"
#include "Shape.h"

class PathInstance : public BaseObject {
public:
								PathInstance(::Path* path);
								PathInstance(::Path* path, ::Shape* shape);
								PathInstance(const PathInstance& other,
									CloneContext& context);
	virtual						~PathInstance();

	// BaseObject interface
	virtual	BaseObject*			Clone(CloneContext& context) const;
	virtual	const char*			DefaultName() const;

	virtual	bool				GetIcon(const BBitmap* bitmap) const;


			const PathRef&		Path() const
									{ return fPath; }
			::Shape*			Shape() const
									{ return fShape; }
			Transformable		Transformation() const;

private:
			PathRef				fPath;
			::Shape*			fShape;
};

typedef Reference<PathInstance>	PathInstanceRef;

#endif // PATH_INSTANCE_H
