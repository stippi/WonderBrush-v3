/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef STYLE_H
#define STYLE_H

#include "BaseObject.h"
#include "Listener.h"
#include "Paint.h"
#include "SetProperty.h"
#include "StrokeProperties.h"

class BRect;

class Style : public BaseObject, public Listener {
public:
								Style();
								Style(const Style& other,
									CloneContext& context);

	virtual						~Style();

	// BaseObject interface
	virtual	BaseObject*			Clone(CloneContext& context) const;
	virtual	status_t			Unarchive(const BMessage* archive);
	virtual status_t			Archive(BMessage* into,
										bool deep = true) const;

	virtual	const char*			DefaultName() const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

	// Style
			bool				operator==(const Style& other) const;
			bool				operator!=(const Style& other) const;

			Style&				operator=(const Style& other);

			void				SetFillPaint(const PaintRef& paint);
			Paint*				FillPaint() const
									{ return fFillPaint.Get(); }

			void				SetStrokePaint(const PaintRef& paint);
			Paint*				StrokePaint() const
									{ return fStrokePaint.Get(); }

			void				SetStrokeProperties(
									const StrokePropertiesRef& properties);
			::StrokeProperties*	StrokeProperties() const
									{ return fStrokeProperties.Get(); }

			void				ExtendBounds(BRect& bounds) const;

private:
			template<typename PropertyType>
			void				_SetProperty(PropertyType*& member,
									PropertyType* newMember);

			template<typename PropertyType, typename ValueType,
				typename CacheType>
			void				_SetProperty(PropertyType*& member,
									const ValueType& newValue,
									CacheType& cache);

			template<typename PropertyType, typename CacheType>
			void				_UnsetProperty(PropertyType*& member,
									CacheType& cache);

			void				_AddPaintProperties(const Paint* paint,
									uint32 groupID, uint32 paintTypeID,
									PropertyObject* object,
									uint32 flags) const;

	static	Style&				_NullStyle();

private:
			Reference<Paint>	fFillPaint;
			Reference<Paint>	fStrokePaint;

			Reference< ::StrokeProperties>	fStrokeProperties;
};

typedef Reference< ::Style>		StyleRef;

#endif // STYLE_H
