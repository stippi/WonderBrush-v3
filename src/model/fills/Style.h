/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef STYLE_H
#define STYLE_H

#include "BaseObject.h"
#include "Paint.h"
#include "SetProperty.h"
#include "SharedObjectCache.h"
#include "StrokeProperties.h"


typedef SharedObject<Paint>			SharedPaint;
typedef SharedObjectCache<Paint>	PaintCache;


class Style : public BaseObject {
public:
								Style();
								Style(const Style& other);

	virtual						~Style();

	// BaseObject interface
	virtual	status_t			Unarchive(const BMessage* archive);
	virtual status_t			Archive(BMessage* into,
										bool deep = true) const;

	virtual	const char*			DefaultName() const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	// Style

			void				SetFillPaint(const Paint& paint);
			void				UnsetFillPaint();
			Paint*				FillPaint() const
									{ return fFillPaint; }

			void				SetStrokePaint(const Paint& paint);
			void				UnsetStrokePaint();
			Paint*				StrokePaint() const
									{ return fStrokePaint; }

			void				SetStrokeProperties(
									const ::StrokeProperties& properties);
			void				SetStrokeProperties(
									::StrokeProperties* properties);
			::StrokeProperties*	StrokeProperties() const
									{ return fStrokeProperties; }

private:
			template<typename PropertyType>
			void				_SetProperty(PropertyType*& member,
									PropertyType* newMember,
									uint64 setProperty);

			template<typename PropertyType, typename ValueType,
				typename CacheType>
			void				_SetProperty(PropertyType*& member,
									const ValueType& newValue,
									CacheType& cache,
									uint64 setProperty);

			template<typename PropertyType, typename CacheType>
			void				_UnsetProperty(PropertyType*& member,
									CacheType& cache,
									uint64 setProperty);

private:
	static	PaintCache			sPaintCache;
	static	Style				sNullStyle;

			uint64				fSetProperties;

			SharedPaint*		fFillPaint;
			SharedPaint*		fStrokePaint;

			::StrokeProperties*	fStrokeProperties;
};

#endif // STYLE_H
