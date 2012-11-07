/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "StyleRunList.h"

#include <stdio.h>

#include "StyleRun.h"

// constructor
StyleRunList::StyleRunList()
	: fRuns(4)
{
}

// constructor
StyleRunList::StyleRunList(const StyleRunList& other)
	: fRuns(4)
{
	*this = other;
}

// =
StyleRunList&
StyleRunList::operator=(const StyleRunList& other)
{
	MakeEmpty();

	int32 count = other.fRuns.CountItems();
	for (int32 i = 0; i < count; i++) {
		StyleRun* run = (StyleRun*)other.fRuns.ItemAtFast(i);
		if (!Append(*run))
			break;
	}

	return *this;
}

// ==
bool
StyleRunList::operator==(const StyleRunList& other) const
{
	int32 count = fRuns.CountItems();
	if (count != other.fRuns.CountItems())
		return false;

	for (int32 i = 0; i < count; i++) {
		StyleRun* runA = (StyleRun*)fRuns.ItemAtFast(i);
		StyleRun* runB = (StyleRun*)other.fRuns.ItemAtFast(i);
		if (*runA != *runB)
			return false;
	}

	return true;
}

// !=
bool
StyleRunList::operator!=(const StyleRunList& other) const
{
	return !(*this == other);
}

// destructor
StyleRunList::~StyleRunList()
{
	MakeEmpty();
}

// MakeEmpty
void
StyleRunList::MakeEmpty()
{
	for (int32 i = fRuns.CountItems() - 1; i >= 0; i--)
		delete (StyleRun*)fRuns.ItemAtFast(i);
	fRuns.MakeEmpty();
}

// Append
bool
StyleRunList::Append(const StyleRun& runToAppend)
{
	if (fRuns.CountItems() > 0) {
		StyleRun* run
			= (StyleRun*)fRuns.ItemAtFast(fRuns.CountItems() - 1);
		if (run->IsSameStyle(runToAppend)) {
			run->SetLength(run->GetLength() + runToAppend.GetLength());
			return true;
		}
	}

	StyleRun* inserted = new(std::nothrow) StyleRun(runToAppend);
	if (inserted == NULL || !fRuns.AddItem(inserted)) {
		delete inserted;
		return false;
	}

	return true;
}

// Insert
bool
StyleRunList::Insert(int32 textOffset, const StyleRun& runToInsert)
{
//printf("StyleRunList::Insert(%ld, StyleRun(%s, %.1f, %ld))\n",
//	textOffset, runToInsert.GetStyle()->GetFont().getName(),
//	runToInsert.GetStyle()->GetFont().getSize(), runToInsert.GetLength());
	int32 startOffset = 0;
	int32 styleCount = fRuns.CountItems();
	for (int32 i = 0; i < styleCount; i++) {
		StyleRun* run = (StyleRun*)fRuns.ItemAtFast(i);
		if (startOffset == textOffset
			|| startOffset + run->GetLength() == textOffset) {
			if (run->IsSameStyle(runToInsert)) {
				// Runs equal, just increase length of existing run
				run->SetLength(run->GetLength() + runToInsert.GetLength());
				return true;
			}
			// Insert before or after run at this index
			StyleRun* inserted = new(std::nothrow) StyleRun(runToInsert);
			if (startOffset != textOffset)
				i++;
			if (inserted == NULL || !fRuns.AddItem(inserted, i)) {
				delete inserted;
				return false;
			}
			return true;
		}

		if (startOffset + run->GetLength() > textOffset) {
			// Insert here
			if (run->IsSameStyle(runToInsert)) {
				// Runs equal, just increase length of existing run
				run->SetLength(run->GetLength() + runToInsert.GetLength());
				return true;
			}
			// Split run at this index
			StyleRun* remaining = new(std::nothrow) StyleRun(*run);
			if (remaining == NULL || !fRuns.AddItem(remaining, i + 1)) {
				delete remaining;
				return false;
			}
			// Insert new style
			StyleRun* inserted = new(std::nothrow) StyleRun(runToInsert);
			if (inserted == NULL || !fRuns.AddItem(inserted, i + 1)) {
				delete (StyleRun*)fRuns.RemoveItem(i + 1);
				delete inserted;
				return false;
			}
			// Success, update lengths of the two split runs
			int32 previousLength = run->GetLength();
			int32 newLength = textOffset - startOffset;
			run->SetLength(newLength);
			remaining->SetLength(previousLength - newLength);
			return true;
		}

		startOffset += run->GetLength();
	}

	return Append(runToInsert);
}

// Insert
bool
StyleRunList::Insert(int32 textOffset, const StyleRunList& runListToInsert)
{
	// TODO: Optimize
	int32 count = runListToInsert.CountRuns();
	for (int32 i = 0; i < count; i++) {
		StyleRun* run = (StyleRun*)runListToInsert.fRuns.ItemAtFast(i);
		if (!Insert(textOffset, *run)) {
			// TODO: Roll back already inserted styles
			return false;
		}
		textOffset += run->GetLength();
	}

	_MergeEqualRuns();

	return true;
}

// Remove
void
StyleRunList::Remove(int32 textOffset, int32 length)
{
	if (textOffset < 0) {
		length += textOffset;
		textOffset = 0;
	}
	if (length <= 0)
		return;

	bool removedRuns = false;

	int32 startOffset = 0;
	for (int32 i = 0; i < fRuns.CountItems(); i++) {
		StyleRun* run = (StyleRun*)fRuns.ItemAtFast(i);
		if (startOffset == textOffset
			|| startOffset + run->GetLength() > textOffset) {

			int32 newRunLength = 0;
			if (startOffset < textOffset)
				newRunLength += textOffset - startOffset;

			if (textOffset + length < startOffset + run->GetLength())
				newRunLength += (startOffset + run->GetLength())
					- (textOffset + length);

			if (newRunLength == 0) {
				length -= run->GetLength();
				// Remove this run altogether
				fRuns.RemoveItem(i);
				i--;
				delete run;
				removedRuns = true;
				continue;
			}

			length -= run->GetLength() - newRunLength;
			run->SetLength(newRunLength);
			if (length < 0)
				debugger("Removed too much in StyleRunList::Remove()");

			if (length == 0)
				break;
		}

		startOffset += run->GetLength();
	}

	if (removedRuns) {
		// Removing runs may have caused equal runs to attach
		_MergeEqualRuns();
	}
}

// FindStyleRun
const StyleRun*
StyleRunList::FindStyleRun(int32 textOffset) const
{
	if (textOffset < 0)
		return NULL;

	int32 startOffset = 0;
	int32 styleCount = fRuns.CountItems();
	for (int32 i = 0; i < styleCount; i++) {
		StyleRun* run = (StyleRun*)fRuns.ItemAtFast(i);
		startOffset += run->GetLength();
		if (startOffset > textOffset)
			return run;
	}

	return (StyleRun*)fRuns.LastItem();
}

// StyleRunAt
const StyleRun*
StyleRunList::StyleRunAt(int32 index) const
{
	return (StyleRun*)fRuns.ItemAt(index);
}

// CountRuns
int32
StyleRunList::CountRuns() const
{
	return fRuns.CountItems();
}

// GetLength
int32
StyleRunList::GetLength() const
{
	int32 length = 0;
	int32 count = fRuns.CountItems();
	for (int32 i = 0; i < count; i++) {
		StyleRun* run = (StyleRun*)fRuns.ItemAtFast(i);
		length += run->GetLength();
	}
	return length;
}

// GetSubList
StyleRunList*
StyleRunList::GetSubList(int32 textOffset, int32 length) const
{
	// TODO: Could be optimized...

	StyleRunList* subList = new(std::nothrow) StyleRunList(*this);
	if (subList == NULL)
		return NULL;

	// Cut off beginning
	if (textOffset > 0)
		subList->Remove(0, textOffset);

	// Cut off end
	int32 subListLength = subList->GetLength();
	if (subListLength > length)
		subList->Remove(length, subListLength - length);

	return subList;
}

// PrintToStream()
void
StyleRunList::PrintToStream() const
{
	printf("%p StyleRunList:\n", this);
	for (int32 i = 0; i < fRuns.CountItems(); i++) {
		StyleRun* run = (StyleRun*)fRuns.ItemAtFast(i);
		const Font& font = run->GetStyle()->GetFont();
		printf("  [%ld] %s/%s %.1f - %ld\n", i,
			font.getFamily(), font.getStyle(), font.getSize(),
			run->GetLength());
	}
}

// #pragma mark - private

// _MergeEqualRuns
void
StyleRunList::_MergeEqualRuns()
{
	for (int32 i = 0; i < fRuns.CountItems() - 1; i++) {
		StyleRun* runA = (StyleRun*)fRuns.ItemAtFast(i);
		StyleRun* runB = (StyleRun*)fRuns.ItemAtFast(i + 1);
		if (runA->IsSameStyle(*runB)) {
			fRuns.RemoveItem(i + 1);
			runA->SetLength(runA->GetLength() + runB->GetLength());
			delete runB;
			i--;
		}
	}
}
