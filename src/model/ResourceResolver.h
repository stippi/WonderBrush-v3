/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef RESOURCE_RESOLVER_H
#define RESOURCE_RESOLVER_H

class BaseObject;

class ResourceResolver {
public:
								ResourceResolver();
	virtual						~ResourceResolver();

	virtual	BaseObject*			Resolve(BaseObject* object);

};

#endif // RESOURCE_RESOLVER_H
