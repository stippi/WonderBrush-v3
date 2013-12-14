/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef MESSAGE_EXPORTER_H
#define MESSAGE_EXPORTER_H


#include "Exporter.h"

class BMessage;
class BPositionIO;

class ObjectSnapshot;
class RectSnapshot;
class ShapeSnapshot;
class TextSnapshot;

class MessageExporter : public Exporter {
public:
								MessageExporter();
	virtual						~MessageExporter();

	virtual	status_t			Export(const LayerSnapshot* rootSnapshot,
									BPositionIO* stream);

	virtual	const char*			MIMEType();

private:
			status_t			_ExportRoot(
									const LayerSnapshot* rootSnapshot,
									BMessage& archive);

			status_t			_ExportLayer(
									const LayerSnapshot* layer,
									BMessage& archive);

			status_t			_ExportObject(
									const ObjectSnapshot* object,
									BMessage& archive);

			status_t			_ExportRectangle(
									const RectSnapshot* rect,
									BMessage& archive);

			status_t			_ExportShape(
									const ShapeSnapshot* shape,
									BMessage& archive);

			status_t			_ExportText(
									const TextSnapshot* text,
									BMessage& archive);
};

#endif // MESSAGE_EXPORTER_H
