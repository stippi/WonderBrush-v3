/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef MESSAGE_IMPORTER_H
#define MESSAGE_IMPORTER_H

#include "Document.h"

class BMessage;
class BPositionIO;

class MessageImporter {
public:
								MessageImporter(const DocumentRef& document);
	virtual						~MessageImporter();

			status_t			Import(BPositionIO& stream) const;

			status_t			ImportDocument(const BMessage& archive) const;

			status_t			ImportGlobalResources(
									const BMessage& archive) const;

			status_t			ImportObjects(const BMessage& archive) const;

			BaseObjectRef		ImportObject(const BMessage& archive) const;

			BaseObjectRef		ImportBrushStroke(
									const BMessage& archive) const;
			BaseObjectRef		ImportFilterGaussianBlur(
									const BMessage& archive) const;
			BaseObjectRef		ImportFilterDropShadow(
									const BMessage& archive) const;
			BaseObjectRef		ImportFilterSaturation(
									const BMessage& archive) const;
			BaseObjectRef		ImportImage(
									const BMessage& archive) const;
			BaseObjectRef		ImportLayer(
									const BMessage& archive) const;
			BaseObjectRef		ImportRect(
									const BMessage& archive) const;
			BaseObjectRef		ImportShape(
									const BMessage& archive) const;
			BaseObjectRef		ImportText(
									const BMessage& archive) const;

			BaseObjectRef		ImportBrush(
									const BMessage& archive) const;
			BaseObjectRef		ImportColor(
									const BMessage& archive) const;
			BaseObjectRef		ImportColorShade(
									const BMessage& archive) const;
			BaseObjectRef		ImportGradient(
									const BMessage& archive) const;
			BaseObjectRef		ImportPaint(
									const BMessage& archive) const;
			BaseObjectRef		ImportStrokeProperties(
									const BMessage& archive) const;
			BaseObjectRef		ImportStyle(
									const BMessage& archive) const;

private:
			DocumentRef			fDocument;
};

#endif // MESSAGE_IMPORTER_H
