/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

// This class should become the rendering backend. Compound rasterizer
// pipeline, blending functions, etc...
// * Attachable to bitmap/surface
// * graphics state properties (only current, no stack, since layers
//   are rendered out of order)

class RenderEngine {
public:
								RenderEngine();
	virtual						~RenderEngine();
};

#endif // RENDER_ENGINE_H
