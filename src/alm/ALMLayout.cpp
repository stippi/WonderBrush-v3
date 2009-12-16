/*
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Distributed under the terms of the MIT License.
 */

#include "ALMLayout.h"

#include <math.h>		// for floor

#include <linprog/ResultType.h>

#include "ALMArea.h"
#include "ALMSpan.h"
#include "ALMTab.h"


/**
 * Constructor.
 * Creates new layout engine.
 */
ALMLayout::ALMLayout()
	: BLayout(),
	LinearSpec()
{
	fLayoutStyle = FIT_TO_SIZE;
	fActivated = true;

	fAreas = new BList(1);
	fLeft = new ALMTab(this);
	fRight = new ALMTab(this);
	fTop = new ALMTab(this);
	fBottom = new ALMTab(this);

	// the Left tab is always at x-position 0, and the Top tab is always at y-position 0
	fLeft->SetRange(0, 0);
	fTop->SetRange(0, 0);

	// cached layout values
	// need to be invalidated whenever the layout specification is changed
	fMinSize = ALMArea::kUndefinedSize;
	fMaxSize = ALMArea::kUndefinedSize;
	fPreferredSize = ALMArea::kUndefinedSize;

	fPerformancePath = NULL;
}


/**
 * Solves the layout.
 */
void
ALMLayout::SolveLayout()
{
	// if autoPreferredContentSize is set on an area,
	// readjust its preferredContentSize and penalties settings
	int32 sizeAreas = fAreas->CountItems();
	ALMArea* currentArea;
	for (int32 i = 0; i < sizeAreas; i++) {
		currentArea = (ALMArea*)fAreas->ItemAt(i);
		if (currentArea->AutoPreferredContentSize())
			currentArea->SetDefaultBehavior();
	}

	// Try to solve the layout until the result is OPTIMAL or INFEASIBLE,
	// maximally 15 tries sometimes the solving algorithm encounters numerical
	// problems (NUMFAILURE), and repeating the solving often helps to overcome
	// them.
	BFile* file = NULL;
	if (fPerformancePath != NULL) {
		file = new BFile(fPerformancePath,
			B_READ_WRITE | B_CREATE_FILE | B_OPEN_AT_END);
	}

	ResultType result;
	for (int32 tries = 0; tries < 15; tries++) {
		result = Solve();
		if (fPerformancePath != NULL) {
			char buffer [100];
			file->Write(buffer, sprintf(buffer, "%d\t%fms\t#vars=%ld\t"
				"#constraints=%ld\n", result, SolvingTime(),
				Variables()->CountItems(), Constraints()->CountItems()));
		}
		if (result == OPTIMAL || result == INFEASIBLE)
			break;
	}
	delete file;
}


/**
 * Adds a new x-tab to the specification.
 *
 * @return the new x-tab
 */
ALMTab*
ALMLayout::AddXTab()
{
	return new ALMTab(this);
}


/**
 * Adds a new y-tab to the specification.
 *
 * @return the new y-tab
 */
ALMTab*
ALMLayout::AddYTab()
{
	return new ALMTab(this);
}


/**
 * Adds a new span to the specification.
 *
 * @return the new span
 */
ALMSpan*
ALMLayout::AddRow()
{
	return new ALMSpan(this);
}


/**
 * Adds a new span to the specification that is glued to the given y-tabs.
 *
 * @param top
 * @param bottom
 * @return the new span
 */
ALMSpan*
ALMLayout::AddRow(ALMTab* top, ALMTab* bottom)
{
	ALMSpan* span = new ALMSpan(this);
	if (top != NULL)
		span->Constraints()->AddItem(span->StartTab()->IsEqual(top));
	if (bottom != NULL)
		span->Constraints()->AddItem(span->EndTab()->IsEqual(bottom));
	return span;
}


/**
 * Adds a new column to the specification.
 *
 * @return the new column
 */
ALMSpan*
ALMLayout::AddColumn()
{
	return new ALMSpan(this);
}


/**
 * Adds a new column to the specification that is glued to the given x-tabs.
 *
 * @param left
 * @param right
 * @return the new column
 */
ALMSpan*
ALMLayout::AddColumn(ALMTab* left, ALMTab* right)
{
	ALMSpan* column = new ALMSpan(this);
	if (left != NULL)
		column->Constraints()->AddItem(column->StartTab()->IsEqual(left));
	if (right != NULL)
		column->Constraints()->AddItem(column->EndTab()->IsEqual(right));
	return column;
}


/**
 * Adds a new area to the specification, setting only the necessary minimum
 * size constraints.
 *
 * @param left				left border
 * @param top				top border
 * @param right			right border
 * @param bottom			bottom border
 * @param content			the control which is the area content
 * @param minContentSize	minimum content size
 * @return the new area
 */
ALMArea*
ALMLayout::AddArea(ALMTab* left, ALMTab* top, ALMTab* right,
	ALMTab* bottom, BView* content, BSize minContentSize)
{
	InvalidateLayout();
	if (content != NULL)
		View()->AddChild(content);
	ALMArea* area = new ALMArea(this, left, top, right, bottom, content,
		minContentSize);
	fAreas->AddItem(area);
	return area;
}


/**
 * Adds a new area to the specification, setting only the necessary minimum
 * size constraints.
 *
 * @param span				the span that defines the top and bottom border
 * @param column			the column that defines the left and right border
 * @param content			the control which is the area content
 * @param minContentSize	minimum content size
 * @return the new area
 */
ALMArea*
ALMLayout::AddArea(ALMSpan* span, ALMSpan* column, BView* content,
	BSize minContentSize)
{
	InvalidateLayout();
	if (content != NULL)
		View()->AddChild(content);
	ALMArea* area = new ALMArea(this, span, column, content, minContentSize);
	fAreas->AddItem(area);
	return area;
}


/**
 * Adds a new area to the specification, automatically setting preferred size
 * constraints.
 *
 * @param left			left border
 * @param top			top border
 * @param right		right border
 * @param bottom		bottom border
 * @param content		the control which is the area content
 * @return the new area
 */
ALMArea*
ALMLayout::AddArea(ALMTab* left, ALMTab* top, ALMTab* right,
	ALMTab* bottom, BView* content)
{
	InvalidateLayout();
	if (content != NULL)
		View()->AddChild(content);
	ALMArea* area = new ALMArea(this, left, top, right, bottom, content,
		BSize(0, 0));
	area->SetDefaultBehavior();
	area->SetAutoPreferredContentSize(false);
	fAreas->AddItem(area);
	return area;
}


/**
 * Adds a new area to the specification, automatically setting preferred size
 * constraints.
 *
 * @param span			the span that defines the top and bottom border
 * @param column		the column that defines the left and right border
 * @param content		the control which is the area content
 * @return the new area
 */
ALMArea*
ALMLayout::AddArea(ALMSpan* span, ALMSpan* column, BView* content)
{
	InvalidateLayout();
	if (content != NULL)
		View()->AddChild(content);
	ALMArea* area = new ALMArea(this, span, column, content, BSize(0, 0));
	area->SetDefaultBehavior();
	area->SetAutoPreferredContentSize(false);
	fAreas->AddItem(area);
	return area;
}


/**
 * Finds the area that contains the given control.
 *
 * @param control	the control to look for
 * @return the area that contains the control
 */
ALMArea*
ALMLayout::AreaOf(BView* control)
{
	ALMArea* area;
	for (int32 i = 0; i < fAreas->CountItems(); i++) {
		area = (ALMArea*)fAreas->ItemAt(i);
		if (area->Content() == control)
			return area;
	}
	return NULL;
}


/**
 * Gets the ares.
 *
 * @return the areas
 */
BList*
ALMLayout::Areas() const
{
	return fAreas;
}


/**
 * Gets the left variable.
 */
ALMTab*
ALMLayout::Left() const
{
	return fLeft;
}


/**
 * Gets the right variable.
 */
ALMTab*
ALMLayout::Right() const
{
	return fRight;
}


/**
 * Gets the top variable.
 */
ALMTab*
ALMLayout::Top() const
{
	return fTop;
}


/**
 * Gets the bottom variable.
 */
ALMTab*
ALMLayout::Bottom() const
{
	return fBottom;
}


/**
 * Reverse engineers a GUI and recovers an ALM specification.
 * @param parent	the parent container of the GUI
 */
void
ALMLayout::RecoverLayout(BView* parent) {}	// Still working on it.


/**
 * Gets the current layout style.
 */
ALMLayoutStyleType
ALMLayout::LayoutStyle() const
{
	return fLayoutStyle;
}


/**
 * Sets the current layout style.
 */
void
ALMLayout::SetLayoutStyle(ALMLayoutStyleType style)
{
	fLayoutStyle = style;
}


/**
 * Adds view to layout.
 */
BLayoutItem*
ALMLayout::AddView(BView* child)
{
	return NULL;
}


/**
 * Adds view to layout.
 */
BLayoutItem*
ALMLayout::AddView(int32 index, BView* child)
{
return NULL;
}


/**
 * Adds item to layout.
 */
bool
ALMLayout::AddItem(BLayoutItem* item)
{
	return false;
}


/**
 * Adds item to layout.
 */
bool
ALMLayout::AddItem(int32 index, BLayoutItem* item)
{
	return false;
}


/**
 * Removes view from layout.
 */
bool
ALMLayout::RemoveView(BView* child)
{
	return false;
}


/**
 * Removes item from layout.
 */
bool
ALMLayout::RemoveItem(BLayoutItem* item)
{
	return false;
}


/**
 * Removes item from layout.
 */
BLayoutItem*
ALMLayout::RemoveItem(int32 index)
{
	return NULL;
}


/**
 * Sets and gets minimum size.
 */
BSize ALMLayout::MinSize() {
	if (fMinSize == ALMArea::kUndefinedSize)
		fMinSize = CalculateMinSize();
	return fMinSize;
}


/**
 * Sets and gets maximum size.
 */
BSize
ALMLayout::MaxSize()
{
	if (fMaxSize == ALMArea::kUndefinedSize)
		fMaxSize = CalculateMaxSize();
	return fMaxSize;
}


/**
 * Sets and gets preferred size.
 */
BSize
ALMLayout::PreferredSize()
{
	if (fPreferredSize == ALMArea::kUndefinedSize)
		fPreferredSize = CalculatePreferredSize();
	return fPreferredSize;
}


/**
 * Gets the alignment.
 */
BAlignment
ALMLayout::Alignment()
{
	BAlignment alignment;
	alignment.SetHorizontal(B_ALIGN_HORIZONTAL_CENTER);
	alignment.SetVertical(B_ALIGN_VERTICAL_CENTER);
	return alignment;
}


/**
 * Gets whether the height of the layout depends on its width.
 */
bool
ALMLayout::HasHeightForWidth()
{
	return false;
}


/**
 * Sets whether the height of the layout depends on its width.
 */
void
ALMLayout::GetHeightForWidth(float width, float* min, float* max,
	float* preferred)
{
}


/**
 * Invalidates the layout.
 * Resets minimum/maximum/preferred size.
 */
void
ALMLayout::InvalidateLayout()
{
	fMinSize = ALMArea::kUndefinedSize;
	fMaxSize = ALMArea::kUndefinedSize;
	fPreferredSize = ALMArea::kUndefinedSize;
}


/**
 * Calculate and set the layout.
 * If no layout specification is given, a specification is reverse engineered
 * automatically.
 */
void
ALMLayout::LayoutView()
{
	// make sure that layout events occuring during layout are ignored
	// i.e. activated is set to false during layout caluclation
	if (!fActivated)
		return;
	fActivated = false;

	if (View() == NULL)
		return;

	// reverse engineer a layout specification if none was given
	//~ if (this == NULL) RecoverLayout(View());

	// If the layout engine is set to fit the GUI to the given size,
	// then the given size is enforced by setting absolute positions for right
	// and bottom sides.
	if (fLayoutStyle == FIT_TO_SIZE) {
		Right()->SetRange(View()->Bounds().Width(), View()->Bounds().Width());
		Bottom()->SetRange(View()->Bounds().Height(),
			View()->Bounds().Height());
	}

	SolveLayout();

	// if new layout is infasible, use previous layout
	if (Result() == INFEASIBLE) {
		fActivated = true; // now layout calculation is allowed to run again
		return;
	}

	if (Result() != OPTIMAL) {
		Save("failed-layout.txt");
		printf("Could not solve the layout specification (%d). ", Result());
		printf("Saved specification in file failed-layout.txt\n");
	}

	// change the size of the GUI according to the calculated size
	// if the layout engine was configured to do so
	if (fLayoutStyle == ADJUST_SIZE) {
		View()->ResizeTo(floor(Right()->Value() - Left()->Value() + 0.5),
				floor(Bottom()->Value() - Top()->Value() + 0.5));
	}

	// set the calculated positions and sizes for every area
	for (int32 i = 0; i < Areas()->CountItems(); i++)
		((ALMArea*)Areas()->ItemAt(i))->DoLayout();

	fActivated = true;
}


/**
 * Gets the path of the performance log file.
 *
 * @return the path of the performance log file
 */
char*
ALMLayout::PerformancePath() const
{
	return fPerformancePath;
}


/**
 * Sets the path of the performance log file.
 *
 * @param path	the path of the performance log file
 */
void
ALMLayout::SetPerformancePath(char* path)
{
	fPerformancePath = path;
}


/**
 * Caculates the miminum size.
 */
BSize
ALMLayout::CalculateMinSize()
{
	BList* oldObjFunction = ObjFunction();
	BList newObjFunction(2);
	newObjFunction.AddItem(new Summand(1.0, fRight));
	newObjFunction.AddItem(new Summand(1.0, fBottom));
	SetObjFunction(&newObjFunction);
	SolveLayout();
	SetObjFunction(oldObjFunction);
	UpdateObjFunction();
	delete (Summand*)newObjFunction.ItemAt(0);
	delete (Summand*)newObjFunction.ItemAt(1);

	if (Result() == UNBOUNDED)
		return ALMArea::kMinSize;
	if (Result() != OPTIMAL) {
		Save("failed-layout.txt");
		printf("Could not solve the layout specification (%d). Saved "
			"specification in file failed-layout.txt", Result());
	}

	return BSize(Right()->Value() - Left()->Value(),
		Bottom()->Value() - Top()->Value());
}


/**
 * Caculates the maximum size.
 */
BSize
ALMLayout::CalculateMaxSize()
{
	BList* oldObjFunction = ObjFunction();
	BList newObjFunction(2);
	newObjFunction.AddItem(new Summand(-1.0, fRight));
	newObjFunction.AddItem(new Summand(-1.0, fBottom));
	SetObjFunction(&newObjFunction);
	SolveLayout();
	SetObjFunction(oldObjFunction);
	UpdateObjFunction();
	delete (Summand*)newObjFunction.ItemAt(0);
	delete (Summand*)newObjFunction.ItemAt(1);

	if (Result() == UNBOUNDED)
		return ALMArea::kMaxSize;
	if (Result() != OPTIMAL) {
		Save("failed-layout.txt");
		printf("Could not solve the layout specification (%d). Saved "
			"specification in file failed-layout.txt", Result());
	}

	return BSize(Right()->Value() - Left()->Value(),
		Bottom()->Value() - Top()->Value());
}


/**
 * Caculates the preferred size.
 */
BSize
ALMLayout::CalculatePreferredSize()
{
	SolveLayout();
	if (Result() != OPTIMAL) {
		Save("failed-layout.txt");
		printf("Could not solve the layout specification (%d). Saved "
			"specification in file failed-layout.txt", Result());
	}

	return BSize(Right()->Value() - Left()->Value(),
		Bottom()->Value() - Top()->Value());
}

