/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef WONDERBRUSH_2_IMPORTER_H
#define WONDERBRUSH_2_IMPORTER_H

#include "Document.h"
#include "Style.h"

class BMessage;
class BPositionIO;
class BaseObject;
class BoundedObject;
class Object;
class Styleable;
class Transformable;

class WonderBrush2Importer {
public:
								WonderBrush2Importer(const DocumentRef& document);
	virtual						~WonderBrush2Importer();

			status_t			Import(BPositionIO& stream) const;

			status_t			ImportDocument(const BMessage& archive) const;

			status_t			ImportObjects(const BMessage& archive,
									Layer* layer) const;

			BaseObjectRef		ImportObject(const BMessage& archive) const;

			BaseObjectRef		ImportBrushStroke(
									const BMessage& archive) const;
			BaseObjectRef		ImportFilterBrightness(
									const BMessage& archive) const;
			BaseObjectRef		ImportFilterContrast(
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
			BaseObjectRef		ImportShape(
									const BMessage& archive) const;
			BaseObjectRef		ImportText(
									const BMessage& archive) const;

			BaseObjectRef		ImportBrush(
									const BMessage& archive) const;
			BaseObjectRef		ImportPath(
									const BMessage& archive) const;
			BaseObjectRef		ImportStrokeProperties(
									const BMessage& archive) const;
			BaseObjectRef		ImportColorRenderer(
									const BMessage& archive) const;
			BaseObjectRef		ImportGradientRenderer(
									const BMessage& archive) const;
			BaseObjectRef		ImportEraseRenderer(
									const BMessage& archive) const;

private:
			template<class Type, class Container>
			status_t			_ImportObjects(const BMessage& archive,
									Container* container) const;

			void				_RestoreStyleable(Styleable* styleable,
									const BMessage& archive) const;
			void				_RestoreBoundedObject(BoundedObject* object,
									const BMessage& archive) const;
			void				_RestoreObject(Object* object,
									const BMessage& archive) const;
			void				_RestoreBaseObject(BaseObject* object,
									const BMessage& archive) const;

			void				_RestoreTransformable(
									Transformable* transformable,
									const BMessage& archive) const;

			StyleRef			_ImportStyle(
									const BMessage& rendererArchive) const;

private:
			DocumentRef			fDocument;
};

#endif // WONDERBRUSH_2_IMPORTER_H
