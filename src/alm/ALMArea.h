/*
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Distributed under the terms of the MIT License.
 */

#ifndef	ALM_AREA_H
#define	ALM_AREA_H

#include <Alignment.h>
#include <List.h>
#include <Size.h>
#include <SupportDefs.h>
#include <View.h>

#include <linprog/Constraint.h>

#include "ALMSpan.h"
#include "ALMTab.h"


class ALMLayout;

/**
 * Rectangular area in the GUI, defined by a tab on each side.
 */
class ALMArea {
public:
								~ALMArea();

			ALMTab*				Left() const;
			void				SetLeft(ALMTab* left);
			ALMTab*				Right() const;
			void				SetRight(ALMTab* right);
			ALMTab*				Top() const;
			void				SetTop(ALMTab* top);
			ALMTab*				Bottom() const;
			void				SetBottom(ALMTab* bottom);

			ALMSpan*			GetRow() const;
			void				SetRow(ALMSpan* span);
			ALMSpan*			GetColumn() const;
			void				SetColumn(ALMSpan* column);

			// TODO: Replace BView by something else.
			BView*				Content() const;
			void				SetContent(BView* content);
			ALMTab*			ContentLeft() const;
			ALMTab*			ContentTop() const;
			ALMTab*			ContentRight() const;
			ALMTab*			ContentBottom() const;
			BSize				MinContentSize() const;
			void				SetMinContentSize(BSize min);
			BSize				MaxContentSize() const;
			void				SetMaxContentSize(BSize max);
			BSize				PreferredContentSize() const;
			void				SetPreferredContentSize(BSize preferred);
			double				ContentAspectRatio() const;
			void				SetContentAspectRatio(double ratio);

			BSize				ShrinkPenalties() const;
			void				SetShrinkPenalties(BSize shrink);
			BSize				GrowPenalties() const;
			void				SetGrowPenalties(BSize grow);

			BAlignment			Alignment() const;
			void				SetAlignment(BAlignment alignment);
			void				SetHorizontalAlignment(alignment horizontal);
			void				SetVerticalAlignment(
									vertical_alignment vertical);

			int32				LeftInset() const;
			void				SetLeftInset(int32 left);
			int32				TopInset() const;
			void				SetTopInset(int32 top);
			int32				RightInset() const;
			void				SetRightInset(int32 right);
			int32				BottomInset() const;
			void				SetBottomInset(int32 bottom);

			void				SetDefaultBehavior();
			bool				AutoPreferredContentSize() const;
			void				SetAutoPreferredContentSize(bool value);

								operator BString() const;
			void				GetString(BString& string) const;

			Constraint*			HasSameWidthAs(ALMArea* area);
			Constraint*			HasSameHeightAs(ALMArea* area);
			BList*				HasSameSizeAs(ALMArea* area);


protected:
								ALMArea(ALMLayout* ls, ALMTab* left,
									ALMTab* top, ALMTab* right,
									ALMTab* bottom, BView* content,
									BSize minContentSize);
								ALMArea(ALMLayout* ls, ALMSpan* span,
									ALMSpan* column, BView* content,
									BSize minContentSize);
			void				DoLayout();

private:
			void				InitChildArea();
			void				UpdateHorizontal();
			void				UpdateVertical();
			void				Init(ALMLayout* ls, ALMTab* left,
									ALMTab* top, ALMTab* right,
									ALMTab* bottom, BView* content,
									BSize minContentSize);

public:
	static	BSize				kMaxSize;
	static	BSize				kMinSize;
	static	BSize				kUndefinedSize;

protected:
			BView*				fContent;
			BList*				fConstraints;

private:
			ALMLayout*			fLS;
			ALMTab*			fLeft;
			ALMTab*			fRight;
			ALMTab*			fTop;
			ALMTab*			fBottom;
			ALMSpan*			fRow;
			ALMSpan*			fColumn;
			BSize				fMinContentSize;
			BSize				fMaxContentSize;
			Constraint*			fMinContentWidth;
			Constraint*			fMaxContentWidth;
			Constraint*			fMinContentHeight;
			Constraint*			fMaxContentHeight;
			BSize				fPreferredContentSize;
			BSize				fShrinkPenalties;
			BSize				fGrowPenalties;
			double				fContentAspectRatio;
			Constraint*			fContentAspectRatioC;
			bool				fAutoPreferredContentSize;
			Constraint*			fPreferredContentWidth;
			Constraint*			fPreferredContentHeight;
			ALMArea*			fChildArea;
			BAlignment			fAlignment;
			int32				fLeftInset;
			int32				fTopInset;
			int32				fRightInset;
			int32				fBottomInset;
			Constraint*			fLeftConstraint;
			Constraint*			fTopConstraint;
			Constraint*			fRightConstraint;
			Constraint*			fBottomConstraint;

public:
			friend class		ALMLayout;
};

#endif	// ALM_AREA_H

