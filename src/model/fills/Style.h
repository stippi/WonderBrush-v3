/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef STYLE_H
#define STYLE_H

#include "BaseObject.h"
#include "SetProperty.h"
#include "StrokeProperties.h"

class Paint;

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
	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object);


			void				SetFillPaint(Paint* paint);
			Paint*				FillPaint() const
									{ return fFillPaint; }

			void				SetStrokePaint(Paint* paint);
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

			uint64				fSetProperties;

			Paint*				fFillPaint;
			Paint*				fStrokePaint;
			::StrokeProperties*	fStrokeProperties;
};

#endif // STYLE_H
