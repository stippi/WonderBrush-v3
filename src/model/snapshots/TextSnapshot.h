/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef TEXT_SNAPSHOT_H
#define TEXT_SNAPSHOT_H

#include <GraphicsDefs.h>

#include "StyleableSnapshot.h"
#include "TextLayout.h"

class Text;

class TextSnapshot : public StyleableSnapshot {
public:
								TextSnapshot(const Text* rect);
	virtual						~TextSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Render(RenderEngine& engine,
									RenderBuffer* bitmap, BRect area) const;

private:
			const Text*			fOriginal;
			TextLayout			fTextLayout;
};

#endif // TEXT_SNAPSHOT_H
