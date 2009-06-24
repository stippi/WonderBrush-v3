// Column.cpp

#include "Column.h"
#include "ColumnHeader.h"
#include "ColumnTreeView.h"
#include "LabelColumnHeader.h"

// constructor
Column::Column(const char* label, const char* name, float width, uint32 flags)
	: fParent(NULL),
	  fHeader(NULL),
	  fName(name),
	  fIndex(0),
	  fXOffset(0),
	  fWidth(COLUMN_MIN_WIDTH),
	  fMinWidth(COLUMN_MIN_WIDTH),
	  fMaxWidth(COLUMN_MAX_WIDTH),
	  fFlags(0)
{
	SetHeader(new LabelColumnHeader(label));
	SetWidth(width);
	SetFlags(flags);
}

// constructor
Column::Column(ColumnHeader* header, const char* name, float width,
			   uint32 flags)
	: fParent(NULL),
	  fHeader(NULL),
	  fName(name),
	  fIndex(0),
	  fXOffset(0),
	  fWidth(COLUMN_MIN_WIDTH),
	  fMinWidth(COLUMN_MIN_WIDTH),
	  fMaxWidth(COLUMN_MAX_WIDTH),
	  fFlags(0)
{
	SetHeader(header);
	SetWidth(width);
	SetFlags(flags & COLUMN_USER_FLAGS);
}

// destructor
Column::~Column()
{
	delete fHeader;
}

// Name
const char*
Column::Name() const
{
	return fName.String();
}

// SetHeader
void
Column::SetHeader(ColumnHeader* header)
{
	if (header != fHeader) {
		if (fHeader)
			delete fHeader;
		fHeader = header;
		if (fHeader) {
			fHeader->SetParentColumn(this);
			InvalidateHeader();
		}
	}
}

// Header
ColumnHeader*
Column::Header() const
{
	return fHeader;
}

// InvalidateHeader
void
Column::InvalidateHeader()
{
	if (fParent)
		fParent->InvalidateHeader(fIndex);
}

// SetIndex
void
Column::SetIndex(int32 index)
{
	fIndex = index;
}

// Index
int32
Column::Index() const
{
	return fIndex;
}

// SetXOffset
void
Column::SetXOffset(float offset)
{
	fXOffset = offset;
}

// XOffset
float
Column::XOffset() const
{
	return fXOffset;
}

// SetWidth
void
Column::SetWidth(float width)
{
	// check parameter
	if (width < fMinWidth)
		width = fMinWidth;
	if (width > fMaxWidth)
		width = fMaxWidth;
	if (width != fWidth) {
		fWidth = width;
		// ...
	}
}

// Width
float
Column::Width() const
{
	return fWidth;
}

// SetWidthLimits
void
Column::SetWidthLimits(float minWidth, float maxWidth)
{
	// check parameters
	if (minWidth < COLUMN_MIN_WIDTH)
		minWidth = COLUMN_MIN_WIDTH;
	if (maxWidth > COLUMN_MAX_WIDTH)
		maxWidth = COLUMN_MAX_WIDTH;
	if (maxWidth < minWidth)
		maxWidth = minWidth;
	if (minWidth != fMinWidth || maxWidth != fMaxWidth) {
		fMinWidth = minWidth;
		fMaxWidth = maxWidth;
		// adjust width, if necessary
		SetWidth(fWidth);
		// ...
	}
}

// MinWidth
float
Column::MinWidth() const
{
	return fMinWidth;
}

// MaxWidth
float
Column::MaxWidth() const
{
	return fMaxWidth;
}

// SetFlags
void
Column::SetFlags(uint32 flags)
{
	if (flags & COLUMN_JOIN_LEFT)
		flags &= ~COLUMN_JOIN_RIGHT;
	fFlags = flags;
}

// Flags
uint32
Column::Flags() const
{
	return fFlags;
}

// AddFlags
void
Column::AddFlags(uint32 flags)
{
	fFlags |= flags;
}

// ClearFlags
void
Column::ClearFlags(uint32 flags)
{
	fFlags &= ~flags;
}

// IsMovable
bool
Column::IsMovable() const
{
	return fFlags & COLUMN_MOVABLE;
}

// IsSortKeyable
bool
Column::IsSortKeyable() const
{
	return fFlags & COLUMN_SORT_KEYABLE;
}

// SetVisible
void
Column::SetVisible(bool visible)
{
	if (visible != IsVisible()) {
		if (visible)
			fFlags |= COLUMN_VISIBLE;
		else
			fFlags &= ~COLUMN_VISIBLE;
	}
}

// IsVisible
bool
Column::IsVisible() const
{
	return (fFlags & COLUMN_VISIBLE);
}

// SetParent
void
Column::SetParent(ColumnTreeView* parent)
{
	fParent = parent;
	// ...
}

// Parent
ColumnTreeView*
Column::Parent() const
{
	return fParent;
}

