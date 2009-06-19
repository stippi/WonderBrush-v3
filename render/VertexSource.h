/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef VERTEX_SOURCE_H
#define VERTEX_SOURCE_H

class VertexSource {
 public:
								VertexSource();
	virtual						~VertexSource();

    virtual	void				rewind(unsigned path_id) = 0;
    virtual	unsigned			vertex(double* x, double* y) = 0;

	virtual	bool				WantsOpenPaths() const = 0;
	virtual	double				ApproximationScale() const = 0;
};

#endif // VERTEX_SOURCE_H
