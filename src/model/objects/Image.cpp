/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Image.h"

#include "ImageSnapshot.h"
#include "Interpolation.h"
#include "RenderBuffer.h"
#include "OptionProperty.h"
#include "RenderEngine.h"

// constructor
ImageListener::ImageListener()
{
}

// destructor
ImageListener::~ImageListener()
{
}

// Deleted
void
ImageListener::Deleted(Image* rect)
{
}

// #pragma mark -

// constructor
Image::Image()
	: BoundedObject()
	, fBuffer()
	, fInterpolation(INTERPOLATION_RESAMPLE)
	, fListeners(4)
{
}

// constructor
Image::Image(RenderBuffer* buffer)
	: BoundedObject()
	, fBuffer(buffer)
	, fInterpolation(INTERPOLATION_RESAMPLE)
	, fListeners(4)
{
}

// constructor
Image::Image(const Image& other)
	: BoundedObject(other)
	, fBuffer(other.fBuffer)
	, fInterpolation(INTERPOLATION_RESAMPLE)
	, fListeners(4)
{
}

// destructor
Image::~Image()
{
	_NotifyDeleted();
}

// #pragma mark -

// Clone
BaseObject*
Image::Clone(CloneContext& context) const
{
	return new(std::nothrow) Image(*this);
}

// DefaultName
const char*
Image::DefaultName() const
{
	return "Image";
}

// AddProperties
void
Image::AddProperties(PropertyObject* object, uint32 flags) const
{
	BoundedObject::AddProperties(object, flags);

	OptionProperty* interpolationProperty = new(std::nothrow) OptionProperty(
		PROPERTY_INTERPOLATION);
	if (interpolationProperty != NULL) {
		interpolationProperty->AddOption(
			INTERPOLATION_NEAREST_NEIGHBOR, "Nearest neighbor");
		interpolationProperty->AddOption(
			INTERPOLATION_BILINEAR, "Bilinear");
		interpolationProperty->AddOption(
			INTERPOLATION_RESAMPLE, "Resample");

		interpolationProperty->SetCurrentOptionID(fInterpolation);
	
		object->AddProperty(interpolationProperty);
	}
}

// SetToPropertyObject
bool
Image::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);
	BoundedObject::SetToPropertyObject(object, flags);

	// interpolation
	OptionProperty* interpolationProperty = dynamic_cast<OptionProperty*>(
		object->FindProperty(PROPERTY_INTERPOLATION));
	if (interpolationProperty != NULL)
		SetInterpolation(interpolationProperty->CurrentOptionID());

	return HasPendingNotifications();
}

// Snapshot
ObjectSnapshot*
Image::Snapshot() const
{
	return new ImageSnapshot(this);
}

// HitTest
bool
Image::HitTest(const BPoint& canvasPoint)
{
	if (fBuffer.Get() == NULL)
		return false;
	RenderEngine engine(Transformation());
	return engine.HitTest(fBuffer->Bounds(), canvasPoint);
}

// #pragma mark -

// Bounds
BRect
Image::Bounds()
{
	if (fBuffer != NULL)
		return fBuffer->Bounds();
	return BRect();
}

// SetBuffer
void
Image::SetBuffer(const RenderBufferRef& buffer)
{
	if (fBuffer == buffer)
		return;

	fBuffer = buffer;

	NotifyAndUpdate();
}

// SetInterpolation
void
Image::SetInterpolation(uint32 interpolation)
{
	if (fInterpolation == interpolation)
		return;

	fInterpolation = interpolation;

	NotifyAndUpdate();
}

// #pragma mark -

// AddListener
bool
Image::AddListener(ImageListener* listener)
{
	if (!listener || fListeners.HasItem(listener))
		return false;
	return fListeners.AddItem(listener);
}

// RemoveListener
void
Image::RemoveListener(ImageListener* listener)
{
	fListeners.RemoveItem(listener);
}

// #pragma mark -

// _NotifyDeleted
void
Image::_NotifyDeleted()
{
	int32 count = fListeners.CountItems();
	if (count == 0)
		return;

	BList listeners(fListeners);
	for (int32 i = 0; i < count; i++) {
		ImageListener* listener
			= (ImageListener*)listeners.ItemAtFast(i);
		listener->Deleted(this);
	}
}

