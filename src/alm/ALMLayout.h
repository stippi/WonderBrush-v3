/*
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Distributed under the terms of the MIT License.
 */
#ifndef	ALM_LAYOUT_H
#define	ALM_LAYOUT_H

#include <File.h>
#include <Layout.h>
#include <List.h>
#include <Size.h>
#include <View.h>

#include <linprog/Constraint.h>
#include <linprog/LinearSpec.h>

class ALMArea;
class ALMSpan;
class ALMTab;


/**
 * The possibilities for adjusting a GUI's layout.
 * Either change the child controls so that they fit into their parent
 * control, or adjust the size of the parent control so that the children
 * have as much space as they want.
 */
enum ALMLayoutStyleType {
	FIT_TO_SIZE,
	ADJUST_SIZE
};


/**
 * A GUI layout engine using the ALM.
 */
class ALMLayout : public BLayout, public LinearSpec {
public:
								ALMLayout();
			void				SolveLayout();

			ALMTab*				AddXTab();
			ALMTab*				AddYTab();
			ALMSpan*			AddRow();
			ALMSpan*			AddRow(ALMTab* top, ALMTab* bottom);
			ALMSpan*			AddColumn();
			ALMSpan*			AddColumn(ALMTab* left, ALMTab* right);

			ALMArea*			AddArea(ALMTab* left, ALMTab* top,
									ALMTab* right, ALMTab* bottom,
									BView* content, BSize minContentSize);
			ALMArea*			AddArea(ALMSpan* span, ALMSpan* column,
									BView* content, BSize minContentSize);
			ALMArea*			AddArea(ALMTab* left, ALMTab* top,
									ALMTab* right, ALMTab* bottom,
									BView* content);
			ALMArea*			AddArea(ALMSpan* span, ALMSpan* column,
									BView* content);
			ALMArea*			AreaOf(BView* control);
			BList*				Areas() const;

			ALMTab*				Left() const;
			ALMTab*				Right() const;
			ALMTab*				Top() const;
			ALMTab*				Bottom() const;

			void				RecoverLayout(BView* parent);

			ALMLayoutStyleType	LayoutStyle() const;
			void				SetLayoutStyle(ALMLayoutStyleType style);

			BLayoutItem*		AddView(BView* child);
			BLayoutItem*		AddView(int32 index, BView* child);
			bool				AddItem(BLayoutItem* item);
			bool				AddItem(int32 index, BLayoutItem* item);
			bool				RemoveView(BView* child);
			bool				RemoveItem(BLayoutItem* item);
			BLayoutItem*		RemoveItem(int32 index);

			BSize				MinSize();
			BSize				MaxSize();
			BSize				PreferredSize();
			BAlignment			Alignment();
			bool				HasHeightForWidth();
			void				GetHeightForWidth(float width, float* min,
										float* max, float* preferred);
			void				InvalidateLayout();
			void				LayoutView();

			char*				PerformancePath() const;
			void				SetPerformancePath(char* path);

private:
			BSize				CalculateMinSize();
			BSize				CalculateMaxSize();
			BSize				CalculatePreferredSize();

private:
			ALMLayoutStyleType	fLayoutStyle;
			bool				fActivated;

			BList*				fAreas;
			ALMTab*				fLeft;
			ALMTab*				fRight;
			ALMTab*				fTop;
			ALMTab*				fBottom;
			BSize				fMinSize;
			BSize				fMaxSize;
			BSize				fPreferredSize;
			char*				fPerformancePath;
};

#endif	// ALM_LAYOUT_H
