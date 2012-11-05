/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#ifndef STYLE_RUN_LIST_H
#define STYLE_RUN_LIST_H

#include <List.h>

class StyleRun;

class StyleRunList {
public:
								StyleRunList();
								StyleRunList(const StyleRunList& other);
	virtual						~StyleRunList();

				StyleRunList&	operator=(const StyleRunList& other);
				bool			operator==(const StyleRunList& other) const;
				bool			operator!=(const StyleRunList& other) const;

				void			MakeEmpty();

				bool			Append(const StyleRun& runToAppend);
				bool			Insert(int32 textOffset,
									const StyleRun& runToInsert);
				bool			Insert(int32 textOffset,
									const StyleRunList& runListToInsert);

				void			Remove(int32 textOffset, int32 length);

				const StyleRun*	FindStyleRun(int32 textOffset) const;
				const StyleRun*	StyleRunAt(int32 index) const;

				int32			CountRuns() const;
				int32			GetLength() const;

				StyleRunList*	GetSubList(int32 textOffset,
									int32 length) const;

private:

				void			_MergeEqualRuns();

private:
				BList			fRuns;
};

#endif // STYLE_RUN_LIST_H
