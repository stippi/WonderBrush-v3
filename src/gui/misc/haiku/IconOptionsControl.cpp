/*
 * Copyright 2001-2009, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2006-2008, Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IconOptionsControl.h"

#include <stdio.h>

#include <new>

#include <AbstractLayoutItem.h>
#include <ControlLook.h>
#include <GroupView.h>
#include <LayoutUtils.h>
#include <Window.h>

#include "IconButton.h"


struct IconOptionsControl::LayoutData {
	LayoutData()
		: label_layout_item(NULL),
		  icon_bar_layout_item(NULL),
		  valid(false)
	{
	}

	LabelLayoutItem*	label_layout_item;
	IconBarLayoutItem*	icon_bar_layout_item;
	font_height			font_info;
	float				label_width;
	float				label_height;
	BSize				min;
	BSize				icon_bar_min;
	bool				valid;
};


class IconOptionsControl::LabelLayoutItem : public BAbstractLayoutItem {
public:
	LabelLayoutItem(IconOptionsControl* parent)
		:
		fParent(parent),
		fFrame()
	{
	}

	virtual bool IsVisible()
	{
		return !fParent->IsHidden(fParent);
	}

	virtual void SetVisible(bool visible)
	{
		// not allowed
	}

	virtual BRect Frame()
	{
		return fFrame;
	}
	virtual void SetFrame(BRect frame)
	{
		fFrame = frame;
		fParent->_UpdateFrame();
	}

	virtual BView* View()
	{
		return fParent;
	}

	virtual BSize BaseMinSize()
	{
		fParent->_ValidateLayoutData();

		if (!fParent->Label())
			return BSize(-1, -1);

		if (fParent->fOrientation == B_HORIZONTAL) {
			return BSize(fParent->fLayoutData->label_width,
				fParent->fLayoutData->label_height);
		} else {
			return BSize(fParent->fLayoutData->label_height,
				fParent->fLayoutData->label_width);
		}
	}

	virtual BSize BaseMaxSize()
	{
		return BaseMinSize();
	}

	virtual BSize BasePreferredSize()
	{
		return BaseMinSize();
	}

	virtual BAlignment BaseAlignment()
	{
		return BAlignment(B_ALIGN_USE_FULL_WIDTH, B_ALIGN_USE_FULL_HEIGHT);
	}

private:
	IconOptionsControl*	fParent;
	BRect				fFrame;
};


class IconOptionsControl::IconBarLayoutItem : public BAbstractLayoutItem {
public:
	IconBarLayoutItem(IconOptionsControl* parent)
		:
		fParent(parent),
		fFrame()
	{
	}

	virtual bool IsVisible()
	{
		return !fParent->IsHidden(fParent);
	}

	virtual void SetVisible(bool visible)
	{
		// not allowed
	}

	virtual BRect Frame()
	{
		return fFrame;
	}

	virtual void SetFrame(BRect frame)
	{
		fFrame = frame;
		fParent->_UpdateFrame();
	}

	virtual BView* View()
	{
		return fParent;
	}

	virtual BSize BaseMinSize()
	{
		fParent->_ValidateLayoutData();
		return fParent->fLayoutData->icon_bar_min;;
	}

	virtual BSize BaseMaxSize()
	{
		BSize size(BaseMinSize());
		if (fParent->fOrientation == B_HORIZONTAL)
			size.width = B_SIZE_UNLIMITED;
		else
			size.height = B_SIZE_UNLIMITED;
		return size;
	}

	virtual BSize BasePreferredSize()
	{
		return BaseMinSize();
	}

	virtual BAlignment BaseAlignment()
	{
		return BAlignment(B_ALIGN_USE_FULL_WIDTH, B_ALIGN_USE_FULL_HEIGHT);
	}

private:
	IconOptionsControl*	fParent;
	BRect				fFrame;
};


// #pragma mark -


// constructor
IconOptionsControl::IconOptionsControl(const char* name, const char* label,
	BMessage* message, BHandler* target, enum orientation orientation)
	:
	BControl(name, label, message, B_WILL_DRAW | B_FRAME_EVENTS),
	fTargetCache(target),
	fOrientation(orientation),
	fLayoutData(new(std::nothrow) LayoutData),
	fIconGroup(new(std::nothrow) BGroupView(orientation, 0.0f))
{
	fIconGroup->SetLowColor(fIconGroup->ViewColor());
	AddChild(fIconGroup);
}

// destructor
IconOptionsControl::~IconOptionsControl()
{
	delete fLayoutData;
}

// AttachedToWindow
void
IconOptionsControl::AttachedToWindow()
{
	BControl::AttachedToWindow();

	SetViewColor(B_TRANSPARENT_32_BIT);
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

// AllAttached
void
IconOptionsControl::AllAttached()
{
	for (int32 i = 0; IconButton* button = _FindIcon(i); i++)
		button->SetTarget(this);
	if (fTargetCache != NULL)
		SetTarget(fTargetCache);
}

// Draw
void
IconOptionsControl::Draw(BRect updateRect)
{
	FillRect(updateRect, B_SOLID_LOW);

	if (Label()) {
		if (!IsEnabled())
			SetHighColor(tint_color(LowColor(), B_DISABLED_LABEL_TINT));
		else
			SetHighColor(tint_color(LowColor(), B_DARKEN_MAX_TINT));

		font_height fh;
		GetFontHeight(&fh);
		BPoint p(Bounds().LeftTop());
		p.y += floorf(Bounds().Height() / 2.0 + (fh.ascent + fh.descent) / 2.0) - 2.0;
		DrawString(Label(), p);
	}
}

// FrameResized
void
IconOptionsControl::FrameResized(float width, float height)
{
	BControl::FrameResized(width, height);
}

// GetPreferredSize
void
IconOptionsControl::GetPreferredSize(float *_width, float *_height)
{
	_ValidateLayoutData();

	if (_width)
		*_width = fLayoutData->min.width;

	if (_height)
		*_height = fLayoutData->min.height;
}

// MinSize
BSize
IconOptionsControl::MinSize()
{
	_ValidateLayoutData();
	return BLayoutUtils::ComposeSize(ExplicitMinSize(), fLayoutData->min);
}

// MaxSize
BSize
IconOptionsControl::MaxSize()
{
	_ValidateLayoutData();

	BSize max = fLayoutData->min;
	max.width = B_SIZE_UNLIMITED;

	return BLayoutUtils::ComposeSize(ExplicitMaxSize(), max);
}

// PreferredSize
BSize
IconOptionsControl::PreferredSize()
{
	_ValidateLayoutData();
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(), fLayoutData->min);
}

// InvalidateLayout
void
IconOptionsControl::InvalidateLayout(bool descendants)
{
	fLayoutData->valid = false;
	BView::InvalidateLayout(descendants);
}

// CreateLabelLayoutItem
BLayoutItem*
IconOptionsControl::CreateLabelLayoutItem()
{
	if (fLayoutData->label_layout_item == NULL) {
		fLayoutData->label_layout_item
			= new(std::nothrow) LabelLayoutItem(this);
	}
	return fLayoutData->label_layout_item;
}

// CreateIconBarLayoutItem
BLayoutItem*
IconOptionsControl::CreateIconBarLayoutItem()
{
	if (fLayoutData->icon_bar_layout_item == NULL) {
		fLayoutData->icon_bar_layout_item = new IconBarLayoutItem(this);
	}
	return fLayoutData->icon_bar_layout_item;
}

// SetValue
void
IconOptionsControl::SetValue(int32 value)
{
	if (IconButton* valueButton = _FindIcon(value)) {
		for (int32 i = 0; IconButton* button = _FindIcon(i); i++) {
			button->SetPressed(button == valueButton);
		}
	}
	BControl::SetValueNoUpdate(value);
}

// SetEnabled
void
IconOptionsControl::SetEnabled(bool enable)
{
	for (int32 i = 0; IconButton* button = _FindIcon(i); i++) {
		button->SetEnabled(enable);
	}
	BControl::SetEnabled(enable);
	Invalidate();
}

// MessageReceived
void
IconOptionsControl::MessageReceived(BMessage* message)
{
	// catch a message from the attached IconButtons to
	// handle switching the pressed icon
	BView* source;
	if (message->FindPointer("be:source", (void**)&source) >= B_OK) {
		if (IconButton* sourceIcon = dynamic_cast<IconButton*>(source)) {
			for (int32 i = 0; IconButton* button = _FindIcon(i); i++) {
				if (button == sourceIcon) {
					SetValue(i);
					break;
				}
			}
			// forward the message
			Invoke(message);
			return;
		}
	}
	BControl::MessageReceived(message);
}

// Invoke
status_t
IconOptionsControl::Invoke(BMessage* message)
{
	return BInvoker::Invoke(message);
}

// AddOption
void
IconOptionsControl::AddOption(IconButton* icon)
{
	if (icon == NULL)
		return;

	// first icon added, mark it
	icon->SetPressed(_FindIcon(0) == NULL);

	fIconGroup->GroupLayout()->AddView(icon);
	icon->SetTarget(this);
}

// DoLayout
void
IconOptionsControl::DoLayout()
{
	BControl::DoLayout();
	_UpdateFrame();
}

// _FindIcon
IconButton*
IconOptionsControl::_FindIcon(int32 index) const
{
	if (BView* view = fIconGroup->ChildAt(index))
		return dynamic_cast<IconButton*>(view);
	return NULL;
}

// _ValidateLayoutData
void
IconOptionsControl::_ValidateLayoutData()
{
	if (fLayoutData->valid)
		return;

	// cache font height
	font_height& fh = fLayoutData->font_info;
	GetFontHeight(&fh);

	if (Label() != NULL) {
		fLayoutData->label_width = ceilf(StringWidth(Label()));
		if (fOrientation == B_HORIZONTAL)
			fLayoutData->label_width += be_control_look->DefaultLabelSpacing();
		fLayoutData->label_height = ceilf(fh.ascent) + ceilf(fh.descent);
	} else {
		fLayoutData->label_width = 0;
		fLayoutData->label_height = 0;
	}

	// get the minimal (== preferred) icon bar size
	fLayoutData->icon_bar_min = fIconGroup->MinSize();

	// compute our minimal (== preferred) size
	BSize min(fLayoutData->icon_bar_min);

	if (fOrientation == B_HORIZONTAL) {
		// horizontal layout looks like this
		// Label_[][][][][] // _ == divider
		if (fLayoutData->label_width > 0)
			min.width += fLayoutData->label_width;
		if (fLayoutData->label_height > min.height)
			min.height = fLayoutData->label_height;
	} else {
		// vertical layout looks like this
		//  []
		//  []
		// l[]
		// e[]
		// b[]
		// a[]
		// L[] // no divider
		if (fLayoutData->label_width > 0) {
			min.width += fLayoutData->label_height;
			if (fLayoutData->label_width > min.height)
				min.height = fLayoutData->label_width;
		}
	}

	fLayoutData->min = min;

	fLayoutData->valid = true;
	ResetLayoutInvalidation();
}

// _UpdateFrame
void
IconOptionsControl::_UpdateFrame()
{
	if (fLayoutData->label_layout_item && fLayoutData->icon_bar_layout_item) {
		BRect labelFrame = fLayoutData->label_layout_item->Frame();
		BRect iconFrame = fLayoutData->icon_bar_layout_item->Frame();

		// update our frame
		MoveTo(labelFrame.left, min_c(labelFrame.top, iconFrame.top));
		ResizeTo(labelFrame.Width() + iconFrame.Width(),
			max_c(labelFrame.Height(), iconFrame.Height()));

		fIconGroup->MoveTo(iconFrame.LeftTop());
		fIconGroup->ResizeTo(iconFrame.Width(), iconFrame.Height());
	} else {
		fIconGroup->MoveTo(0.0f, 0.0f);
		fIconGroup->ResizeTo(Bounds().Width(), Bounds().Height());
	}
}


