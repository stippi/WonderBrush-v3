/*
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Distributed under the terms of the MIT License.
 */

#include "ALMArea.h"

#include <algorithm>	// for max

#include <Button.h>
#include <CheckBox.h>
#include <PictureButton.h>
#include <RadioButton.h>
#include <StatusBar.h>
#include <StringView.h>

#include <linprog/OperatorType.h>

#include "ALMLayout.h"


using namespace std;

BSize ALMArea::kMaxSize(INT_MAX, INT_MAX);
BSize ALMArea::kMinSize(0, 0);
BSize ALMArea::kUndefinedSize(-1, -1);


/**
 * Gets the auto preferred content size.
 *
 * @return the auto preferred content size
 */
bool
ALMArea::AutoPreferredContentSize() const
{
	return fAutoPreferredContentSize;
}


/**
 * Sets the auto preferred content size true or false.
 *
 * @param value	the auto preferred content size
 */
void
ALMArea::SetAutoPreferredContentSize(bool value)
{
	fAutoPreferredContentSize = value;
}


/**
 * Gets the left tab of the area.
 *
 * @return the left tab of the area
 */
ALMTab*
ALMArea::Left() const
{
	return fLeft;
}


/**
 * Sets the left tab of the area.
 *
 * @param left	the left tab of the area
 */
void
ALMArea::SetLeft(ALMTab* left)
{
	fLeft = left;

	// TODO: What about previous fColumn or previous fLeft?
	fColumn = NULL;

	if (fChildArea == NULL) {
		fMinContentWidth->SetLeftSide(-1.0, fLeft, 1.0, fRight);

		if (fMaxContentWidth != NULL)
			fMaxContentWidth->SetLeftSide(-1.0, fLeft, 1.0, fRight);
	} else
		UpdateHorizontal();
	fLS->InvalidateLayout();
}


/**
 * Gets the right tab of the area.
 *
 * @return the right tab of the area
 */
ALMTab*
ALMArea::Right() const
{
	return fRight;
}


/**
 * Sets the right tab of the area.
 *
 * @param right	the right tab of the area
 */
void
ALMArea::SetRight(ALMTab* right)
{
	fRight = right;

	// TODO: What about previous fColumn or previous fRight?
	fColumn = NULL;

	if (fChildArea == NULL) {
		fMinContentWidth->SetLeftSide(-1.0, fLeft, 1.0, fRight);

		if (fMaxContentWidth != NULL)
			fMaxContentWidth->SetLeftSide(-1.0, fLeft, 1.0, fRight);
	} else
		UpdateHorizontal();
	fLS->InvalidateLayout();
}


/**
 * Gets the top tab of the area.
 */
ALMTab*
ALMArea::Top() const
{
	return fTop;
}


/**
 * Sets the top tab of the area.
 */
void
ALMArea::SetTop(ALMTab* top)
{
	fTop = top;

	// TODO: What about previous fRow or previous fTop?
	fRow = NULL;

	if (fChildArea == NULL) {
		fMinContentHeight->SetLeftSide(-1.0, fTop, 1.0, fBottom);

		if (fMaxContentHeight != NULL)
			fMaxContentHeight->SetLeftSide(-1.0, fTop, 1.0, fBottom);
	} else
		UpdateVertical();
	fLS->InvalidateLayout();
}


/**
 * Gets the bottom tab of the area.
 */
ALMTab*
ALMArea::Bottom() const
{
	return fBottom;
}


/**
 * Sets the bottom tab of the area.
 */
void
ALMArea::SetBottom(ALMTab* bottom)
{
	fBottom = bottom;

	// TODO: What about previous fRow or previous fBottom?
	fRow = NULL;

	if (fChildArea == NULL) {
		fMinContentHeight->SetLeftSide(-1.0, fTop, 1.0, fBottom);

		if (fMaxContentHeight != NULL)
			fMaxContentHeight->SetLeftSide(-1.0, fTop, 1.0, fBottom);
	} else
		UpdateVertical();
	fLS->InvalidateLayout();
}


/**
 * Gets the span that defines the top and bottom tabs.
 */
ALMSpan*
ALMArea::GetRow() const
{
	return fRow;
}


/**
 * Sets the span that defines the top and bottom tabs.
 * May be null.
 */
void
ALMArea::SetRow(ALMSpan* span)
{
	SetTop(span->StartTab());
	SetBottom(span->EndTab());
	fRow = span;
	fLS->InvalidateLayout();
}


/**
 * Gets the column that defines the left and right tabs.
 */
ALMSpan*
ALMArea::GetColumn() const
{
	return fColumn;
}


/**
 * Sets the column that defines the left and right tabs.
 * May be null.
 */
void
ALMArea::SetColumn(ALMSpan* column)
{
	SetLeft(column->StartTab());
	SetRight(column->EndTab());
	fColumn = column;
	fLS->InvalidateLayout();
}


/**
 * Gets the control that is the content of the area.
 */
BView*
ALMArea::Content() const
{
	return (fChildArea == NULL) ? fContent : fChildArea->Content();
}


/**
 * Sets the control that is the content of the area.
 */
void
ALMArea::SetContent(BView* content)
{
	if (fChildArea == NULL) fContent = content;
	else fChildArea->fContent = content;
	fLS->InvalidateLayout();
}


/**
 * Left tab of the area's content. May be different from the left tab of the
 * area.
 */
ALMTab*
ALMArea::ContentLeft() const
{
	return (fChildArea == NULL) ? fLeft : fChildArea->fLeft;
}


/**
 * Top tab of the area's content. May be different from the top tab of the
 * area.
 */
ALMTab*
ALMArea::ContentTop() const
{
	return (fChildArea == NULL) ? fTop : fChildArea->fTop;
}


/**
 * Right tab of the area's content. May be different from the right tab of
 * the area.
 */
ALMTab*
ALMArea::ContentRight() const
{
	return (fChildArea == NULL) ? fRight : fChildArea->fRight;
}


/**
 * Bottom tab of the area's content. May be different from the bottom tab of
 * the area.
 */
ALMTab*
ALMArea::ContentBottom() const
{
	return (fChildArea == NULL) ? fBottom : fChildArea->fBottom;
}


/**
 * Gets minimum size of the area's content.
 */
BSize
ALMArea::MinContentSize() const
{
	return (fChildArea == NULL) ? fMinContentSize
		: fChildArea->fMinContentSize;
}


/**
 * Sets minimum size of the area's content.
 * May be different from the minimum size of the area.
 */
void
ALMArea::SetMinContentSize(BSize min)
{
	if (fChildArea == NULL) {
		fMinContentSize = min;
		fMinContentWidth->SetRightSide(fMinContentSize.Width());
		fMinContentHeight->SetRightSide(fMinContentSize.Height());
	} else
		fChildArea->SetMinContentSize(min);
	fLS->InvalidateLayout();
}


/**
 * Gets maximum size of the area's content.
 */
BSize ALMArea::MaxContentSize() const {
	return (fChildArea == NULL) ? fMaxContentSize
		: fChildArea->fMaxContentSize;
}


/**
 * Sets maximum size of the area's content.
 * May be different from the maximum size of the area.
 */
void
ALMArea::SetMaxContentSize(BSize max)
{
	if (fChildArea == NULL) {
		fMaxContentSize = max;
		if (fMaxContentWidth == NULL) {
			fMaxContentWidth = fLS->AddConstraint(-1.0, fLeft, 1.0, fRight,
				OperatorType(LE), fMaxContentSize.Width());
			fConstraints->AddItem(fMaxContentWidth);

			fMaxContentHeight = fLS->AddConstraint(-1.0, fTop, 1.0, fBottom,
				OperatorType(LE), fMaxContentSize.Height());
			fConstraints->AddItem(fMaxContentHeight);
		} else {
			fMaxContentWidth->SetRightSide(fMaxContentSize.Width());
			fMaxContentHeight->SetRightSide(fMaxContentSize.Height());
		}
	} else
		fChildArea->SetMaxContentSize(max);
	fLS->InvalidateLayout();
}


/**
 * Gets Preferred size of the area's content.
 */
BSize
ALMArea::PreferredContentSize() const
{
	return (fChildArea == NULL) ? fPreferredContentSize
		: fChildArea->fPreferredContentSize;
}


/**
 * Sets Preferred size of the area's content.
 * May be different from the preferred size of the area.
 * Manual changes of PreferredContentSize are ignored unless
 * autoPreferredContentSize is set to false.
 */
void
ALMArea::SetPreferredContentSize(BSize preferred)
{
	if (fChildArea == NULL) {
		fPreferredContentSize = preferred;
		if (fPreferredContentWidth == NULL) {
			fPreferredContentWidth = fLS->AddConstraint(
				-1.0, fLeft, 1.0, fRight, OperatorType(EQ),
				fPreferredContentSize.Width(), fShrinkPenalties.Width(),
				fGrowPenalties.Width());
			fConstraints->AddItem(fPreferredContentWidth);

			fPreferredContentHeight = fLS->AddConstraint(
				-1.0, fTop, 1.0, fBottom, OperatorType(EQ),
				fPreferredContentSize.Height(), fShrinkPenalties.Height(),
				fGrowPenalties.Height());
			fConstraints->AddItem(fPreferredContentHeight);
		} else {
			fPreferredContentWidth->SetRightSide(preferred.Width());
			fPreferredContentHeight->SetRightSide(preferred.Height());
		}
	} else
		fChildArea->SetPreferredContentSize(preferred);
	fLS->InvalidateLayout();
}


/**
 * The reluctance with which the area's content shrinks below its preferred
 * size. The bigger the less likely is such shrinking.
 */
BSize
ALMArea::ShrinkPenalties() const
{
	return (fChildArea == NULL) ? fShrinkPenalties
		: fChildArea->fShrinkPenalties;
}


void ALMArea::SetShrinkPenalties(BSize shrink) {
	if (fChildArea == NULL) {
		fShrinkPenalties = shrink;
		if (fPreferredContentWidth != NULL) {
			fPreferredContentWidth->SetPenaltyNeg(shrink.Width());
			fPreferredContentHeight->SetPenaltyNeg(shrink.Height());
		}
	} else
		fChildArea->SetShrinkPenalties(shrink);
	fLS->InvalidateLayout();
}


/**
 * The reluctance with which the area's content grows over its preferred size.
 * The bigger the less likely is such growth.
 */
BSize
ALMArea::GrowPenalties() const
{
	return (fChildArea == NULL) ? fGrowPenalties : fChildArea->fGrowPenalties;
}


void
ALMArea::SetGrowPenalties(BSize grow)
{
	if (fChildArea == NULL) {
		fGrowPenalties = grow;
		if (fPreferredContentWidth != NULL) {
			fPreferredContentWidth->SetPenaltyPos(grow.Width());
			fPreferredContentHeight->SetPenaltyPos(grow.Height());
		}
	} else
		fChildArea->SetGrowPenalties(grow);
	fLS->InvalidateLayout();
}


/**
 * Gets aspect ratio of the area's content.
 */
double
ALMArea::ContentAspectRatio() const
{
	return (fChildArea == NULL) ? fContentAspectRatio
		: fChildArea->fContentAspectRatio;
}


/**
 * Sets aspect ratio of the area's content.
 * May be different from the aspect ratio of the area.
 */
void
ALMArea::SetContentAspectRatio(double ratio)
{
	if (fChildArea == NULL) {
		fContentAspectRatio = ratio;
		if (fContentAspectRatioC == NULL) {
			fContentAspectRatioC = fLS->AddConstraint(
				-1.0, fLeft, 1.0, fRight, ratio, fTop, -ratio, fBottom,
				OperatorType(EQ), 0.0);
			fConstraints->AddItem(fContentAspectRatioC);
		} else {
			fContentAspectRatioC->SetLeftSide(
				-1.0, fLeft, 1.0, fRight, ratio, fTop, -ratio, fBottom);
		}
	} else
		fChildArea->SetContentAspectRatio(ratio);
	fLS->InvalidateLayout();
}


/**
 * Gets alignment of the content in its area.
 */
BAlignment
ALMArea::Alignment() const
{
	return fAlignment;
}


/**
 * Sets alignment of the content in its area.
 */
void
ALMArea::SetAlignment(BAlignment alignment)
{
	fAlignment = alignment;
	UpdateHorizontal();
	UpdateVertical();
	fLS->InvalidateLayout();
}


/**
 * Sets horizontal alignment of the content in its area.
 */
void ALMArea::SetHorizontalAlignment(alignment horizontal) {
	fAlignment.SetHorizontal(horizontal);
	UpdateHorizontal();
	fLS->InvalidateLayout();
}


/**
 * Sets vertical alignment of the content in its area.
 */
void
ALMArea::SetVerticalAlignment(vertical_alignment vertical)
{
	fAlignment.SetVertical(vertical);
	UpdateVertical();
	fLS->InvalidateLayout();
}


/**
 * Gets left inset between area and its content.
 */
int32
ALMArea::LeftInset() const
{
	return fLeftInset;
}


/**
 * Sets left inset between area and its content.
 */
void
ALMArea::SetLeftInset(int32 left)
{
	fLeftInset = left;
	UpdateHorizontal();
	fLS->InvalidateLayout();
}


/**
 * Gets top inset between area and its content.
 */
int32
ALMArea::TopInset() const
{
	return fTopInset;
}


/**
 * Sets top inset between area and its content.
 */
void
ALMArea::SetTopInset(int32 top)
{
	fTopInset = top;
	UpdateVertical();
	fLS->InvalidateLayout();
}


/**
 * Gets right inset between area and its content.
 */
int32
ALMArea::RightInset() const
{
	return fRightInset;
}


/**
 * Sets right inset between area and its content.
 */
void
ALMArea::SetRightInset(int32 right)
{
	fRightInset = right;
	UpdateHorizontal();
}


/**
 * Gets bottom inset between area and its content.
 */
int32
ALMArea::BottomInset() const
{
	return fBottomInset;
}


/**
 * Sets bottom inset between area and its content.
 */
void
ALMArea::SetBottomInset(int32 bottom)
{
	fBottomInset = bottom;
	UpdateVertical();
}


/**
 * Sets the preferred size according to the content's PreferredSize method,
 * and the penalties according to heuristics.
 */
void
ALMArea::SetDefaultBehavior()
{
	if (Content() == NULL) {
		SetPreferredContentSize(BSize(0, 0));
		SetShrinkPenalties(BSize(0, 0));
		SetGrowPenalties(BSize(0, 0));
		return;
	}

	if (PreferredContentSize() != Content()->PreferredSize()){
		SetPreferredContentSize(Content()->PreferredSize());
		fLS->InvalidateLayout();
	}

	if (dynamic_cast<BButton*>(Content()) != NULL
		|| dynamic_cast<BRadioButton*>(Content()) != NULL
		|| dynamic_cast<BCheckBox*>(Content()) != NULL
		|| dynamic_cast<BStringView*>(Content()) != NULL
		|| dynamic_cast<BPictureButton*>(Content()) != NULL
		|| dynamic_cast<BStatusBar*>(Content()) != NULL) {
		fShrinkPenalties = BSize(4, 4);
		fGrowPenalties = BSize(3, 3);
	} else {
		fShrinkPenalties = BSize(2, 2);
		fGrowPenalties = BSize(1, 1);
	}
}


ALMArea::operator BString() const
{
	BString string;
	GetString(string);
	return string;
}


void
ALMArea::GetString(BString& string) const
{
	string << "ALMArea(";
	fLeft->GetString(string);
	string << ", ";
	fTop->GetString(string);
	string << ", ";
	fRight->GetString(string);
	string << ", ";
	fBottom->GetString(string);
	string << ")";
}


/**
 * Sets the width of the area to be the same as the width of the given area.
 *
 * @param area	the area that should have the same width
 * @return the same-width constraint
 */
Constraint*
ALMArea::HasSameWidthAs(ALMArea* area)
{
	return fLS->AddConstraint(
		-1.0, fLeft, 1.0, fRight, 1.0, area->fLeft, -1.0, area->fRight,
		OperatorType(EQ), 0.0);
}


/**
 * Sets the height of the area to be the same as the height of the given area.
 *
 * @param area	the area that should have the same height
 * @return the same-height constraint
 */
Constraint*
ALMArea::HasSameHeightAs(ALMArea* area)
{
	return fLS->AddConstraint(
		-1.0, fTop, 1.0, fBottom, 1.0, area->fTop, -1.0, area->fBottom,
		OperatorType(EQ), 0.0);
}


/**
 * Sets the size of the area to be the same as the size of the given area.
 *
 * @param area	the area that should have the same size
 * @return a list containing a same-width and same-height constraint
 */
BList*
ALMArea::HasSameSizeAs(ALMArea* area)
{
	BList* constraints = new BList(2);
	constraints->AddItem(this->HasSameWidthAs(area));
	constraints->AddItem(this->HasSameHeightAs(area));
	return constraints;
}


/**
 * Destructor.
 * Removes the area from its specification.
 */
ALMArea::~ALMArea()
{
	if (fChildArea != NULL) delete fChildArea;
	for (int32 i = 0; i < fConstraints->CountItems(); i++)
		delete (Constraint*)fConstraints->ItemAt(i);
	fLS->Areas()->RemoveItem(this);
}


/**
 * Constructor.
 * Uses XTabs and YTabs.
 */
ALMArea::ALMArea(ALMLayout* ls, ALMTab* left, ALMTab* top, ALMTab* right,
	ALMTab* bottom, BView* content, BSize minContentSize)
{
	Init(ls, left, top, right, bottom, content, minContentSize);
}


/**
 * Constructor.
 * Uses Rows and Columns.
 */
ALMArea::ALMArea(ALMLayout* ls, ALMSpan* row, ALMSpan* column, BView* content,
	BSize minContentSize)
{
	Init(ls, column->StartTab(), row->StartTab(), column->EndTab(),
		row->EndTab(), content, minContentSize);
	fRow = row;
	fColumn = column;
}


/**
 * Initialize variables.
 */
void
ALMArea::Init(ALMLayout* ls, ALMTab* left, ALMTab* top, ALMTab* right,
	ALMTab* bottom, BView* content, BSize minContentSize)
{
	fConstraints = new BList(2);
	fMaxContentSize = kMaxSize;

	fMaxContentWidth = NULL;
	fMaxContentHeight = NULL;

	fPreferredContentSize = kUndefinedSize;
	fShrinkPenalties = BSize(2, 2);
	fGrowPenalties = BSize(1, 1);
	fContentAspectRatio = 0;
	fContentAspectRatioC = NULL;

	fAutoPreferredContentSize = false;

	fPreferredContentWidth = NULL;
	fPreferredContentHeight = NULL;

	fChildArea = NULL;

	fAlignment = BAlignment(B_ALIGN_USE_FULL_WIDTH, B_ALIGN_USE_FULL_HEIGHT);
	fLeftInset = 0;
	fTopInset = 0;
	fRightInset = 0;
	fBottomInset = 0;

	fLeftConstraint = NULL;
	fTopConstraint = NULL;
	fRightConstraint = NULL;
	fBottomConstraint = NULL;

	fLS = ls;
	fLeft = left;
	fRight = right;
	fTop = top;
	fBottom = bottom;
	SetContent(content);
	fMinContentSize = minContentSize;

	// adds the two essential constraints of the area that make sure that the
	// left x-tab is really to the left of the right x-tab, and the top y-tab
	// really above the bottom y-tab
	fMinContentWidth = ls->AddConstraint(-1.0, left, 1.0, right,
		OperatorType(GE), minContentSize.Width());
	fConstraints->AddItem(fMinContentWidth);

	fMinContentHeight = ls->AddConstraint(-1.0, top, 1.0, bottom,
		OperatorType(GE), minContentSize.Height());
	fConstraints->AddItem(fMinContentHeight);
}


/**
 * Perform layout on the area.
 */
void ALMArea::DoLayout()
{
	if (Content() == NULL)
		return; // empty areas need no layout

	// If there is a childArea, then it is the childArea that actually contains
	// the content.
	ALMArea* area = (fChildArea != NULL) ? fChildArea : this;

	// set content location and size
	area->Content()->MoveTo(floor(area->Left()->Value() + 0.5),
			floor(area->Top()->Value() + 0.5));
	int32 width = (int32)floor(area->Right()->Value() - area->Left()->Value()
		+ 0.5);
	int32 height = (int32)floor(area->Bottom()->Value() - area->Top()->Value()
		+ 0.5);
	area->Content()->ResizeTo(width, height);
}


/**
 * Adds a childArea to this area, together with constraints that specify the
 * relative location of the childArea within this area. It is called when
 * such a childArea becomes necessary, i.e. when the user requests insets or
 * special alignment.
 */
void
ALMArea::InitChildArea()
{
	// add a child area with new tabs,
	// and add constraints that set its tabs to be equal to the
	// coresponding tabs of this area (for a start)
	fChildArea = new ALMArea(fLS, new ALMTab(fLS), new ALMTab(fLS),
		new ALMTab(fLS), new ALMTab(fLS), fContent, BSize(0, 0));
	fLeftConstraint = fLeft->IsEqual(fChildArea->Left());
	fConstraints->AddItem(fLeftConstraint);
	fTopConstraint = fTop->IsEqual(fChildArea->Top());
	fConstraints->AddItem(fTopConstraint);
	fRightConstraint = fRight->IsEqual(fChildArea->Right());
	fConstraints->AddItem(fRightConstraint);
	fBottomConstraint = fBottom->IsEqual(fChildArea->Bottom());
	fConstraints->AddItem(fBottomConstraint);

	// remove the minimum content size constraints from this area
	// and copy the minimum content size setting to the childArea
	fConstraints->RemoveItem(fMinContentWidth);
	delete fMinContentWidth;
	fMinContentWidth = fChildArea->fMinContentWidth;
	fConstraints->RemoveItem(fMinContentHeight);
	delete fMinContentHeight;
	fMinContentHeight = fChildArea->fMinContentHeight;
	fChildArea->SetMinContentSize(fMinContentSize);

	// if there are maximum content size constraints on this area,
	// change them so that they refer to the tabs of the childArea
	// and copy the minimum content size settings to the childArea
	if (fMaxContentWidth != NULL) {
		fChildArea->fMaxContentSize = fMaxContentSize;

		fChildArea->fMaxContentWidth = fMaxContentWidth;
		fMaxContentWidth->SetLeftSide(
			-1.0, fChildArea->Left(), 1.0, fChildArea->Right());

		fChildArea->fMaxContentHeight = fMaxContentHeight;
		fMaxContentHeight->SetLeftSide(
			-1.0, fChildArea->Top(), 1.0, fChildArea->Bottom());
	}

	// if there are preferred content size constraints on this area,
	// change them so that they refer to the tabs of the childArea
	// and copy the preferred content size settings to the childArea
	if (fPreferredContentHeight != NULL) {
		fChildArea->fPreferredContentSize = fPreferredContentSize;
		fChildArea->fShrinkPenalties = fShrinkPenalties;
		fChildArea->fGrowPenalties = fGrowPenalties;

		fChildArea->fPreferredContentWidth = fPreferredContentWidth;
		fPreferredContentWidth->SetLeftSide(
			-1.0, fChildArea->Left(), 1.0, fChildArea->Right());

		fChildArea->fPreferredContentHeight = fPreferredContentHeight;
		fPreferredContentHeight->SetLeftSide(
			-1.0, fChildArea->Top(), 1.0, fChildArea->Bottom());
	}
}


/**
 * Update the constraints for horizontal insets and alignment.
 */
void
ALMArea::UpdateHorizontal()
{
	// if the area does not have a childAdrea yet, this is the time to add it
	if (fChildArea == NULL)
		InitChildArea();

	// change the constraints leftConstraint and rightConstraint so that the
	// horizontal alignment and insets of the childArea within this area are
	// as specified by the user
	if (fAlignment.Horizontal() == B_ALIGN_LEFT) {
		fLeftConstraint->SetLeftSide(-1.0, fLeft, 1.0, fChildArea->Left());
		fLeftConstraint->SetOp(OperatorType(EQ));
		fLeftConstraint->SetRightSide(fLeftInset);

		fRightConstraint->SetLeftSide(-1.0, fChildArea->Right(), 1.0, fRight);
		fRightConstraint->SetOp(OperatorType(GE));
		fRightConstraint->SetRightSide(fRightInset);
	} else if (fAlignment.Horizontal() == B_ALIGN_RIGHT) {
		fLeftConstraint->SetLeftSide(-1.0, fLeft, 1.0, fChildArea->Left());
		fLeftConstraint->SetOp(OperatorType(GE));
		fLeftConstraint->SetRightSide(fLeftInset);

		fRightConstraint->SetLeftSide(-1.0, fChildArea->Right(), 1.0, fRight);
		fRightConstraint->SetOp(OperatorType(EQ));
		fRightConstraint->SetRightSide(fRightInset);
	} else if (fAlignment.Horizontal() == B_ALIGN_HORIZONTAL_CENTER) {
		fLeftConstraint->SetLeftSide(-1.0, fLeft, 1.0, fChildArea->Left());
		fLeftConstraint->SetOp(OperatorType(GE));
		fLeftConstraint->SetRightSide(max(fLeftInset, fRightInset));

		fRightConstraint->SetLeftSide(-1.0, fLeft, 1.0, fChildArea->Left(),
			1.0, fChildArea->Right(), -1.0, fRight);
		fRightConstraint->SetOp(OperatorType(EQ));
		fRightConstraint->SetRightSide(0);
	} else if (fAlignment.Horizontal() == B_ALIGN_USE_FULL_WIDTH) {
		fLeftConstraint->SetLeftSide(-1.0, fLeft, 1.0, fChildArea->Left());
		fLeftConstraint->SetOp(OperatorType(EQ));
		fLeftConstraint->SetRightSide(fLeftInset);

		fRightConstraint->SetLeftSide(-1.0, fChildArea->Right(), 1.0, fRight);
		fRightConstraint->SetOp(OperatorType(EQ));
		fRightConstraint->SetRightSide(fRightInset);
	} else if (fAlignment.Horizontal() == B_ALIGN_HORIZONTAL_UNSET) {
		fLeftConstraint->SetLeftSide(-1.0, fLeft, 1.0, fChildArea->Left());
		fLeftConstraint->SetOp(OperatorType(GE));
		fLeftConstraint->SetRightSide(fLeftInset);

		fRightConstraint->SetLeftSide(-1.0, fChildArea->Right(), 1.0, fRight);
		fRightConstraint->SetOp(OperatorType(GE));
		fRightConstraint->SetRightSide(fRightInset);
	}
}


/**
 * Update the constraints for vertical insets and alignment.
 */
void ALMArea::UpdateVertical() {
	// if the area does not have a childAdrea yet, this is the time to add it
	if (fChildArea == NULL)
		InitChildArea();

	// change the constraints topConstraint and bottomConstraint so that the
	// vertical alignment and insets of the childArea within this area are as
	// specified by the user
	if (fAlignment.Vertical() == B_ALIGN_TOP) {
		fTopConstraint->SetLeftSide(-1.0, fTop, 1.0, fChildArea->Top());
		fTopConstraint->SetOp(OperatorType(EQ));
		fTopConstraint->SetRightSide(fTopInset);

		fBottomConstraint->SetLeftSide(-1.0, fChildArea->Bottom(), 1.0,
			fBottom);
		fBottomConstraint->SetOp(OperatorType(GE));
		fBottomConstraint->SetRightSide(fBottomInset);
	} else if (fAlignment.Vertical() == B_ALIGN_BOTTOM) {
		fTopConstraint->SetLeftSide(-1.0, fTop, 1.0, fChildArea->Top());
		fTopConstraint->SetOp(OperatorType(GE));
		fTopConstraint->SetRightSide(fTopInset);

		fBottomConstraint->SetLeftSide(-1.0, fChildArea->Bottom(), 1.0,
			fBottom);
		fBottomConstraint->SetOp(OperatorType(EQ));
		fBottomConstraint->SetRightSide(fBottomInset);
	} else if (fAlignment.Vertical() == B_ALIGN_VERTICAL_CENTER) {
		fTopConstraint->SetLeftSide(-1.0, fTop, 1.0, fChildArea->Top());
		fTopConstraint->SetOp(OperatorType(GE));
		fTopConstraint->SetRightSide(max(fTopInset, fBottomInset));

		fBottomConstraint->SetLeftSide(-1.0, fTop, 1.0, fChildArea->Top(),
			1.0, fChildArea->Bottom(), -1.0, fBottom);
		fBottomConstraint->SetOp(OperatorType(EQ));
		fBottomConstraint->SetRightSide(0);
	} else if (fAlignment.Vertical() == B_ALIGN_USE_FULL_HEIGHT) {
		fTopConstraint->SetLeftSide(-1.0, fTop, 1.0, fChildArea->Top());
		fTopConstraint->SetOp(OperatorType(EQ));
		fTopConstraint->SetRightSide(fTopInset);

		fBottomConstraint->SetLeftSide(-1.0, fChildArea->Bottom(), 1.0,
			fBottom);
		fBottomConstraint->SetOp(OperatorType(EQ));
		fBottomConstraint->SetRightSide(fBottomInset);
	} else if (fAlignment.Vertical() == B_ALIGN_VERTICAL_UNSET) {
		fTopConstraint->SetLeftSide(-1.0, fTop, 1.0, fChildArea->Top());
		fTopConstraint->SetOp(OperatorType(GE));
		fTopConstraint->SetRightSide(fTopInset);

		fBottomConstraint->SetLeftSide(-1.0, fChildArea->Bottom(), 1.0,
			fBottom);
		fBottomConstraint->SetOp(OperatorType(GE));
		fBottomConstraint->SetRightSide(fBottomInset);
	}
}

