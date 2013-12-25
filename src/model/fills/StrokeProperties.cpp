/*
 * Copyright 2009-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "StrokeProperties.h"

#include <new>

#include <stdio.h>

#include <Autolock.h>

#include "OptionProperty.h"

#define DEBUG_INSTANCE_COUNT 0

#if DEBUG_INSTANCE_COUNT
#include "StackTrace.h"
static vint32 sInstanceCount = 0;
#endif

// constructor
StrokeProperties::StrokeProperties()
	: BaseObject()
	, fWidth(0.0f)
	, fMiterLimit(4.0f)
	, fSetProperties(0)
	, fCapMode(ButtCap)
	, fJoinMode(MiterJoin)
	, fStrokePosition(CenterStroke)
{
#if DEBUG_INSTANCE_COUNT
	int32 count = atomic_add(&sInstanceCount, 1);
	BAutolock _(get_stack_trace_locker());
	printf("[%ld] %p StrokeProperties(): %ld\n", find_thread(NULL), this,
		count + 1);
	print_stack_trace();
#endif
}

// constructor
StrokeProperties::StrokeProperties(float width)
	: BaseObject()
	, fWidth(width)
	, fMiterLimit(4.0f)
	, fSetProperties(STROKE_WIDTH)
	, fCapMode(ButtCap)
	, fJoinMode(MiterJoin)
	, fStrokePosition(CenterStroke)
{
#if DEBUG_INSTANCE_COUNT
	int32 count = atomic_add(&sInstanceCount, 1);
	BAutolock _(get_stack_trace_locker());
	printf("[%ld] %p StrokeProperties(%.1f): %ld\n", find_thread(NULL), this,
		width, count + 1);
	print_stack_trace();
#endif
}

// constructor
StrokeProperties::StrokeProperties(float width, ::CapMode capMode)
	: BaseObject()
	, fWidth(width)
	, fMiterLimit(4.0f)
	, fSetProperties(STROKE_WIDTH | STROKE_CAP_MODE)
	, fCapMode(capMode)
	, fJoinMode(MiterJoin)
	, fStrokePosition(CenterStroke)
{
#if DEBUG_INSTANCE_COUNT
	int32 count = atomic_add(&sInstanceCount, 1);
	BAutolock _(get_stack_trace_locker());
	printf("[%ld] %p StrokeProperties(%.1f, cap %d): %ld\n", find_thread(NULL),
		this, width, capMode, count + 1);
	print_stack_trace();
#endif
}

// constructor
StrokeProperties::StrokeProperties(float width, ::JoinMode joinMode)
	: BaseObject()
	, fWidth(width)
	, fMiterLimit(4.0f)
	, fSetProperties(STROKE_WIDTH | STROKE_JOIN_MODE)
	, fCapMode(ButtCap)
	, fJoinMode(joinMode)
	, fStrokePosition(CenterStroke)
{
#if DEBUG_INSTANCE_COUNT
	int32 count = atomic_add(&sInstanceCount, 1);
	BAutolock _(get_stack_trace_locker());
	printf("[%ld] %p StrokeProperties(%.1f, join %d): %ld\n", find_thread(NULL),
		this, width, joinMode, count + 1);
	print_stack_trace();
#endif
}

// constructor
StrokeProperties::StrokeProperties(float width, ::CapMode capMode,
		::JoinMode joinMode)
	: BaseObject()
	, fWidth(width)
	, fMiterLimit(4.0f)
	, fSetProperties(STROKE_WIDTH | STROKE_CAP_MODE | STROKE_JOIN_MODE)
	, fCapMode(capMode)
	, fJoinMode(joinMode)
	, fStrokePosition(CenterStroke)
{
#if DEBUG_INSTANCE_COUNT
	int32 count = atomic_add(&sInstanceCount, 1);
	BAutolock _(get_stack_trace_locker());
	printf("[%ld] %p StrokeProperties(%.1f, cap: %d, join %d): %ld\n",
		find_thread(NULL), this, width, capMode, joinMode, count + 1);
	print_stack_trace();
#endif
}

// constructor
StrokeProperties::StrokeProperties(float width, ::CapMode capMode,
		::JoinMode joinMode, float miterLimit)
	: BaseObject()
	, fWidth(width)
	, fMiterLimit(miterLimit)
	, fSetProperties(STROKE_WIDTH | STROKE_CAP_MODE | STROKE_JOIN_MODE
		| STROKE_MITER_LIMIT)
	, fCapMode(capMode)
	, fJoinMode(joinMode)
	, fStrokePosition(CenterStroke)
{
#if DEBUG_INSTANCE_COUNT
	int32 count = atomic_add(&sInstanceCount, 1);
	BAutolock _(get_stack_trace_locker());
	printf("[%ld] %p StrokeProperties(%.1f, cap: %d, join %d, %.1f): %ld\n",
		find_thread(NULL), this, width, capMode, joinMode, miterLimit,
		count + 1);
	print_stack_trace();
#endif
}

// constructor
StrokeProperties::StrokeProperties(const StrokeProperties& other)
	: BaseObject()
{
#if DEBUG_INSTANCE_COUNT
	int32 count = atomic_add(&sInstanceCount, 1);
	BAutolock _(get_stack_trace_locker());
	printf("[%ld] %p StrokeProperties(copy): %ld\n", find_thread(NULL), this,
		count + 1);
	print_stack_trace();
#endif
	*this = other;
}

// destructor
StrokeProperties::~StrokeProperties()
{
#if DEBUG_INSTANCE_COUNT
	int32 count = atomic_add(&sInstanceCount, -1);
	BAutolock _(get_stack_trace_locker());
	printf("[%ld] %p ~StrokeProperties(): %ld\n", find_thread(NULL), this,
		count - 1);
	print_stack_trace();
#endif
}

// #pragma mark -

// Clone
BaseObject*
StrokeProperties::Clone(ResourceResolver& resolver) const
{
	return new(std::nothrow) StrokeProperties(*this);
}

// Unarchive
status_t
StrokeProperties::Unarchive(const BMessage* archive)
{
	status_t ret = BaseObject::Unarchive(archive);

	// TODO: ...

	return ret;
}

// Archive
status_t
StrokeProperties::Archive(BMessage* into, bool deep) const
{
	status_t ret = BaseObject::Archive(into, deep);

//	if (ret == B_OK)
//		ret = into->AddInt32("type", fType);

	return ret;
}

// AddProperties
void
StrokeProperties::AddProperties(PropertyObject* object, uint32 flags) const
{
//	BaseObject::AddProperties(object, flags);

	// width
	FloatProperty* widthProperty = new(std::nothrow) FloatProperty(
		PROPERTY_WIDTH, fWidth);
	if (widthProperty == NULL || !object->AddProperty(widthProperty)) {
		delete widthProperty;
		return;
	}
	if ((fSetProperties & STROKE_WIDTH) == 0)
		widthProperty->SetValue(0.0f);

	// miter limit
	FloatProperty* miterLimitProperty = new(std::nothrow) FloatProperty(
		PROPERTY_MITER_LIMIT, fMiterLimit);
	if (miterLimitProperty == NULL
		|| !object->AddProperty(miterLimitProperty)) {
		delete miterLimitProperty;
		return;
	}
	if ((fSetProperties & STROKE_MITER_LIMIT) == 0)
		miterLimitProperty->SetValue(0.0f);

	// cap mode
	OptionProperty* capMode = new(std::nothrow) OptionProperty(
		PROPERTY_CAP_MODE);
	if (capMode == NULL || !object->AddProperty(capMode)) {
		delete capMode;
		return;
	}
	capMode->AddOption(-1, "Inherited");
	capMode->AddOption(ButtCap, "Butt");
	capMode->AddOption(SquareCap, "Square");
	capMode->AddOption(RoundCap, "Round");
	if ((fSetProperties & STROKE_CAP_MODE) != 0)
		capMode->SetCurrentOptionID(CapMode());
	else
		capMode->SetCurrentOptionID(-1);

	// join mode
	OptionProperty* joinMode = new(std::nothrow) OptionProperty(
		PROPERTY_JOIN_MODE);
	if (joinMode == NULL || !object->AddProperty(joinMode)) {
		delete joinMode;
		return;
	}
	joinMode->AddOption(-1, "Inherited");
	joinMode->AddOption(MiterJoin, "Miter");
	joinMode->AddOption(MiterJoinRevert, "Miter revert");
	joinMode->AddOption(MiterJoinRound, "Miter round");
	joinMode->AddOption(RoundJoin, "Round");
	joinMode->AddOption(BevelJoin, "Bevel");
	if ((fSetProperties & STROKE_JOIN_MODE) != 0)
		joinMode->SetCurrentOptionID(JoinMode());
	else
		joinMode->SetCurrentOptionID(-1);

	// stroke position
	OptionProperty* strokePosition = new(std::nothrow) OptionProperty(
		PROPERTY_STROKE_POSITION);
	if (strokePosition == NULL || !object->AddProperty(strokePosition)) {
		delete strokePosition;
		return;
	}
	strokePosition->AddOption(-1, "Inherited");
	strokePosition->AddOption(CenterStroke, "Center");
	strokePosition->AddOption(OutsideStroke, "Outside");
	strokePosition->AddOption(InsideStroke, "Inside");
	if ((fSetProperties & STROKE_POSITION) != 0)
		strokePosition->SetCurrentOptionID(StrokePosition());
	else
		strokePosition->SetCurrentOptionID(-1);
}

// SetToPropertyObject
bool
StrokeProperties::SetToPropertyObject(const PropertyObject* object,
	uint32 flags)
{
	AutoNotificationSuspender _(this);

	BaseObject::SetToPropertyObject(object, flags);

	// TODO: Allow value for "inherited"
	if (object->FindProperty(PROPERTY_WIDTH))
		SetWidth(object->Value(PROPERTY_WIDTH, fWidth));
	// TODO: Allow value for "inherited"
	if (object->FindProperty(PROPERTY_MITER_LIMIT))
		SetMiterLimit(object->Value(PROPERTY_MITER_LIMIT, fMiterLimit));

	OptionProperty* capMode = dynamic_cast<OptionProperty*>(
		object->FindProperty(PROPERTY_CAP_MODE));
	if (capMode != NULL) {
		if (capMode->CurrentOptionID() == -1) {
			fSetProperties &= ~STROKE_CAP_MODE;
			Notify();
		} else
			SetCapMode(static_cast< ::CapMode>(capMode->CurrentOptionID()));
	}

	OptionProperty* joinMode = dynamic_cast<OptionProperty*>(
		object->FindProperty(PROPERTY_JOIN_MODE));
	if (joinMode != NULL) {
		if (joinMode->CurrentOptionID() == -1) {
			fSetProperties &= ~STROKE_JOIN_MODE;
			Notify();
		} else
			SetJoinMode(static_cast< ::JoinMode>(joinMode->CurrentOptionID()));
	}

	OptionProperty* strokePosition = dynamic_cast<OptionProperty*>(
		object->FindProperty(PROPERTY_STROKE_POSITION));
	if (strokePosition != NULL) {
		if (strokePosition->CurrentOptionID() == -1) {
			fSetProperties &= ~STROKE_POSITION;
			Notify();
		} else {
			SetStrokePosition(static_cast< ::StrokePosition>(
				strokePosition->CurrentOptionID()));
		}
	}

	return HasPendingNotifications();
}

// DefaultName
const char*
StrokeProperties::DefaultName() const
{
	return "Stroke properties";
}

// #pragma mark -

// operator=
StrokeProperties&
StrokeProperties::operator=(const StrokeProperties& other)
{
	AutoNotificationSuspender _(this);

	if (fWidth != other.fWidth) {
		fWidth = other.fWidth;
		if ((other.fSetProperties & STROKE_WIDTH) != 0)
			Notify();
	}

	if (fMiterLimit != other.fMiterLimit) {
		fMiterLimit = other.fMiterLimit;
		if ((other.fSetProperties & STROKE_MITER_LIMIT) != 0)
			Notify();
	}

	if (fCapMode != other.fCapMode) {
		fCapMode = other.fCapMode;
		if ((other.fSetProperties & STROKE_CAP_MODE) != 0)
			Notify();
	}

	if (fJoinMode != other.fJoinMode) {
		fJoinMode = other.fJoinMode;
		if ((other.fSetProperties & STROKE_JOIN_MODE) != 0)
			Notify();
	}

	if (fStrokePosition != other.fStrokePosition) {
		fStrokePosition = other.fStrokePosition;
		if ((other.fSetProperties & STROKE_POSITION) != 0)
			Notify();
	}

	if (fSetProperties != other.fSetProperties) {
		fSetProperties = other.fSetProperties;
		Notify();
	}

	return *this;
}

// operator==
bool
StrokeProperties::operator==(const StrokeProperties& other) const
{
	return fWidth == other.fWidth
		&& fMiterLimit == other.fMiterLimit
		&& fSetProperties == other.fSetProperties
		&& fCapMode == other.fCapMode
		&& fJoinMode == other.fJoinMode
		&& fStrokePosition == other.fStrokePosition;
}

// operator!=
bool
StrokeProperties::operator!=(const StrokeProperties& other) const
{
	return !(*this == other);
}

// SetWidth
void
StrokeProperties::SetWidth(float width)
{
	if (width == 0.0f) {
		if ((fSetProperties & STROKE_WIDTH) == 0)
			return;
		fSetProperties &= ~STROKE_WIDTH;
	} else {
		if ((fSetProperties & STROKE_WIDTH) != 0
			&& fWidth == width) {
			return;
		}
		fWidth = width;
		fSetProperties |= STROKE_WIDTH;
	}
	Notify();
}

// SetMiterLimit
void
StrokeProperties::SetMiterLimit(float miterLimit)
{
	if (miterLimit == 0.0f) {
		if ((fSetProperties & STROKE_MITER_LIMIT) == 0)
			return;
		fSetProperties &= ~STROKE_MITER_LIMIT;
	} else {
		if ((fSetProperties & STROKE_MITER_LIMIT) != 0
			&& fMiterLimit == miterLimit) {
			return;
		}
		fMiterLimit = miterLimit;
		fSetProperties |= STROKE_MITER_LIMIT;
	}
	Notify();
}

// SetCapMode
void
StrokeProperties::SetCapMode(::CapMode capMode)
{
	if ((fSetProperties & STROKE_CAP_MODE) != 0
		&& CapMode() == capMode) {
		return;
	}
	fCapMode = capMode;
	fSetProperties |= STROKE_CAP_MODE;
	Notify();
}

// SetJoinMode
void
StrokeProperties::SetJoinMode(::JoinMode joinMode)
{
	if ((fSetProperties & STROKE_JOIN_MODE) != 0
		&& JoinMode() == joinMode) {
		return;
	}
	fJoinMode = joinMode;
	fSetProperties |= STROKE_JOIN_MODE;
	Notify();
}

// SetStrokePosition
void
StrokeProperties::SetStrokePosition(::StrokePosition position)
{
	if ((fSetProperties & STROKE_POSITION) != 0
		&& StrokePosition() == position) {
		return;
	}
	fStrokePosition = position;
	fSetProperties |= STROKE_POSITION;
	Notify();
}

// HashKey
size_t
StrokeProperties::HashKey() const
{
	// TODO: ...
//	return 0;
	return (size_t)((int)(fWidth * 10) ^ (int)(fMiterLimit * 10)
		^ fSetProperties ^ fCapMode ^ fJoinMode ^ fStrokePosition);
}

/*static*/ const StrokeProperties&
StrokeProperties::EmptyStrokeProperties()
{
	static StrokeProperties empty;
	return empty;
}

/*static*/ ::StrokePropertiesCache&
StrokeProperties::StrokePropertiesCache()
{
	static ::StrokePropertiesCache cache;
	return cache;
}
