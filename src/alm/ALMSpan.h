/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Distributed under the terms of the MIT License.
 */
#ifndef	ALM_SPAN_H
#define	ALM_SPAN_H

#include <linprog/Constraint.h>

#include <List.h>


class ALMLayout;
class ALMTab;

/**
 * Represents a span defined by two tabs.
 */
class ALMSpan {
public:
			ALMTab*				StartTab() const;
			ALMTab*				EndTab() const;

			ALMSpan*			Previous() const;
			void				SetPrevious(ALMSpan* span);
			ALMSpan*			Next() const;
			void				SetNext(ALMSpan* span);

			void				InsertBefore(ALMSpan* span);
			void				InsertAfter(ALMSpan* span);

			Constraint*			SetSameSizeAs(ALMSpan* span);
			BList*				Constraints() const;
			void				SetConstraints(BList* constraints);

								~ALMSpan();

protected:
								ALMSpan(ALMLayout* ls);

protected:
			ALMLayout*			fLayout;
			ALMTab*				fStartTab;
			ALMTab*				fEndTab;

private:
			ALMSpan*			fPrevious;
			ALMSpan*			fNext;
			Constraint*			fPreviousGlue;
			Constraint*			fNextGlue;
			BList*				fConstraints;

public:
			friend class		ALMLayout;
};

#endif	// ALM_SPAN_H
